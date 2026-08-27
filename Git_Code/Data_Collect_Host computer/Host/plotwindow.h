#ifndef PLOTWINDOW_H
#define PLOTWINDOW_H

#include <QWidget>
#include <QList>
#include <QVector>
#include <QColor>
#include <QTimer>
#include <QElapsedTimer>
#include <QPoint>
#include <cmath>
#include "protocolloader.h"

class QComboBox;
class QListWidget;
class QListWidgetItem;
class QCheckBox;
class QDoubleSpinBox;
class QSpinBox;
class QPushButton;

// 一条曲线的数据（对应某个帧的某个变量）
class PlotSeries
{
public:
    int pktIdx = -1;
    int fieldIdx = -1;
    QString name;
    QColor color;
    QVector<double> data;
    QVector<double> tSec;        // 每点时间戳（秒，自绘图窗口打开起）
    quint64 frameCount = 0;      // 累计接收帧数（不随截断减少）
    int maxPoints = 5000;

    void append(double v, double t)
    {
        if (std::isnan(v)) return;      // 跳过无效值
        data.append(v);
        tSec.append(t);
        frameCount++;
        // 翻倍才批量裁剪，避免每次 O(n) 搬移导致卡顿
        if (data.size() > maxPoints * 2) {
            int over = data.size() - maxPoints;
            data.remove(0, over);
            tSec.remove(0, over);
        }
    }
    void clear() { data.clear(); tSec.clear(); }
};

// 曲线绘制控件：X 轴帧数/时间两种单位，按时间对齐、从右往左滚动；
// 30ms 定时重绘 + 密集数据逐像素 min/max 抽取，大数据不卡顿；
// 支持暂停后缩放/平移/读数
class PlotWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PlotWidget(QWidget *parent = nullptr);

    void setSeries(const QList<PlotSeries*> &s);
    void setAutoY(bool on);
    void setYRange(double mn, double mx);
    void setMaxPoints(int n);
    void setXTime(bool on);          // true=时间轴, false=帧数轴
    void setTimeSpan(double sec);
    void setInteractive(bool on);    // 暂停时进入交互（缩放/平移/读数）
    void resetView();                // 复位缩放/平移

protected:
    void paintEvent(QPaintEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    QList<PlotSeries*> m_series;
    bool m_autoY = true;
    double m_yMin = 0.0, m_yMax = 1.0;
    int m_maxPoints = 5000;
    bool m_xTime = false;
    double m_timeSpan = 30.0;
    QTimer *m_repaintTimer;

    // Y 轴平滑（扩展立即、收缩缓慢）
    double m_autoMin = 0.0, m_autoMax = 1.0;
    bool m_autoInit = false;
    QElapsedTimer m_paintClock;

    // 交互（暂停时）
    bool m_interactive = false;
    double m_zoomY = 1.0;      // Y 缩放
    double m_panY = 0.0;       // Y 平移（数据单位）
    double m_panXSec = 0.0;    // X 回看（秒）
    QPoint m_lastPos;
    bool m_dragging = false;
    QPointF m_cursor;
    bool m_cursorValid = false;
    double m_viewYMin = -1.0, m_viewYMax = 1.0, m_viewWinSec = 30.0; // 上次绘制的真实范围（平移换算用）
};

// 实时绘图窗口：选帧 → 勾选变量 → 自动绘制，可调坐标轴，可暂停观察
class PlotWindow : public QWidget
{
    Q_OBJECT
public:
    explicit PlotWindow(QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());

public slots:
    void refreshFrames();                          // 协议切换后刷新帧列表
    void onFrameData(int pktIdx, const QVector<double> &nums); // 实时数据接入

private slots:
    void onFrameSelected(int idx);
    void onFieldToggled(QListWidgetItem *item);
    void onAutoYToggled(bool on);
    void onYRangeChanged();
    void onPointsChanged(int n);
    void onXUnitChanged(int idx);
    void onTimeSpanChanged(double sec);
    void togglePause();
    void clearAll();

private:
    QComboBox *m_frameCombo;
    QListWidget *m_fieldList;
    PlotWidget *m_plot;
    QCheckBox *m_autoY;
    QDoubleSpinBox *m_yMinSpin;
    QDoubleSpinBox *m_yMaxSpin;
    QSpinBox *m_pointsSpin;
    QComboBox *m_xUnitCombo;
    QDoubleSpinBox *m_timeSpanSpin;
    QPushButton *m_pauseBtn;

    QElapsedTimer m_clock;
    QList<PlotSeries*> m_series;
    int m_curPktIdx = -1;
    bool m_updating = false;
    bool m_paused = false;

    void rebuildFieldList();
    PlotSeries* findSeries(int pktIdx, int fieldIdx);
};

#endif // PLOTWINDOW_H
