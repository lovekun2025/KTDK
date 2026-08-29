#include "batchsendwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QCheckBox>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QCloseEvent>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QRegularExpression>

BatchSendWindow::BatchSendWindow(QSerialPort *port, QWidget *parent)
    : QWidget(parent), m_port(port)
{
    setWindowTitle("批量发送");
    setMinimumSize(780, 500);
    setStyleSheet(
        "QWidget { background:#223039; }"
        "QLabel { color:#e6eef2; font-size:14px; }"
        "QListWidget { background:#2e3c46; color:#e6eef2; border:1px solid #4a5b66; font-size:14px; }"
        "QListWidget::item:selected { background:#33434e; }"
        "QTableWidget { background:#26333d; color:#e6eef2; border:1px solid #4a5b66; font-size:14px; gridline-color:#3d515c; }"
        "QHeaderView::section { background:#1c2830; color:#dce6ec; border:1px solid #3d515c; font-size:13px; padding:4px; }"
        "QTableWidget::item:selected { background:#33434e; }"
        "QSpinBox { background:#2e3c46; color:#e6eef2; border:1px solid #4a5b66; font-size:14px; }"
        "QPushButton { background:#2e3c46; color:#e6eef2; border:1px solid #4a5b66; border-radius:4px; padding:5px 12px; font-size:14px; }"
        "QPushButton:hover { background:#37474f; }"
    );

    m_savePath = QCoreApplication::applicationDirPath() + "/batchsend.json";

    // 左侧页列表
    m_pageList = new QListWidget();
    m_pageList->setFixedWidth(140);
    m_btnAddPage = new QPushButton("添加页");
    m_btnRenamePage = new QPushButton("重命名");
    m_btnDelPage = new QPushButton("删除页");
    m_btnEcho = new QPushButton("数据回传");
    QVBoxLayout *left = new QVBoxLayout();
    left->addWidget(m_pageList);
    left->addWidget(m_btnAddPage);
    left->addWidget(m_btnRenamePage);
    left->addWidget(m_btnDelPage);
    left->addWidget(m_btnEcho);

    // 表格：编号 | 延迟 | 重复 | 数据 | 备注 | 发送
    m_table = new QTableWidget(0, ColCount);
    m_table->setHorizontalHeaderLabels({"编号", "延迟(ms)", "重复次数", "数据", "备注", "发送"});
    m_table->horizontalHeader()->setSectionResizeMode(ColCheck, QHeaderView::Fixed);
    m_table->horizontalHeader()->setSectionResizeMode(ColDelay, QHeaderView::Fixed);
    m_table->horizontalHeader()->setSectionResizeMode(ColRepeat, QHeaderView::Fixed);
    m_table->horizontalHeader()->setSectionResizeMode(ColData, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(ColRemark, QHeaderView::Fixed);
    m_table->horizontalHeader()->setSectionResizeMode(ColSend, QHeaderView::Fixed);
    m_table->setColumnWidth(ColCheck, 60);
    m_table->setColumnWidth(ColDelay, 90);
    m_table->setColumnWidth(ColRepeat, 90);
    m_table->setColumnWidth(ColRemark, 110);
    m_table->setColumnWidth(ColSend, 70);
    m_table->verticalHeader()->setVisible(false);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::DoubleClicked
                           | QAbstractItemView::EditKeyPressed
                           | QAbstractItemView::AnyKeyPressed);

    // 底部控制栏
    m_btnAddRow = new QPushButton("+");
    m_btnAddRow->setFixedWidth(40);
    m_btnDelRow = new QPushButton("x");
    m_btnDelRow->setFixedWidth(40);
    m_led = new QLabel();
    m_led->setFixedSize(14, 14);
    setLed(false);

    m_chkCR = new QCheckBox("自动回车");
    m_chkCR->setChecked(false);

    QHBoxLayout *ctrlRow = new QHBoxLayout();
    ctrlRow->addWidget(new QLabel("行:"));
    ctrlRow->addWidget(m_btnAddRow);
    ctrlRow->addWidget(m_btnDelRow);
    ctrlRow->addSpacing(12);
    ctrlRow->addWidget(m_chkCR);
    ctrlRow->addSpacing(12);
    ctrlRow->addWidget(new QLabel("循环次数:"));
    m_spLoop = new QSpinBox();
    m_spLoop->setRange(1, 9999);
    m_spLoop->setValue(1);
    ctrlRow->addWidget(m_spLoop);

    m_labStatus = new QLabel("就绪");
    m_labStatus->setStyleSheet("color:#7fd4ff;");
    ctrlRow->addWidget(m_labStatus);
    ctrlRow->addStretch(1);

    m_btnStart = new QPushButton("启动批量发送");
    m_btnStart->setStyleSheet("background:#1d6f42; padding:8px 20px; font-size:15px; font-weight:bold;");
    m_btnStop = new QPushButton("停止");
    m_btnStop->setEnabled(false);
    ctrlRow->addWidget(m_led);
    ctrlRow->addWidget(m_btnStart);
    ctrlRow->addWidget(m_btnStop);

    QVBoxLayout *right = new QVBoxLayout();
    right->addWidget(m_table, 1);
    right->addLayout(ctrlRow);

    QHBoxLayout *main = new QHBoxLayout(this);
    main->setContentsMargins(8, 8, 8, 8);
    main->addLayout(left);
    main->addLayout(right, 1);

    m_sendTimer = new QTimer(this);
    m_sendTimer->setSingleShot(false);
    connect(m_sendTimer, &QTimer::timeout, this, &BatchSendWindow::sendNext);

    connect(m_btnAddPage, &QPushButton::clicked, this, [this]() {
        addPage(QString("页 %1").arg(m_pages.size() + 1));
    });
    connect(m_btnRenamePage, &QPushButton::clicked, this, [this]() {
        if (m_cur < 0) return;
        bool ok = false;
        QString name = QInputDialog::getText(this, "重命名", "页名称：",
                                             QLineEdit::Normal, m_pages[m_cur].name, &ok);
        if (ok && !name.trimmed().isEmpty()) {
            m_pages[m_cur].name = name.trimmed();
            refreshList();
        }
    });
    connect(m_btnDelPage, &QPushButton::clicked, this, [this]() {
        if (m_cur < 0) return;
        m_pages.removeAt(m_cur);
        if (m_pages.isEmpty()) { addPage("页 1"); return; }
        if (m_cur >= m_pages.size()) m_cur = m_pages.size() - 1;
        refreshList();
        switchPage(m_cur);
    });
    connect(m_btnEcho, &QPushButton::clicked, this, [this]() {
        if (m_cur < 0) return;
        saveCurrentToPage();
        const Page &p = m_pages[m_cur];
        QString text = QString("== %1 数据回传 ==").arg(p.name);
        for (int i = 0; i < p.rows.size(); i++) {
            const Row &row = p.rows[i];
            text += QString("\n[%1] %2 | 延迟%3ms | 重复%4 | 备注:%5")
                        .arg(row.enabled ? "√" : " ")
                        .arg(row.data.isEmpty() ? "(空)" : row.data)
                        .arg(row.delay).arg(row.repeat).arg(row.remark);
        }
        emit signalEchoData(text);
    });
    connect(m_pageList, &QListWidget::currentRowChanged, this, &BatchSendWindow::switchPage);
    connect(m_btnAddRow, &QPushButton::clicked, this, [this]() { addRow(); });
    connect(m_btnDelRow, &QPushButton::clicked, this, &BatchSendWindow::delRow);
    connect(m_btnStart, &QPushButton::clicked, this, &BatchSendWindow::onStartClicked);
    connect(m_btnStop, &QPushButton::clicked, this, [this]() {
        m_sendTimer->stop();
        m_sending = false;
        setLed(false);
        m_btnStart->setEnabled(true);
        m_btnStop->setEnabled(false);
        m_labStatus->setText("已停止");
    });

    // 加载上次数据
    loadFromFile();
    if (m_pages.isEmpty()) addPage("页 1");
    if (m_pages.first().rows.isEmpty()) { m_pages.first().rows.append(Row()); m_pages.first().rows.append(Row()); }
    refreshList();
    switchPage(0);
}

void BatchSendWindow::closeEvent(QCloseEvent *e)
{
    saveToFile();   // 自动保存所有页/行数据
    QWidget::closeEvent(e);
}

// ===================== 页管理 =====================
void BatchSendWindow::addPage(const QString &name)
{
    saveCurrentToPage();
    Page p;
    p.name = name;
    m_pages.append(p);
    refreshList();
    m_pageList->setCurrentRow(m_pages.size() - 1);
}

void BatchSendWindow::switchPage(int idx)
{
    if (idx < 0 || idx >= m_pages.size()) return;
    saveCurrentToPage();
    m_cur = idx;
    loadPageToTable(idx);
    update();
}

void BatchSendWindow::saveCurrentToPage()
{
    if (m_cur < 0 || m_cur >= m_pages.size()) return;
    Page &p = m_pages[m_cur];
    p.rows.clear();
    p.loop = m_spLoop->value();
    for (int r = 0; r < m_table->rowCount(); r++) {
        Row row;
        QWidget *w = m_table->cellWidget(r, ColCheck);
        QCheckBox *ck = w ? w->findChild<QCheckBox*>() : nullptr;
        row.enabled = ck ? ck->isChecked() : true;
        row.delay = m_table->item(r, ColDelay) ? m_table->item(r, ColDelay)->text().toInt() : 200;
        row.repeat = m_table->item(r, ColRepeat) ? m_table->item(r, ColRepeat)->text().toInt() : 1;
        row.data = m_table->item(r, ColData) ? m_table->item(r, ColData)->text() : QString();
        row.remark = m_table->item(r, ColRemark) ? m_table->item(r, ColRemark)->text() : QString();
        p.rows << row;
    }
}

void BatchSendWindow::loadPageToTable(int idx)
{
    m_table->setRowCount(0);
    const Page &p = m_pages[idx];
    m_spLoop->setValue(p.loop);
    for (const Row &row : p.rows)
        addRow(row);
    renumber();
}

void BatchSendWindow::refreshList()
{
    m_pageList->blockSignals(true);
    m_pageList->clear();
    for (const Page &p : m_pages)
        m_pageList->addItem(p.name);
    if (m_cur >= 0 && m_cur < m_pages.size())
        m_pageList->setCurrentRow(m_cur);
    m_pageList->blockSignals(false);
    bool has = m_pages.size() > 0;
    m_btnDelPage->setEnabled(has);
    m_btnRenamePage->setEnabled(has);
}

// ===================== 表格行操作 =====================
void BatchSendWindow::addRow(const Row &row)
{
    int r = m_table->rowCount();
    m_table->insertRow(r);

    QCheckBox *ck = new QCheckBox();
    ck->setChecked(row.enabled);
    QWidget *wrap = new QWidget();
    QHBoxLayout *hl = new QHBoxLayout(wrap);
    hl->setContentsMargins(6, 0, 0, 0);
    hl->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    QLabel *lab = new QLabel(QString("%1").arg(r + 1, 2, 10, QChar('0')));
    lab->setStyleSheet("background:transparent; color:#7fd4ff; font-size:14px;");
    hl->addWidget(ck);
    hl->addWidget(lab);
    hl->addStretch(1);
    m_table->setCellWidget(r, ColCheck, wrap);

    QTableWidgetItem *itDelay = new QTableWidgetItem(QString::number(row.delay));
    itDelay->setTextAlignment(Qt::AlignCenter);
    m_table->setItem(r, ColDelay, itDelay);

    QTableWidgetItem *itRepeat = new QTableWidgetItem(QString::number(row.repeat));
    itRepeat->setTextAlignment(Qt::AlignCenter);
    m_table->setItem(r, ColRepeat, itRepeat);

    m_table->setItem(r, ColData, new QTableWidgetItem(row.data));
    m_table->setItem(r, ColRemark, new QTableWidgetItem(row.remark));
    makeSendButton(r);
}

void BatchSendWindow::addRow()
{
    saveCurrentToPage(); // 确保行数据先落到当前页（避免表格与页不一致）
    Row row;
    m_pages[m_cur].rows.append(row);
    addRow(row);
    renumber();
}

void BatchSendWindow::delRow()
{
    int r = m_table->currentRow();
    if (r < 0 || m_table->rowCount() <= 0) return;
    m_table->removeRow(r);
    saveCurrentToPage();
    renumber();
}

void BatchSendWindow::renumber()
{
    for (int r = 0; r < m_table->rowCount(); r++) {
        QWidget *w = m_table->cellWidget(r, ColCheck);
        QLabel *lab = w ? w->findChild<QLabel*>() : nullptr;
        if (lab) lab->setText(QString("%1").arg(r + 1, 2, 10, QChar('0')));
    }
}

void BatchSendWindow::makeSendButton(int row)
{
    QPushButton *btn = new QPushButton("发送");
    btn->setStyleSheet("background:#1d6f42; border:1px solid #2e9e5c; color:#ffffff;");
    btn->setMinimumHeight(24);
    connect(btn, &QPushButton::clicked, this, [this, row]() {
        if (!m_port || !m_port->isOpen()) { m_labStatus->setText("串口未打开"); return; }
        QTableWidgetItem *d = m_table->item(row, ColData);
        if (!d || d->text().trimmed().isEmpty()) { m_labStatus->setText("该行没有数据"); return; }
        sendData(encodeText(d->text()));
        m_labStatus->setText("已发送单行");
    });
    m_table->setCellWidget(row, ColSend, btn);
}

QByteArray BatchSendWindow::encodeText(const QString &t)
{
    QString s = t.trimmed();
    if (s.isEmpty()) return QByteArray();
    bool hexOnly = true;
    for (const QChar &c : s) {
        if (c.isSpace() || c == ',') continue;
        if (c.isLetter() && c.isLetter()) hexOnly = false;
    }
    QByteArray out;
    if (hexOnly) {
        QString h = s;
        h.remove(QRegularExpression("[^0-9a-fA-F]"));
        out = QByteArray::fromHex(h.toUtf8());
    } else {
        out = s.toUtf8();
    }
    // 自动回车
    if (m_chkCR->isChecked()) {
        if (hexOnly) out.append(char(0x0D)).append(char(0x0A));
        else out.append("\r\n");
    }
    return out;
}

void BatchSendWindow::sendData(const QByteArray &d)
{
    if (m_port && m_port->isOpen() && !d.isEmpty()) {
        m_port->write(d);
        emit signalTxBytes(d.size());   // 同步到主窗口收发统计
    }
}

// ===================== 批量发送 =====================
void BatchSendWindow::onStartClicked()
{
    if (!m_port || !m_port->isOpen()) {
        m_labStatus->setText("串口未打开");
        return;
    }
    saveCurrentToPage();
    m_sendRows.clear();
    const Page &p = m_pages[m_cur];
    for (const Row &row : p.rows) {
        if (!row.enabled) continue;
        if (row.data.trimmed().isEmpty()) continue;
        QByteArray data = encodeText(row.data);
        if (data.isEmpty()) continue;
        Row t = row;
        m_sendRows << t;
    }
    if (m_sendRows.isEmpty()) {
        m_labStatus->setText("没有勾选的有效数据行");
        return;
    }

    m_rowIndex = 0;
    m_repeatRemain = m_sendRows[0].repeat;
    m_loopRemain = p.loop;
    m_sending = true;
    setLed(true);
    m_btnStart->setEnabled(false);
    m_btnStop->setEnabled(true);
    m_labStatus->setText(QString("发送中：%1 行 × %2 次").arg(m_sendRows.size()).arg(m_loopRemain));
    m_sendTimer->start(m_sendRows[0].delay);
    sendNext();
}

void BatchSendWindow::sendNext()
{
    if (!m_sending || m_rowIndex >= m_sendRows.size()) return;

    const Row &t = m_sendRows[m_rowIndex];
    sendData(encodeText(t.data));
    m_repeatRemain--;

    if (m_repeatRemain > 0) {
        m_sendTimer->start(t.delay);
        return;
    }
    m_rowIndex++;
    if (m_rowIndex < m_sendRows.size()) {
        m_repeatRemain = m_sendRows[m_rowIndex].repeat;
        m_sendTimer->start(m_sendRows[m_rowIndex].delay);
        return;
    }
    m_loopRemain--;
    if (m_loopRemain <= 0) {
        m_sendTimer->stop();
        m_sending = false;
        setLed(false);
        m_btnStart->setEnabled(true);
        m_btnStop->setEnabled(false);
        m_labStatus->setText("发送完成");
        return;
    }
    m_rowIndex = 0;
    m_repeatRemain = m_sendRows[0].repeat;
    m_sendTimer->start(m_sendRows[0].delay);
}

void BatchSendWindow::setLed(bool on)
{
    m_led->setStyleSheet(on
        ? "background:#2ecc71; border-radius:7px;"
        : "background:#45555e; border-radius:7px;");
}

// ===================== 自动保存/加载 =====================
void BatchSendWindow::saveToFile()
{
    saveCurrentToPage();
    QJsonArray pagesArr;
    for (const Page &p : m_pages) {
        QJsonObject po;
        po["name"] = p.name;
        po["loop"] = p.loop;
        QJsonArray rowsArr;
        for (const Row &row : p.rows) {
            QJsonObject ro;
            ro["enabled"] = row.enabled;
            ro["delay"] = row.delay;
            ro["repeat"] = row.repeat;
            ro["data"] = row.data;
            ro["remark"] = row.remark;
            rowsArr.append(ro);
        }
        po["rows"] = rowsArr;
        pagesArr.append(po);
    }
    QJsonObject root;
    root["pages"] = pagesArr;
    QFile f(m_savePath);
    if (f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        f.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
        f.close();
    }
}

void BatchSendWindow::loadFromFile()
{
    QFile f(m_savePath);
    if (!f.open(QIODevice::ReadOnly)) return;
    QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    f.close();
    if (!doc.isObject()) return;
    QJsonArray pagesArr = doc.object()["pages"].toArray();
    for (const QJsonValue &v : pagesArr) {
        QJsonObject po = v.toObject();
        Page p;
        p.name = po["name"].toString();
        p.loop = po["loop"].toInt(1);
        for (const QJsonValue &rv : po["rows"].toArray()) {
            QJsonObject ro = rv.toObject();
            Row row;
            row.enabled = ro["enabled"].toBool(true);
            row.delay = ro["delay"].toInt(200);
            row.repeat = ro["repeat"].toInt(1);
            row.data = ro["data"].toString();
            row.remark = ro["remark"].toString();
            p.rows << row;
        }
        m_pages << p;
    }
}
