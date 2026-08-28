#include "plotwindow.h"

#include <QPainter>
#include <QPolygonF>
#include <QComboBox>
#include <QListWidget>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWheelEvent>
#include <QMouseEvent>
#include <limits>

// 曲线颜色轮转
static const QColor g_seriesColors[] = {
    QColor("#52dcff"), QColor("#4ade80"), QColor("#ffd166"),
    QColor("#ff5c8a"), QColor("#b48cff"), QColor("#ff9f40"),
    QColor("#7ee8fa"), QColor("#f15bb5"),
};

// ===================== PlotWidget =====================
PlotWidget::PlotWidget(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_OpaquePaintEvent);
    setMinimumSize(500, 380);
    setMouseTracking(true);

    // 30ms 定时重绘，数据只累加不触发重绘，避免高频卡顿
    m_repaintTimer = new QTimer(this);
    m_repaintTimer->setInterval(30);
    connect(m_repaintTimer, &QTimer::timeout, this, [this](){ update(); });
    m_repaintTimer->start();
}

void PlotWidget::setSeries(const QList<PlotSeries*> &s) { m_series = s; update(); }
void PlotWidget::setAutoY(bool on) { m_autoY = on; if (on) m_autoInit = false; update(); }
void PlotWidget::setYRange(double mn, double mx) { m_yMin = mn; m_yMax = mx; update(); }
void PlotWidget::setMaxPoints(int n) { m_maxPoints = n; update(); }
void PlotWidget::setXTime(bool on) { m_xTime = on; update(); }
void PlotWidget::setTimeSpan(double sec) { m_timeSpan = sec; update(); }

void PlotWidget::setInteractive(bool on)
{
    m_interactive = on;
    if (!on) {
        m_zoomY = 1.0;
        m_panY = 0.0;
        m_panXSec = 0.0;
        m_cursorValid = false;
    }
    update();
}

void PlotWidget::resetView()
{
    m_zoomY = 1.0;
    m_panY = 0.0;
    m_panXSec = 0.0;
    update();
}

void PlotWidget::wheelEvent(QWheelEvent *e)
{
    if (!m_interactive) { QWidget::wheelEvent(e); return; }
    double factor = (e->angleDelta().y() > 0) ? 1.25 : 0.8;
    m_zoomY *= factor;
    if (m_zoomY < 0.01) m_zoomY = 0.01;
    if (m_zoomY > 500.0) m_zoomY = 500.0;
    update();
}

void PlotWidget::mousePressEvent(QMouseEvent *e)
{
    if (m_interactive && e->button() == Qt::LeftButton) {
        m_dragging = true;
        m_lastPos = e->pos();
        setCursor(Qt::ClosedHandCursor);
    }
    QWidget::mousePressEvent(e);
}

void PlotWidget::mouseMoveEvent(QMouseEvent *e)
{
    m_cursor = e->pos();
    m_cursorValid = true;

    if (m_interactive && m_dragging) {
        QPoint d = e->pos() - m_lastPos;
        m_lastPos = e->pos();
        // 用当前视窗的尺度换算像素->数据
        int w = width() - 68 - 16;
        int h = height() - 16 - 42;
        if (w > 10 && h > 10) {
            double yspan = m_viewYMax - m_viewYMin;
            m_panY += d.y() * yspan / h;
            m_panXSec += d.x() * m_viewWinSec / w;
        }
        update();
    }
    QWidget::mouseMoveEvent(e);
}

void PlotWidget::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        m_dragging = false;
        unsetCursor();
    }
    QWidget::mouseReleaseEvent(e);
}

void PlotWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.fillRect(rect(), QColor("#0d1b24"));

    const int left = 68, top = 16, right = 16, bottom = 42;
    QRect plotRect(left, top, width() - left - right, height() - top - bottom);
    if (plotRect.width() < 20 || plotRect.height() < 20) return;

    double tNewest = 0;
    quint64 newestFrame = 0;
    bool haveSeries = false;
    for (PlotSeries *s : m_series) {
        if (s->data.isEmpty()) continue;
        haveSeries = true;
        if (!s->tSec.isEmpty() && s->tSec.last() > tNewest) tNewest = s->tSec.last();
        if (s->frameCount > newestFrame) newestFrame = s->frameCount;
    }
    if (!haveSeries) {
        QFont font = p.font(); font.setPointSize(11); p.setFont(font);
        p.setPen(QColor("#7a8b96"));
        p.drawText(plotRect, Qt::AlignCenter, "在左侧勾选变量后开始实时绘图");
        return;
    }

    // 帧周期估计（取各系列最小正周期 = 最高频率）
    double period = 0.1;
    double minP = 1e18;
    for (PlotSeries *s : m_series) {
        if (s->tSec.size() >= 2) {
            double pp = (s->tSec.last() - s->tSec.first()) / (s->tSec.size() - 1);
            if (pp > 1e-6) minP = qMin(minP, pp);
        }
    }
    if (minP < 1e18) period = minP;

    double winSec = m_xTime ? m_timeSpan : (m_maxPoints * period);
    if (winSec < 1e-6) winSec = 1e-6;
    double stepX = plotRect.width() / winSec;   // 像素/秒
    double xRight = plotRect.right();
    double winRight = tNewest - m_panXSec;      // 暂停回看时右缘左移

    // ---- Y 范围：平滑（扩展立即、收缩缓慢） ----
    double ymin = m_yMin, ymax = m_yMax;
    if (m_autoY) {
        double mn = 0, mx = 1;
        bool has = false;
        for (PlotSeries *s : m_series) {
            for (int i = s->data.size() - 1; i >= 0; i--) {
                double dt = winRight - s->tSec[i];
                if (dt < 0) continue;
                if (dt > winSec) break;
                double v = s->data[i];
                if (!has) { mn = mx = v; has = true; }
                else { mn = qMin(mn, v); mx = qMax(mx, v); }
            }
        }
        if (!has) { mn = -1; mx = 1; }
        if (mn == mx) { mn -= 1; mx += 1; }

        double dt = m_paintClock.elapsed() / 1000.0;
        m_paintClock.restart();
        if (dt < 0.001) dt = 0.033;
        double k = 1.0 - exp(-dt / 1.0);   // 收缩时间常数约 1 秒

        if (!m_autoInit) { m_autoMin = mn; m_autoMax = mx; m_autoInit = true; }
        else {
            if (mn < m_autoMin) m_autoMin = mn;   // 扩展立即
            if (mx > m_autoMax) m_autoMax = mx;
            m_autoMin += (mn - m_autoMin) * k;     // 收缩缓慢
            m_autoMax += (mx - m_autoMax) * k;
        }
        double pad = (m_autoMax - m_autoMin) * 0.1;
        ymin = m_autoMin - pad;
        ymax = m_autoMax + pad;
    }
    if (ymin >= ymax) { ymin -= 1; ymax += 1; }

    // 交互缩放/平移
    if (m_interactive) {
        double cy = (ymin + ymax) / 2.0 + m_panY;
        double hy = (ymax - ymin) / 2.0 / m_zoomY;
        ymin = cy - hy;
        ymax = cy + hy;
    }

    m_viewYMin = ymin;
    m_viewYMax = ymax;
    m_viewWinSec = winSec;

    QFont font = p.font();
    font.setPointSize(10);
    p.setFont(font);

    // 网格 + Y 刻度
    const int xdiv = 8, ydiv = 5;
    for (int i = 0; i <= ydiv; i++) {
        double yv = ymax - (ymax - ymin) * i / ydiv;
        int y = plotRect.top() + plotRect.height() * i / ydiv;
        p.setPen(QPen(QColor("#2a3b47"), 1));
        p.drawLine(plotRect.left(), y, plotRect.right(), y);
        p.setPen(QColor("#7a8b96"));
        p.drawText(QRect(0, y - 8, left - 6, 16), Qt::AlignRight | Qt::AlignVCenter,
                   QString::number(yv, 'g', 4));
    }
    // 网格 + X 刻度
    for (int i = 0; i <= xdiv; i++) {
        int x = plotRect.left() + plotRect.width() * i / xdiv;
        p.setPen(QPen(QColor("#2a3b47"), 1));
        p.drawLine(x, plotRect.top(), x, plotRect.bottom());
        double t = winRight - (1.0 - i / double(xdiv)) * winSec;
        QString label;
        if (m_xTime) {
            label = QString::number(t, 'f', 1) + "s";
        } else {
            double ageFrames = (winRight - t) / period;
            label = QString::number(quint64(newestFrame - m_panXSec / period) - quint64(ageFrames));
        }
        p.setPen(QColor("#7a8b96"));
        p.drawText(QRect(x - 30, plotRect.bottom() + 4, 60, 16),
                   Qt::AlignHCenter | Qt::AlignTop, label);
    }

    // 绘图区边框
    p.setPen(QPen(QColor("#3d515c"), 1));
    p.drawRect(plotRect);

    // 曲线
    auto yToPix = [&](double v) { return plotRect.bottom() - (v - ymin) / (ymax - ymin) * plotRect.height(); };

    for (PlotSeries *s : m_series) {
        if (s->data.isEmpty()) continue;

        if (s->data.size() <= plotRect.width() * 2) {
            // 稀疏：折线
            QPolygonF poly;
            for (int i = s->data.size() - 1; i >= 0; i--) {
                double dt = winRight - s->tSec[i];
                if (dt < 0) continue;
                if (dt > winSec) break;
                double x = xRight - dt * stepX;
                double y = yToPix(s->data[i]);
                poly.prepend(QPointF(x, y));
            }
            if (poly.size() == 1) poly << QPointF(poly[0].x() + 1, poly[0].y());
            p.setPen(QPen(s->color, 1.5));
            p.drawPolyline(poly);
        } else {
            // 密集：逐像素 min/max，保留每一帧、丝滑
            const int cols = plotRect.width();
            QVector<double> colMin(cols + 1, std::numeric_limits<double>::quiet_NaN());
            QVector<double> colMax(cols + 1, std::numeric_limits<double>::quiet_NaN());
            for (int i = s->data.size() - 1; i >= 0; i--) {
                double dt = winRight - s->tSec[i];
                if (dt < 0) continue;
                if (dt > winSec) break;
                int c = int(xRight - dt * stepX - plotRect.left());
                if (c < 0) c = 0;
                if (c > cols) c = cols;
                double v = s->data[i];
                if (std::isnan(colMin[c]) || v < colMin[c]) colMin[c] = v;
                if (std::isnan(colMax[c]) || v > colMax[c]) colMax[c] = v;
            }
            p.setPen(QPen(s->color, 1.2));
            for (int c = 0; c <= cols; c++) {
                if (std::isnan(colMin[c])) continue;
                double y1 = yToPix(colMax[c]);
                double y2 = yToPix(colMin[c]);
                p.drawLine(plotRect.left() + c, y1, plotRect.left() + c, y2);
            }
        }
    }

    // 图例（右上角）
    int legY = plotRect.top() + 8;
    for (PlotSeries *s : m_series) {
        int tw = p.fontMetrics().horizontalAdvance(s->name) + 20;
        int x = plotRect.right() - tw - 6;
        p.fillRect(x, legY, 12, 12, s->color);
        p.setPen(QColor("#dce6ec"));
        p.drawText(QRect(x + 16, legY, tw - 16, 12), Qt::AlignLeft | Qt::AlignVCenter, s->name);
        legY += 18;
    }

    // 十字线 + 读数（暂停交互时）
    if (m_interactive && m_cursorValid && plotRect.contains(m_cursor.toPoint())) {
        double xd = winRight - (xRight - m_cursor.x()) / stepX;
        double yd = ymin + (plotRect.bottom() - m_cursor.y()) / plotRect.height() * (ymax - ymin);

        p.setPen(QPen(QColor("#5a6b76"), 1, Qt::DashLine));
        p.drawLine(plotRect.left(), int(m_cursor.y()), plotRect.right(), int(m_cursor.y()));
        p.drawLine(int(m_cursor.x()), plotRect.top(), int(m_cursor.x()), plotRect.bottom());

        QString sx = m_xTime ? QString("t=%1s").arg(xd, 0, 'f', 2)
                             : QString("帧=%1").arg(quint64(newestFrame - (tNewest - xd) / period));
        QString txt = QString("%1  v=%2").arg(sx).arg(yd, 0, 'g', 5);

        QFontMetrics fm(p.font());
        int bw = fm.horizontalAdvance(txt) + 12;
        int bx = int(m_cursor.x()) + 12;
        int by = int(m_cursor.y()) - 22;
        if (bx + bw > plotRect.right()) bx = plotRect.right() - bw;
        if (by < plotRect.top()) by = plotRect.top() + 2;
        p.fillRect(bx, by, bw, 16, QColor(20, 30, 40, 220));
        p.setPen(QColor("#e6eef2"));
        p.drawText(QRect(bx, by, bw, 16), Qt::AlignCenter, txt);
    }
}

// ===================== PlotWindow =====================
PlotWindow::PlotWindow(QWidget *parent, Qt::WindowFlags flags) : QWidget(parent, flags)
{
    setWindowTitle("实时绘图");
    resize(1280, 800);

    setStyleSheet(R"(
        QWidget { background:#1a242c; }
        QLabel { background:transparent; color:#dce6ec; font-size:15px; }
        QCheckBox { background:transparent; color:#dce6ec; font-size:15px; }
        QComboBox {
            background:#2e3c46; color:#e6eef2; border:1px solid #4a5b66;
            border-radius:4px; padding:4px 8px; min-height:22px;
        }
        QComboBox QAbstractItemView {
            background:#223039; color:#e6eef2; selection-background-color:#33434e;
        }
        QListWidget {
            background:#152029; color:#dce6ec; border:1px solid #4a5b66; border-radius:4px;
        }
        QListWidget::item { padding:3px 4px; }
        QListWidget::item:selected { background:#33434e; }
        QDoubleSpinBox, QSpinBox {
            background:#2e3c46; color:#e6eef2; border:1px solid #4a5b66;
            border-radius:4px; padding:3px 6px;
        }
        QPushButton {
            background:#2e3c46; color:#e6eef2; border:1px solid #4a5b66;
            border-radius:4px; padding:5px 12px;
        }
        QPushButton:hover { background:#37474f; }
    )");

    m_clock.start();

    QHBoxLayout *root = new QHBoxLayout(this);
    root->setContentsMargins(8, 8, 8, 8);
    root->setSpacing(8);

    QWidget *side = new QWidget(this);
    side->setFixedWidth(250);
    QVBoxLayout *panel = new QVBoxLayout(side);
    panel->setContentsMargins(0, 0, 0, 0);
    panel->setSpacing(6);

    panel->addWidget(new QLabel("选择帧"));
    m_frameCombo = new QComboBox(side);
    panel->addWidget(m_frameCombo);

    panel->addWidget(new QLabel("勾选变量（可多选）"));
    m_fieldList = new QListWidget(side);
    m_fieldList->setSelectionMode(QAbstractItemView::NoSelection);
    panel->addWidget(m_fieldList, 1);

    m_autoY = new QCheckBox("Y轴自动调整", side);
    m_autoY->setChecked(true);
    panel->addWidget(m_autoY);

    QHBoxLayout *yr = new QHBoxLayout();
    yr->addWidget(new QLabel("Ymin"));
    m_yMinSpin = new QDoubleSpinBox(side);
    m_yMinSpin->setRange(-1e9, 1e9);
    m_yMinSpin->setDecimals(6);
    m_yMinSpin->setValue(-1);
    yr->addWidget(m_yMinSpin, 1);
    yr->addWidget(new QLabel("Ymax"));
    m_yMaxSpin = new QDoubleSpinBox(side);
    m_yMaxSpin->setRange(-1e9, 1e9);
    m_yMaxSpin->setDecimals(6);
    m_yMaxSpin->setValue(1);
    yr->addWidget(m_yMaxSpin, 1);
    panel->addLayout(yr);

    QHBoxLayout *pr = new QHBoxLayout();
    pr->addWidget(new QLabel("缓存点数"));
    m_pointsSpin = new QSpinBox(side);
    m_pointsSpin->setRange(10, 100000);
    m_pointsSpin->setValue(5000);
    pr->addWidget(m_pointsSpin, 1);
    panel->addLayout(pr);

    QHBoxLayout *xr = new QHBoxLayout();
    xr->addWidget(new QLabel("X轴单位"));
    m_xUnitCombo = new QComboBox(side);
    m_xUnitCombo->addItems({"帧数", "时间"});
    xr->addWidget(m_xUnitCombo, 1);
    panel->addLayout(xr);

    QHBoxLayout *tr = new QHBoxLayout();
    tr->addWidget(new QLabel("时间窗口(s)"));
    m_timeSpanSpin = new QDoubleSpinBox(side);
    m_timeSpanSpin->setRange(1.0, 3600.0);
    m_timeSpanSpin->setValue(30.0);
    tr->addWidget(m_timeSpanSpin, 1);
    panel->addLayout(tr);

    m_pauseBtn = new QPushButton("暂停", side);
    panel->addWidget(m_pauseBtn);

    QPushButton *clearBtn = new QPushButton("清空曲线", side);
    panel->addWidget(clearBtn);

    panel->addStretch(1);
    root->addWidget(side);

    m_plot = new PlotWidget(this);
    root->addWidget(m_plot, 1);

    connect(m_frameCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &PlotWindow::onFrameSelected);
    connect(m_fieldList, &QListWidget::itemChanged, this, &PlotWindow::onFieldToggled);
    connect(m_autoY, &QCheckBox::toggled, this, &PlotWindow::onAutoYToggled);
    connect(m_yMinSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [this](double){ onYRangeChanged(); });
    connect(m_yMaxSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [this](double){ onYRangeChanged(); });
    connect(m_pointsSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &PlotWindow::onPointsChanged);
    connect(m_xUnitCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &PlotWindow::onXUnitChanged);
    connect(m_timeSpanSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &PlotWindow::onTimeSpanChanged);
    connect(m_pauseBtn, &QPushButton::clicked, this, &PlotWindow::togglePause);
    connect(clearBtn, &QPushButton::clicked, this, &PlotWindow::clearAll);

    onAutoYToggled(true);
    onXUnitChanged(0);
    refreshFrames();
}

void PlotWindow::refreshFrames()
{
    for (PlotSeries *s : m_series) delete s;
    m_series.clear();
    m_plot->setSeries(m_series);
    m_curPktIdx = -1;

    m_frameCombo->blockSignals(true);
    m_frameCombo->clear();
    const QList<ProtoPacket> &pkts = g_GlobalProtoModel.packetList;
    for (int i = 0; i < pkts.size(); i++) {
        QString t = pkts[i].windowTitle.isEmpty() ? QString("帧%1").arg(i + 1) : pkts[i].windowTitle;
        m_frameCombo->addItem(QString("%1. %2").arg(i + 1).arg(t));
    }
    m_frameCombo->blockSignals(false);

    if (m_frameCombo->count() > 0) {
        m_frameCombo->setCurrentIndex(0);
        onFrameSelected(0);
    } else {
        rebuildFieldList();
    }
}

void PlotWindow::onFrameSelected(int idx)
{
    if (idx < 0 || idx >= g_GlobalProtoModel.packetList.size()) return;
    m_curPktIdx = idx;
    rebuildFieldList();
}

void PlotWindow::rebuildFieldList()
{
    m_updating = true;
    m_fieldList->clear();
    if (m_curPktIdx >= 0 && m_curPktIdx < g_GlobalProtoModel.packetList.size()) {
        const ProtoPacket &pkt = g_GlobalProtoModel.packetList[m_curPktIdx];
        for (int i = 0; i < pkt.fieldList.size(); i++) {
            if (pkt.fieldList[i].isString) continue;
            QListWidgetItem *it = new QListWidgetItem(pkt.fieldList[i].showName);
            it->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
            it->setCheckState(findSeries(m_curPktIdx, i) ? Qt::Checked : Qt::Unchecked);
            it->setData(Qt::UserRole, i);
            m_fieldList->addItem(it);
        }
    }
    m_updating = false;
}

PlotSeries* PlotWindow::findSeries(int pktIdx, int fieldIdx)
{
    for (PlotSeries *s : m_series)
        if (s->pktIdx == pktIdx && s->fieldIdx == fieldIdx) return s;
    return nullptr;
}

void PlotWindow::onFieldToggled(QListWidgetItem *item)
{
    if (m_updating) return;
    if (m_curPktIdx < 0 || m_curPktIdx >= g_GlobalProtoModel.packetList.size()) return;

    int fieldIdx = item->data(Qt::UserRole).toInt();
    bool checked = (item->checkState() == Qt::Checked);

    if (checked) {
        if (findSeries(m_curPktIdx, fieldIdx)) return;
        const ProtoPacket &pkt = g_GlobalProtoModel.packetList[m_curPktIdx];
        if (fieldIdx < 0 || fieldIdx >= pkt.fieldList.size()) return;

        PlotSeries *s = new PlotSeries;
        s->pktIdx = m_curPktIdx;
        s->fieldIdx = fieldIdx;
        s->name = pkt.fieldList[fieldIdx].showName;
        if (!pkt.fieldList[fieldIdx].unit.isEmpty())
            s->name += "(" + pkt.fieldList[fieldIdx].unit + ")";
        s->color = g_seriesColors[m_series.size() % 8];
        s->maxPoints = m_pointsSpin->value();
        m_series.append(s);
    } else {
        PlotSeries *s = findSeries(m_curPktIdx, fieldIdx);
        if (s) {
            m_series.removeAll(s);
            delete s;
        }
    }
    m_plot->setSeries(m_series);
}

void PlotWindow::onFrameData(int pktIdx, const QVector<double> &nums)
{
    if (m_paused) return;   // 暂停时冻结
    double t = m_clock.elapsed() / 1000.0;
    for (PlotSeries *s : m_series) {
        if (s->pktIdx == pktIdx && s->fieldIdx >= 0 && s->fieldIdx < nums.size())
            s->append(nums[s->fieldIdx], t);
    }
}

void PlotWindow::onAutoYToggled(bool on)
{
    m_yMinSpin->setEnabled(!on);
    m_yMaxSpin->setEnabled(!on);
    m_plot->setAutoY(on);
}

void PlotWindow::onYRangeChanged()
{
    double mn = m_yMinSpin->value();
    double mx = m_yMaxSpin->value();
    if (mn >= mx) mx = mn + 1e-6;
    m_plot->setYRange(mn, mx);
}

void PlotWindow::onPointsChanged(int n)
{
    for (PlotSeries *s : m_series) s->maxPoints = n;
    m_plot->setMaxPoints(n);
}

void PlotWindow::onXUnitChanged(int idx)
{
    bool timeMode = (idx == 1);
    m_timeSpanSpin->setEnabled(timeMode);
    m_plot->setXTime(timeMode);
}

void PlotWindow::onTimeSpanChanged(double sec)
{
    m_plot->setTimeSpan(sec);
}

void PlotWindow::togglePause()
{
    m_paused = !m_paused;
    m_pauseBtn->setText(m_paused ? "继续" : "暂停");
    m_plot->setInteractive(m_paused);
}

void PlotWindow::clearAll()
{
    for (PlotSeries *s : m_series) s->clear();
    m_plot->update();
}
