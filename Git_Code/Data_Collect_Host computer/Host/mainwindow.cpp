#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "plotwindow.h"
#include <QDockWidget>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QFileDialog>
#include <QStandardPaths>
#include <QFileInfo>
#include <QDir>
#include <QCoreApplication>
#include <QDateTime>
#include <QPainter>
#include <QPixmap>
#include <QDebug>
#include <QProcess>
#include <QScrollArea>

// 字节数转可读字符串
static QString fmtBytes(quint64 b)
{
    if (b < 1024)                    return QString::number(b) + " B";
    if (b < 1024 * 1024)             return QString::number(double(b) / 1024.0, 'f', 1) + " KB";
    if (b < 1024ull * 1024 * 1024)   return QString::number(double(b) / (1024.0 * 1024), 'f', 2) + " MB";
    return QString::number(double(b) / (1024.0 * 1024 * 1024), 'f', 2) + " GB";
}

// 去掉文件名里的非法字符
static QString sanitizeFilename(const QString &name)
{
    QString s = name;
    const char* invalid = "\\/:*?\"<>|";
    for (const char* p = invalid; *p; ++p) s.remove(QChar(*p));
    s = s.trimmed();
    if (s.isEmpty()) s = "未命名";
    return s;
}

// 帧主题色：0 = 天青蓝（航姿主帧，渐变），其余为多巴胺亮色
static const QColor g_frameAccents[] = {
    QColor("#00a8c8"), // 0 天青蓝
    QColor("#ff5c8a"), // 1 粉
    QColor("#ff9f40"), // 2 橙
    QColor("#f4c531"), // 3 黄
    QColor("#9b5de5"), // 4 紫
    QColor("#2ecc71"), // 5 绿
    QColor("#f15bb5"), // 6 玫红
    QColor("#12a4a4"), // 7 青
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("数据采集上位机");
    if (ui->lineEdit) ui->lineEdit->hide(); // 隐藏 .ui 遗留的 RX 文本框

    // 丝滑停靠：动画 + 嵌套 + 标签页 + 拖拽分组
    setDockNestingEnabled(true);
    setDockOptions(QMainWindow::AnimatedDocks
                 | QMainWindow::AllowNestedDocks
                 | QMainWindow::AllowTabbedDocks
                 | QMainWindow::GroupedDragging);
    setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::North);

    // 顶栏固定
    ui->dockWidget_2->setAllowedAreas(Qt::TopDockWidgetArea);
    ui->dockWidget_2->setFeatures(QDockWidget::NoDockWidgetFeatures);
    ui->dockWidget_2->setFixedHeight(74);
    ui->dockWidget_2->setTitleBarWidget(new QWidget()); // 去掉顶栏标题条/边框

    // 接收环形缓冲（512KB）
    m_rxCycle = new CCycleBuffer(512 * 1024);
    m_ledElapsed.start();

    buildStatusBar();
    buildTopBar();
    setupMenus();
    applyStylesheet();

    m_protocolLoader = new ProtocolLoader(this);
    connect(m_protocolLoader, &ProtocolLoader::signalLoadProtoFinish,
            this, &MainWindow::onProtocolLoaded);

    refreshPortList();
    initializeGlobal();

    // 界面刷新节流（30ms），避免高频数据把 UI 刷爆
    m_uiTimer = new QTimer(this);
    m_uiTimer->setInterval(30);
    connect(m_uiTimer, &QTimer::timeout, this, &MainWindow::flushUi);
    m_uiTimer->start();

    // 每秒统计
    m_statsTimer = new QTimer(this);
    m_statsTimer->setInterval(1000);
    connect(m_statsTimer, &QTimer::timeout, this, &MainWindow::updateStats);
    m_statsTimer->start();
}

MainWindow::~MainWindow()
{
    for (QFile* f : m_saveFiles) {
        if (f) { if (f->isOpen()) f->close(); delete f; }
    }
    delete ui;
}

// ===================== 界面搭建 =====================
void MainWindow::setupMenus()
{
    // 协议文件子菜单（放在"打开协议文件"之上）
    QMenu *protoMenu = new QMenu("协议文件", ui->menulmy);
    ui->menulmy->addMenu(protoMenu);

    QAction *openProto = new QAction("打开协议文件...", this);
    openProto->setShortcut(QKeySequence::Open);
    connect(openProto, &QAction::triggered, this, &MainWindow::onOpenProtocolFile);
    ui->menulmy->addAction(openProto);

    // 工具：串口设置
    QAction *cfg = new QAction("串口设置...", this);
    connect(cfg, &QAction::triggered, this, &MainWindow::onOpenConfigDialog);
    ui->menu_2->addAction(cfg);

    // 工具：实时绘图
    QAction *plot = new QAction("实时绘图", this);
    connect(plot, &QAction::triggered, this, &MainWindow::openPlotWindow);
    ui->menu_2->addAction(plot);

    // 工具：固件升级（外部调用 BootApp.exe）
    QAction *boot = new QAction("固件升级...", this);
    connect(boot, &QAction::triggered, this, &MainWindow::onLaunchBootApp);
    ui->menu_2->addAction(boot);

    // 帮助：关于
    QAction *about = new QAction("关于", this);
    connect(about, &QAction::triggered, this, [this](){
        QMessageBox::about(this, "关于",
            "数据采集上位机\n\n"
            "使用步骤：\n"
            "1. 文件 → 打开协议文件（选择 XML 协议）\n"
            "2. 工具 → 串口设置（或顶栏直接选串口）\n"
            "3. 打开串口后实时解析显示\n"
            "4. 点\"保存\"可实时存储各帧数据到 Data 目录");
    });
    ui->menu_4->addAction(about);
}

void MainWindow::buildTopBar()
{
    QWidget *top = ui->widget;
    QHBoxLayout *lay = new QHBoxLayout(top);
    lay->setContentsMargins(12, 8, 12, 8);
    lay->setSpacing(8);

    lay->addWidget(new QLabel("串口", top));
    ui->comboBox->setMinimumHeight(28);
    lay->addWidget(ui->comboBox);

    lay->addWidget(new QLabel("波特率", top));
    m_comboBaud = new QComboBox(top);
    m_comboBaud->setMinimumHeight(28);
    m_comboBaud->addItems({"9600","19200","38400","57600","115200","230400","460800","921600"});
    m_comboBaud->setCurrentText("115200");
    lay->addWidget(m_comboBaud);

    ui->pushButton->setText("打开串口");
    ui->pushButton->setMinimumHeight(28);
    lay->addWidget(ui->pushButton);

    m_btnOpenProto = new QPushButton("打开协议", top);
    m_btnOpenProto->setMinimumHeight(28);
    lay->addWidget(m_btnOpenProto);

    m_btnSave = new QPushButton("保存", top);
    m_btnSave->setMinimumHeight(28);
    lay->addWidget(m_btnSave);

    m_saveNameEdit = new QLineEdit(top);
    m_saveNameEdit->setPlaceholderText("文件夹备注");
    m_saveNameEdit->setMaximumWidth(140);
    m_saveNameEdit->setMinimumHeight(28);
    lay->addWidget(m_saveNameEdit);

    // 实时绘图
    QPushButton *btnPlot = new QPushButton("绘图", top);
    btnPlot->setMinimumHeight(28);
    lay->addWidget(btnPlot);
    connect(btnPlot, &QPushButton::clicked, this, &MainWindow::openPlotWindow);

    lay->addStretch(1);

    // 发送区
    m_sendEdit = new QLineEdit(top);
    m_sendEdit->setPlaceholderText("发送内容");
    m_sendEdit->setMinimumWidth(180);
    m_sendEdit->setMinimumHeight(28);
    lay->addWidget(m_sendEdit);

    m_sendHex = new QCheckBox("Hex", top);
    lay->addWidget(m_sendHex);

    m_btnSend = new QPushButton("发送", top);
    m_btnSend->setMinimumHeight(28);
    lay->addWidget(m_btnSend);

    connect(ui->pushButton, &QPushButton::clicked, this, &MainWindow::toggleSerial);
    connect(m_btnOpenProto, &QPushButton::clicked, this, &MainWindow::onOpenProtocolFile);
    connect(m_btnSave, &QPushButton::clicked, this, &MainWindow::toggleSave);
    connect(m_btnSend, &QPushButton::clicked, this, &MainWindow::sendCurrentData);
    connect(m_sendEdit, &QLineEdit::returnPressed, this, &MainWindow::sendCurrentData);
}

void MainWindow::buildStatusBar()
{
    ui->statusbar->addWidget(new QLabel("串口 "));
    m_ledSerial = new QLabel;
    m_ledSerial->setFixedSize(11, 11);
    setLedColor(m_ledSerial, "#4a5158");
    ui->statusbar->addWidget(m_ledSerial);

    ui->statusbar->addWidget(new QLabel("  收 "));
    m_ledRx = new QLabel;
    m_ledRx->setFixedSize(11, 11);
    setLedColor(m_ledRx, "#4a5158");
    ui->statusbar->addWidget(m_ledRx);

    ui->statusbar->addWidget(new QLabel("  发 "));
    m_ledTx = new QLabel;
    m_ledTx->setFixedSize(11, 11);
    setLedColor(m_ledTx, "#4a5158");
    ui->statusbar->addWidget(m_ledTx);

    m_labStats = new QLabel("  等待打开串口");
    ui->statusbar->addPermanentWidget(m_labStats);
}

void MainWindow::applyStylesheet()
{
    // 全局基础字号 15px，界面更清晰
    setStyleSheet(R"(
        QMainWindow { background:#24313a; }
        QMenuBar { background:#1c2830; color:#dce6ec; font-size:15px; border:none; }
        QMenuBar::item { padding:5px 12px; }
        QMenuBar::item:selected { background:#2e3c46; }
        QMenu { background:#223039; color:#dce6ec; border:1px solid #3d515c; font-size:15px; }
        QMenu::item { padding:6px 28px 6px 14px; }
        QMenu::item:selected { background:#2e3c46; }
        QStatusBar { background:#1c2830; color:#a9b8c2; font-size:14px; }
        QDockWidget { color:#dce6ec; }
        QDockWidget::title { background:#1c2830; border:1px solid #3d515c; padding:6px 14px; font-size:15px; }
        QComboBox {
            background:#2e3c46; color:#e6eef2; border:1px solid #4a5b66;
            border-radius:4px; padding:5px 10px; min-height:26px; font-size:15px;
        }
        QComboBox:hover { border-color:#5e7280; }
        QComboBox QAbstractItemView {
            background:#223039; color:#e6eef2; selection-background-color:#33434e; font-size:15px;
        }
        QPushButton {
            background:#2e3c46; color:#e6eef2; border:1px solid #4a5b66;
            border-radius:4px; padding:6px 14px; min-height:26px; font-size:15px;
        }
        QPushButton:hover { background:#37474f; border-color:#5e7280; }
        QPushButton:pressed { background:#28343d; }
        QLineEdit {
            background:#26333d; color:#e6eef2; border:1px solid #4a5b66;
            border-radius:4px; padding:5px 8px; min-height:24px; font-size:15px;
        }
        QCheckBox { color:#dce6ec; font-size:15px; }
        QLabel { color:#dce6ec; font-size:15px; }
        QToolTip { background:#223039; color:#dce6ec; border:1px solid #4a5b66; font-size:14px; }
        QMessageBox { background:#223039; }
        QMessageBox QLabel { color:#e6eef2; font-size:15px; }
        QMessageBox QPushButton {
            background:#2e3c46; color:#e6eef2; border:1px solid #4a5b66;
            border-radius:4px; padding:6px 18px; min-height:26px; font-size:15px;
        }
        QMessageBox QPushButton:hover { background:#37474f; border-color:#5e7280; }
    )");
}

void MainWindow::initializeGlobal()
{
    QString appDir = QCoreApplication::applicationDirPath();
    QStringList candidates = {
        QDir(appDir).filePath("Protocols"),
        QDir(appDir).filePath("../Protocols"),
        QDir(appDir).filePath("../../Protocols"),
        QDir(appDir).filePath("../../../Protocols"),
    };
    QString protoFolder;
    for (const QString &c : candidates) {
        if (QDir(c).exists()) { protoFolder = c; break; }
    }
    if (protoFolder.isEmpty())
        protoFolder = QDir(appDir).filePath("Protocols");

    m_protocolLoader->loadProtocolToMenu(protoFolder, this, ui->menulmy);
}

// ===================== 协议加载 =====================
void MainWindow::onOpenProtocolFile()
{
    QString start = m_lastDir;
    if (start.isEmpty())
        start = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

    QString path = QFileDialog::getOpenFileName(this, "选择协议文件", start,
                                                "XML 协议 (*.xml);;所有文件 (*)");
    if (path.isEmpty()) return;

    QFileInfo fi(path);
    m_lastDir = fi.absolutePath();
    m_protocolLoader->loadProtocolFile(path);
}

void MainWindow::onProtocolLoaded(const ProtoXmlModel& model)
{
    stopSave();
    m_rxBuffer.clear();
    clearFrameDocks();

    int posCount = model.packetList.size();
    if (posCount == 0) {
        statusBar()->showMessage("该协议没有可显示的帧", 3000);
        return;
    }

    for (int i = 0; i < posCount; i++)
    {
        const ProtoPacket& pkt = model.packetList[i];
        QString title = pkt.windowTitle.isEmpty() ? QString("帧 %1").arg(i + 1) : pkt.windowTitle;

        QDockWidget* dock = new QDockWidget(title, this);
        dock->setObjectName(QString("frameDock_%1").arg(i));
        dock->setAllowedAreas(Qt::AllDockWidgetAreas);
        dock->setFeatures(QDockWidget::DockWidgetClosable
                        | QDockWidget::DockWidgetMovable
                        | QDockWidget::DockWidgetFloatable);
        dock->setWindowIcon(makeFrameIcon(i, title));
        dock->setMinimumWidth(146);

        AutoGridWidget* g = new AutoGridWidget();
        g->setHeaderTitle(title);
        QStringList names, units;
        for (const ProtoField& f : pkt.fieldList) {
            names << f.showName;
            units << f.unit;
        }
        g->setItems(names);
        g->setUnits(units);

        // 限高+滚动：内容超高时出现滚动条，帧框高度不会顶到串口号
        QScrollArea* scroll = new QScrollArea();
        scroll->setWidget(g);
        scroll->setWidgetResizable(true);
        scroll->setFrameShape(QFrame::NoFrame);
        scroll->setStyleSheet("QScrollArea { background:transparent; border:none; }");
        dock->setWidget(scroll);

        m_docks.append(dock);
        m_grids.append(g);
    }

    // 布局：第一帧竖直拆在顶栏下方，其余水平排列
    for (int i = 0; i < m_docks.size(); i++)
    {
        QDockWidget* d = m_docks[i];
        if (i == 0) {
            addDockWidget(Qt::TopDockWidgetArea, d);
            splitDockWidget(ui->dockWidget_2, d, Qt::Vertical);
        } else {
            addDockWidget(Qt::TopDockWidgetArea, d);
            splitDockWidget(m_docks[i - 1], d, Qt::Horizontal);
        }
    }

    refreshViewMenu();
    statusBar()->showMessage(QString("已加载协议：%1 帧").arg(posCount), 3000);
    emit signalProtoChanged();
}

void MainWindow::clearFrameDocks()
{
    for (QAction* a : m_viewActions)
        ui->menu->removeAction(a);
    m_viewActions.clear();

    for (QDockWidget* d : m_docks) {
        removeDockWidget(d);
        d->deleteLater();
    }
    m_docks.clear();
    m_grids.clear();
    m_pending.clear();
    m_lastRxMs.clear();
    m_frameHz.clear();
    m_frameHzCounts.clear();
}

void MainWindow::refreshViewMenu()
{
    for (int i = 0; i < m_docks.size(); i++)
    {
        QDockWidget* d = m_docks[i];
        QAction* a = d->toggleViewAction();
        if (a->text().isEmpty())
            a->setText(QString("窗口 %1").arg(i + 1));
        ui->menu->addAction(a);
        m_viewActions.append(a);
    }
}

// ===================== 串口 =====================
void MainWindow::refreshPortList()
{
    ui->comboBox->clear();
    const auto ports = QSerialPortInfo::availablePorts();
    for (const auto& p : ports)
        ui->comboBox->addItem(p.portName());
    if (ui->comboBox->count() == 0)
        ui->comboBox->addItem(QString());
}

void MainWindow::onOpenConfigDialog()
{
    SerialConfigDialog* dlg = new SerialConfigDialog(this);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    connect(dlg, &SerialConfigDialog::signalSerialConfig, this, &MainWindow::onSerialConfig);
    dlg->show();
}

// 工具 → 固件升级：外部调用 BootApp.exe（多位置查找）
void MainWindow::onLaunchBootApp()
{
    QString appDir = QCoreApplication::applicationDirPath();
    QStringList candidates;
    candidates << appDir + "/BootApp.exe"                                  // Host.exe 同目录
               << appDir + "/../Host_Release/BootApp.exe"                  // 工程发布目录
               << appDir + "/Host_Release/BootApp.exe"
               << appDir + "/../BootApp.exe"                               // 上一级
               << QDir::currentPath() + "/BootApp.exe";                    // 当前工作目录

    QString bootPath;
    for (const QString &c : candidates) {
        if (QFile::exists(c)) {
            bootPath = QFileInfo(c).absoluteFilePath();
            break;
        }
    }
    if (bootPath.isEmpty()) {
        QMessageBox::warning(this, "固件升级",
            QString("未找到 BootApp.exe，已检查以下位置：\n%1\n\n"
                    "请把 BootApp.exe 放到上位机 exe 同目录。")
                .arg(candidates.join("\n")));
        return;
    }

    // 已在运行则不再重复启动
    if (m_bootProc && m_bootProc->state() != QProcess::NotRunning) {
        QMessageBox::information(this, "固件升级", "BootApp 已在运行中");
        return;
    }

    if (!m_bootProc) {
        m_bootProc = new QProcess(this);
    }
    m_bootProc->setWorkingDirectory(QFileInfo(bootPath).absolutePath());
    m_bootProc->start(bootPath);
    if (!m_bootProc->waitForStarted(3000)) {
        QMessageBox::warning(this, "固件升级",
            QString("启动 BootApp.exe 失败：\n%1\n\n原因：%2").arg(bootPath, m_bootProc->errorString()));
    }
}

void MainWindow::toggleSerial()
{
    if (m_serialPort && m_serialPort->isOpen()) {
        m_serialPort->close();
        setSerialConnected(false);
        return;
    }
    SerialConfig cfg;
    cfg.portName = ui->comboBox->currentText();
    if (cfg.portName.isEmpty()) {
        QMessageBox::warning(this, "提示", "未检测到可用串口");
        return;
    }
    cfg.baudRate = m_comboBaud->currentText().toInt();
    cfg.dataBits = QSerialPort::Data8;
    cfg.parity = QSerialPort::NoParity;
    cfg.stopBits = QSerialPort::OneStop;
    openSerial(cfg);
}

void MainWindow::onSerialConfig(const SerialConfig& config)
{
    ui->comboBox->setCurrentText(config.portName);
    m_comboBaud->setCurrentText(QString::number(config.baudRate));
    openSerial(config);
}

void MainWindow::openSerial(const SerialConfig& config)
{
    if (!m_serialPort) {
        m_serialPort = new QSerialPort(this);
        connect(m_serialPort, &QSerialPort::readyRead, this, &MainWindow::onSerialReadyRead);
        connect(m_serialPort, &QSerialPort::errorOccurred, this, &MainWindow::onSerialError);
    }
    if (m_serialPort->isOpen())
        m_serialPort->close();

    m_serialPort->setPortName(config.portName);
    m_serialPort->setBaudRate(config.baudRate);
    m_serialPort->setDataBits(config.dataBits);
    m_serialPort->setParity(config.parity);
    m_serialPort->setStopBits(config.stopBits);
    m_serialPort->setFlowControl(QSerialPort::NoFlowControl);

    if (m_serialPort->open(QIODevice::ReadWrite)) {
        m_rxBuffer.clear();
        if (m_rxCycle) m_rxCycle->Empty();
        m_errCount = 0;
        setSerialConnected(true);
        statusBar()->showMessage(
            QString("串口已连接: %1 @ %2").arg(config.portName).arg(config.baudRate), 3000);
    } else {
        setSerialConnected(false);
        QMessageBox::warning(this, "串口错误",
            QString("无法打开 %1:\n%2").arg(config.portName).arg(m_serialPort->errorString()));
    }
}

void MainWindow::setSerialConnected(bool on)
{
    setLedColor(m_ledSerial, on ? "#2ecc71" : "#4a5158");
    ui->pushButton->setText(on ? "关闭串口" : "打开串口");
}

void MainWindow::onSerialError(QSerialPort::SerialPortError err)
{
    if (err == QSerialPort::ResourceError && m_serialPort && m_serialPort->isOpen()) {
        m_serialPort->close();
        setSerialConnected(false);
        statusBar()->showMessage("串口连接断开", 5000);
    }
}

void MainWindow::onSerialReadyRead()
{
    if (!m_serialPort) return;
    QByteArray data = m_serialPort->readAll();
    if (data.isEmpty()) return;

    m_rxTotal += data.size();
    m_rxThisSec += data.size();
    m_gotData = true;

    m_rxCycle->Write(reinterpret_cast<quint8*>(data.data()), quint32(data.size()));
    drainRxCycle();
}

void MainWindow::drainRxCycle()
{
    quint32 len = m_rxCycle->GetLength();
    if (len == 0) return;

    QByteArray chunk(int(len), Qt::Uninitialized);
    quint32 got = m_rxCycle->Read(reinterpret_cast<quint8*>(chunk.data()), len);
    chunk.resize(int(got));
    if (got > 0) {
        m_rxBuffer.append(chunk);
        parseRxBuffer();
    }
}

void MainWindow::sendCurrentData()
{
    if (!m_serialPort || !m_serialPort->isOpen()) {
        statusBar()->showMessage("串口未打开，无法发送", 3000);
        return;
    }

    QString txt = m_sendEdit->text();
    if (txt.isEmpty()) return;

    QByteArray out;
    if (m_sendHex->isChecked()) {
        QString h = txt;
        h.remove(' ');
        h.remove(',');
        out = QByteArray::fromHex(h.toUtf8());
    } else {
        out = txt.toUtf8();
    }
    if (out.isEmpty()) return;

    qint64 n = m_serialPort->write(out);
    if (n > 0) {
        m_txTotal += n;
        m_txThisSec += n;
    }
}

// ===================== 实时绘图 =====================
void MainWindow::openPlotWindow()
{
    PlotWindow *pw = new PlotWindow(this, Qt::Window);
    pw->setAttribute(Qt::WA_DeleteOnClose);
    connect(this, &MainWindow::signalFrameUpdate, pw, &PlotWindow::onFrameData);
    connect(this, &MainWindow::signalProtoChanged, pw, &PlotWindow::refreshFrames);
    pw->show();
}

// ===================== 实时存储 =====================
void MainWindow::toggleSave()
{
    if (m_saving) stopSave();
    else startSave();
}

void MainWindow::startSave()
{
    const QList<ProtoPacket>& pkts = g_GlobalProtoModel.packetList;
    if (pkts.isEmpty()) {
        statusBar()->showMessage("未加载协议，无法保存", 3000);
        return;
    }

    QString ts = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    QString note = m_saveNameEdit ? m_saveNameEdit->text().trimmed() : QString();
    if (note.isEmpty()) note = "记录";
    QString folder = ts + "_" + note;

    QString dir = QCoreApplication::applicationDirPath() + "/Data/" + folder;
    if (!QDir().mkpath(dir)) {
        QMessageBox::warning(this, "保存失败", "无法创建目录:\n" + dir);
        return;
    }
    m_saveDir = dir;

    for (int i = 0; i < pkts.size(); i++) {
        const ProtoPacket& pkt = pkts[i];
        QString title = pkt.windowTitle.isEmpty() ? QString("帧 %1").arg(i + 1) : pkt.windowTitle;
        QString fname = ts + "_" + sanitizeFilename(title) + ".txt";

        QFile* f = new QFile(dir + "/" + fname);
        if (f->open(QIODevice::WriteOnly | QIODevice::Text)) {
            QStringList names;
            for (const ProtoField& fd : pkt.fieldList) names << fd.showName;
            f->write((names.join("   ") + "\n").toUtf8());
            f->flush();
        }
        m_saveFiles.append(f);
    }

    m_saving = true;
    if (m_btnSave) m_btnSave->setText("取消");
    statusBar()->showMessage("开始保存到: " + dir, 4000);
}

void MainWindow::stopSave()
{
    for (QFile* f : m_saveFiles) {
        if (f) { if (f->isOpen()) f->close(); delete f; }
    }
    m_saveFiles.clear();
    m_saving = false;
    if (m_btnSave) m_btnSave->setText("保存");
}

void MainWindow::saveFrameLine(int pktIdx, const QStringList& values)
{
    if (!m_saving) return;
    if (pktIdx < 0 || pktIdx >= m_saveFiles.size()) return;
    QFile* f = m_saveFiles[pktIdx];
    if (!f || !f->isOpen()) return;

    f->write((values.join("   ") + "\n").toUtf8());
    f->flush();
}

// ===================== 帧解析 =====================
void MainWindow::parseRxBuffer()
{
    const QList<ProtoPacket>& pkts = g_GlobalProtoModel.packetList;
    if (pkts.isEmpty()) return;

    bool progressed = true;
    while (progressed && !m_rxBuffer.isEmpty())
    {
        progressed = false;

        // 在所有帧里找最早出现的帧头
        int chosenIdx = -1;
        int chosenPos = -1;
        for (int i = 0; i < pkts.size(); i++) {
            if (pkts[i].headerRaw.isEmpty()) continue;
            int p = m_rxBuffer.indexOf(pkts[i].headerRaw);
            if (p >= 0 && (chosenPos < 0 || p < chosenPos)) {
                chosenPos = p;
                chosenIdx = i;
            }
        }

        if (chosenIdx < 0) {
            // 没有帧头：保留可能残缺的尾部
            int maxH = 0;
            for (const ProtoPacket& pk : pkts) maxH = qMax(maxH, pk.headerRaw.size());
            if (maxH > 0 && m_rxBuffer.size() > maxH)
                m_rxBuffer = m_rxBuffer.right(maxH - 1);
            break;
        }

        if (chosenPos > 0)
            m_rxBuffer.remove(0, chosenPos); // 丢弃帧头前的垃圾

        const ProtoPacket& pkt = pkts[chosenIdx];
        QStringList values;
        QVector<double> nums;
        bool haveFrame = false;

        if (pkt.frameIsString) {
            QByteArray endMark;
            if (!pkt.endMark.isEmpty()) {
                QString e = pkt.endMark; e.remove(' ');
                endMark = QByteArray::fromHex(e.toUtf8());
            }
            int endPos = -1;
            if (!endMark.isEmpty())
                endPos = m_rxBuffer.indexOf(endMark, pkt.headerLen);
            if (endPos < 0)
                break; // 帧不完整，等更多数据

            int frameLen = endPos + endMark.size();
            QByteArray frameData = m_rxBuffer.left(frameLen);
            values = ProtocolLoader::parseFrameToValues(pkt, frameData);
            nums = ProtocolLoader::parseFrameToDoubles(pkt, frameData);
            m_rxBuffer.remove(0, frameLen);
            haveFrame = true;
        }
        else {
            // 二进制协议：计算帧长
            int frameLen = 0;
            if (pkt.isDynamicLen) {
                // 动态帧长：需要先读取帧长字段
                int needForLen = pkt.dynamicLenStart + pkt.dynamicLenSize;
                if (m_rxBuffer.size() < needForLen)
                    break; // 数据不足，等待更多

                frameLen = ProtocolLoader::computeFrameLen(pkt, m_rxBuffer);
                int minFrame = pkt.headerLen + pkt.crcResultByteCnt + 1;
                if (frameLen < minFrame || frameLen > (1024 * 1024)) {
                    m_rxBuffer.remove(0, 1); // 异常帧长，跳过1字节重同步
                    progressed = true;
                    continue;
                }
            } else {
                frameLen = pkt.effectiveLen;
                if (frameLen <= 0)
                    frameLen = pkt.headerLen + pkt.calcTotalByte + pkt.crcResultByteCnt;
            }

            if (frameLen <= 0)
                break;
            if (m_rxBuffer.size() < frameLen)
                break; // 帧不完整，等待更多数据

            QByteArray frameData = m_rxBuffer.left(frameLen);
            if (!ProtocolLoader::verifyChecksum(pkt, frameData)) {
                m_errCount++;
                m_rxBuffer.remove(0, 1); // 校验失败，跳过1字节重同步
                progressed = true;
                continue;
            }
            values = ProtocolLoader::parseFrameToValues(pkt, frameData);
            nums = ProtocolLoader::parseFrameToDoubles(pkt, frameData);
            m_rxBuffer.remove(0, frameLen);
            haveFrame = true;
        }

        if (haveFrame) {
            m_pending[chosenIdx] = FrameUpdate{values, nums};
            m_lastRxMs[chosenIdx] = m_ledElapsed.elapsed();
            m_frameHzCounts[chosenIdx]++;
            saveFrameLine(chosenIdx, values);
            m_fpsFrames++;
            emit signalFrameUpdate(chosenIdx, nums); // 每一帧都实时推给绘图
            progressed = true;
        }
    }

    if (m_rxBuffer.size() > 65536)
        m_rxBuffer = m_rxBuffer.right(4096);
}

// ===================== 显示 =====================
void MainWindow::flushUi()
{
    for (auto it = m_pending.constBegin(); it != m_pending.constEnd(); ++it) {
        int idx = it.key();
        if (idx >= 0 && idx < m_grids.size())
            updateGridDisplay(idx, it.value().values);
    }
    m_pending.clear();

    qint64 now = m_ledElapsed.elapsed();
    for (int i = 0; i < m_grids.size(); i++) {
        bool active = m_lastRxMs.contains(i) && (now - m_lastRxMs.value(i) < 1000);
        m_grids[i]->setStatus(active, m_frameHz.value(i, 0));
    }

    setLedColor(m_ledRx, m_gotData ? "#2ecc71" : "#4a5158");
    m_gotData = false;
}

void MainWindow::updateGridDisplay(int pktIdx, const QStringList& values)
{
    if (pktIdx < 0 || pktIdx >= m_grids.size()) return;
    AutoGridWidget* g = m_grids[pktIdx];
    for (int i = 0; i < values.size() && i < g->getItemCount(); i++)
        g->setValue(i, values[i]);
}

void MainWindow::updateStats()
{
    quint64 frames = m_fpsFrames;
    m_fpsFrames = 0;
    quint64 rxRate = m_rxThisSec;
    m_rxThisSec = 0;
    quint64 txRate = m_txThisSec;
    m_txThisSec = 0;

    // 每帧频率（本秒帧数）
    for (auto it = m_frameHzCounts.constBegin(); it != m_frameHzCounts.constEnd(); ++it)
        m_frameHz[it.key()] = int(it.value());
    m_frameHzCounts.clear();

    QString ratio = "--";
    if (m_txTotal > 0 && m_rxTotal > 0)
        ratio = QString::number(double(m_rxTotal) / double(m_txTotal), 'f', 1) + ":1";

    m_labStats->setText(QString("  收 %1 | 发 %2 | 收 %3 B/s | 发 %4 B/s | 收发比 %5 | 帧率 %6 fps | 错误 %7")
                            .arg(fmtBytes(m_rxTotal), fmtBytes(m_txTotal))
                            .arg(rxRate).arg(txRate)
                            .arg(ratio).arg(frames).arg(m_errCount));

    setLedColor(m_ledTx, txRate > 0 ? "#f0a13c" : "#4a5158");
}

// ===================== 工具 =====================
QIcon MainWindow::makeFrameIcon(int idx, const QString& name)
{
    QColor c = g_frameAccents[idx % 8];

    QPixmap pm(16, 16);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.setBrush(c);
    p.setPen(Qt::NoPen);
    p.drawEllipse(0, 0, 16, 16);

    QString letter = name.isEmpty() ? QString::number(idx + 1) : name.left(1);
    QFont f = p.font();
    f.setBold(true);
    f.setPixelSize(10);
    p.setFont(f);
    p.setPen(QColor("#ffffff"));
    p.drawText(pm.rect(), Qt::AlignCenter, letter);
    p.end();
    return QIcon(pm);
}

void MainWindow::setLedColor(QLabel *led, const QString& color)
{
    led->setStyleSheet(QString("background:%1; border-radius:5px;").arg(color));
}
