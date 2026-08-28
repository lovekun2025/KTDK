#include "widget.h"
#include "ui_widget.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QSerialPortInfo>
#include <QDebug>
#include <QtConcurrent>

#include "./IMUBoot/imubootloader.h"
#include "./NavBoot/navbootloader.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget),
    serialPort(new QSerialPort),
    ymodemFileTransmit(new YmodemFileTransmit),
    ymodemFileReceive(new YmodemFileReceive)
{

    transmitButtonStatus = false;
    receiveButtonStatus  = false;

    ui->setupUi(this);
    setWindowTitle(tr("BootLoader"));

    QSerialPortInfo serialPortInfo;

    foreach(serialPortInfo, QSerialPortInfo::availablePorts())
    {
        ui->comPort->addItem(serialPortInfo.portName());
    }

    serialPort->setPortName("COM1");
    serialPort->setBaudRate(115200);
    serialPort->setDataBits(QSerialPort::Data8);
    serialPort->setStopBits(QSerialPort::OneStop);
    serialPort->setParity(QSerialPort::NoParity);
    serialPort->setFlowControl(QSerialPort::NoFlowControl);

    connect(ymodemFileTransmit, SIGNAL(transmitProgress(int)), this, SLOT(transmitProgress(int)));
    connect(ymodemFileReceive, SIGNAL(receiveProgress(int)), this, SLOT(receiveProgress(int)));
    connect(ymodemFileTransmit, SIGNAL(transmitStatus(YmodemFileTransmit::Status)), this, SLOT(transmitStatus(YmodemFileTransmit::Status)));
    connect(ymodemFileReceive, SIGNAL(receiveStatus(YmodemFileReceive::Status)), this, SLOT(receiveStatus(YmodemFileReceive::Status)));

    setStyle();
}

void Widget::setStyle()
{
    const QString qssLightNoHeaderHighlight= R"(
    /* ---------- 全局 ---------- */
    QWidget {
        background:#ffffff;
        color:#000000;
        font-family:"Segoe UI","Microsoft YaHei";
        font-size:9pt;
    }

    /* ---------- 表格 ---------- */
    QTableWidget {
        border:1px solid #b3d9ff;
        gridline-color:#d0d0d0;
        background:#f5f5f5;
        alternate-background-color:#ebebeb;
    }
    QTableWidget::item:selected {
        background:#cce5ff;
        color:#000000;
    }

    /* ---------- 深色表头 ---------- */
    QHeaderView::section {
        background:#484875;
        color:#ffffff;
        padding:2px;
        border:none;
    }

    /* ---------- TabBar ---------- */
    QTabBar::tab {
        background:#ffffff;
        border:1px solid #b3d9ff;
        border-bottom:3px solid #b3d9ff;
        border-top-left-radius:6px;
        border-top-right-radius:6px;
        padding:6px 14px;
        margin-right:2px;
    }
    QTabBar::tab:selected {
        background:#ffffff;
        color:#0078d4;
        border-bottom:3px solid #b3d9ff;
        font-weight:bold;
    }
    QTabBar::tab:hover:!selected {
        background:#e5f1ff;
        border-bottom-color:#b3d9ff;
    }

    /* ---------- 按钮 ---------- */
    QPushButton {
        border:none;
        border-radius:4px;
        padding:6px 18px;
        background:#484875;
        color:#ffffff;
    }
    QPushButton:hover {
        background:#3b4252;
    }
    QPushButton:pressed {
        background:#0078d4;
    }
    QPushButton:disabled {            /* <-- 新增 */
        background: #d0d0d0;
        color: #808080;
        border: 1px solid #b0b0b0;
    }
    /* ---------- QListWidget ---------- */
    QListWidget{background:white;border:none;outline:0;}
    QListWidget::item{padding:6px;border-radius:4px;}
    QListWidget::item:selected{background:#cce8ff;}


    /* ---------- QGroupBox ---------- */

    )";

    setStyleSheet(qssLightNoHeaderHighlight);

}

Widget::~Widget()
{
    delete ui;
    delete serialPort;
    delete ymodemFileTransmit;
    delete ymodemFileReceive;
}

void Widget::on_comButton_clicked()
{
    static bool button_status = false;

    if(button_status == false)
    {
        serialPort->setPortName(ui->comPort->currentText());
        serialPort->setBaudRate(ui->comBaudRate->currentText().toInt());

        if(serialPort->open(QSerialPort::ReadWrite) == true)
        {
            button_status = true;

            ui->comPort->setDisabled(true);
            ui->comBaudRate->setDisabled(true);
            ui->comButton->setText(u8"关闭串口");

            ui->transmitBrowse->setEnabled(true);
            ui->receiveBrowse->setEnabled(true);

            ui->transmitBrowse_imu->setEnabled(true);
            ui->transmitBrowse_nav->setEnabled(true);

            if(ui->transmitPath->text().isEmpty() != true)
            {
                ui->transmitButton->setEnabled(true);
            }

            if(ui->transmitPath_imu->text().isEmpty() != true)
            {
                ui->transmitButton_imu->setEnabled(true);
            }
            if(ui->transmitPath_nav->text().isEmpty() != true)
            {
                ui->transmitButton_nav->setEnabled(true);
            }

            if(ui->receivePath->text().isEmpty() != true)
            {
                ui->receiveButton->setEnabled(true);
            }
        }
        else
        {
            QMessageBox::warning(this, u8"串口打开失败", u8"请检查串口是否已被占用！", u8"关闭");
        }
    }
    else
    {
        button_status = false;

        serialPort->close();

        ui->comPort->setEnabled(true);
        ui->comBaudRate->setEnabled(true);
        ui->comButton->setText(u8"打开串口");

        ui->transmitBrowse->setDisabled(true);
        ui->transmitButton->setDisabled(true);

        ui->transmitBrowse_imu->setDisabled(true);
        ui->transmitButton_imu->setDisabled(true);

        ui->transmitBrowse_nav->setDisabled(true);
        ui->transmitButton_nav->setDisabled(true);

        ui->receiveBrowse->setDisabled(true);
        ui->receiveButton->setDisabled(true);

    }
}

void Widget::on_transmitBrowse_clicked()
{
    ui->transmitPath->setText(QFileDialog::getOpenFileName(this, u8"打开文件", ".", u8"任意文件 (*.*)"));

    if(ui->transmitPath->text().isEmpty() != true)
    {
        ui->transmitButton->setEnabled(true);
    }
    else
    {
        ui->transmitButton->setDisabled(true);
    }
}

void Widget::on_receiveBrowse_clicked()
{
    ui->receivePath->setText(QFileDialog::getExistingDirectory(this, u8"选择目录", ".", QFileDialog::ShowDirsOnly));

    if(ui->receivePath->text().isEmpty() != true)
    {
        ui->receiveButton->setEnabled(true);
    }
    else
    {
        ui->receiveButton->setDisabled(true);
    }
}

void Widget::on_transmitButton_clicked()
{
    if(transmitButtonStatus == false)
    {
        serialPort->close();

        ymodemFileTransmit->setFileName(ui->transmitPath->text());
        ymodemFileTransmit->setPortName(ui->comPort->currentText());
        ymodemFileTransmit->setPortBaudRate(ui->comBaudRate->currentText().toInt());

        if(ymodemFileTransmit->startTransmit() == true)
        {
            transmitButtonStatus = true;

            ui->comButton->setDisabled(true);

            ui->receiveBrowse->setDisabled(true);
            ui->receiveButton->setDisabled(true);

            ui->transmitBrowse->setDisabled(true);
            ui->transmitButton->setText(u8"取消");
            ui->transmitProgress->setValue(0);
        }
        else
        {
            QMessageBox::warning(this, u8"失败", u8"文件发送失败！", u8"关闭");
        }
    }
    else
    {
        ymodemFileTransmit->stopTransmit();
    }
}

void Widget::on_receiveButton_clicked()
{
    if(receiveButtonStatus == false)
    {
        serialPort->close();

        ymodemFileReceive->setFilePath(ui->receivePath->text());
        ymodemFileReceive->setPortName(ui->comPort->currentText());
        ymodemFileReceive->setPortBaudRate(ui->comBaudRate->currentText().toInt());

        if(ymodemFileReceive->startReceive() == true)
        {
            receiveButtonStatus = true;

            ui->comButton->setDisabled(true);

            ui->transmitBrowse->setDisabled(true);
            ui->transmitButton->setDisabled(true);

            ui->receiveBrowse->setDisabled(true);
            ui->receiveButton->setText(u8"取消");
            ui->receiveProgress->setValue(0);
        }
        else
        {
            QMessageBox::warning(this, u8"失败", u8"文件接收失败！", u8"关闭");
        }
    }
    else
    {
        ymodemFileReceive->stopReceive();
    }
}

void Widget::transmitProgress(int progress)
{
    ui->transmitProgress->setValue(progress);
}

void Widget::receiveProgress(int progress)
{
    ui->receiveProgress->setValue(progress);
}

void Widget::transmitStatus(Ymodem::Status status)
{
    switch(status)
    {
        case YmodemFileTransmit::StatusEstablish:
        {
            break;
        }

        case YmodemFileTransmit::StatusTransmit:
        {
            break;
        }

        case YmodemFileTransmit::StatusFinish:
        {
            transmitButtonStatus = false;

            ui->comButton->setEnabled(true);

            ui->receiveBrowse->setEnabled(true);

            if(ui->receivePath->text().isEmpty() != true)
            {
                ui->receiveButton->setEnabled(true);
            }

            ui->transmitBrowse->setEnabled(true);
            ui->transmitButton->setText(u8"发送");

            QMessageBox::warning(this, u8"成功", u8"文件发送成功！", u8"关闭");

            break;
        }

        case YmodemFileTransmit::StatusAbort:
        {
            transmitButtonStatus = false;

            ui->comButton->setEnabled(true);

            ui->receiveBrowse->setEnabled(true);

            if(ui->receivePath->text().isEmpty() != true)
            {
                ui->receiveButton->setEnabled(true);
            }

            ui->transmitBrowse->setEnabled(true);
            ui->transmitButton->setText(u8"发送");

            QMessageBox::warning(this, u8"失败", u8"文件发送失败！", u8"关闭");

            break;
        }

        case YmodemFileTransmit::StatusTimeout:
        {
            transmitButtonStatus = false;

            ui->comButton->setEnabled(true);

            ui->receiveBrowse->setEnabled(true);

            if(ui->receivePath->text().isEmpty() != true)
            {
                ui->receiveButton->setEnabled(true);
            }

            ui->transmitBrowse->setEnabled(true);
            ui->transmitButton->setText(u8"发送");

            QMessageBox::warning(this, u8"失败", u8"文件发送失败！", u8"关闭");

            break;
        }

        default:
        {
            transmitButtonStatus = false;

            ui->comButton->setEnabled(true);

            ui->receiveBrowse->setEnabled(true);

            if(ui->receivePath->text().isEmpty() != true)
            {
                ui->receiveButton->setEnabled(true);
            }

            ui->transmitBrowse->setEnabled(true);
            ui->transmitButton->setText(u8"发送");

            QMessageBox::warning(this, u8"失败", u8"文件发送失败！", u8"关闭");
        }
    }
}

void Widget::receiveStatus(YmodemFileReceive::Status status)
{
    switch(status)
    {
        case YmodemFileReceive::StatusEstablish:
        {
            break;
        }

        case YmodemFileReceive::StatusTransmit:
        {
            break;
        }

        case YmodemFileReceive::StatusFinish:
        {
            receiveButtonStatus = false;

            ui->comButton->setEnabled(true);

            ui->transmitBrowse->setEnabled(true);

            if(ui->transmitPath->text().isEmpty() != true)
            {
                ui->transmitButton->setEnabled(true);
            }

            ui->receiveBrowse->setEnabled(true);
            ui->receiveButton->setText(u8"接收");

            QMessageBox::warning(this, u8"成功", u8"文件接收成功！", u8"关闭");

            break;
        }

        case YmodemFileReceive::StatusAbort:
        {
            receiveButtonStatus = false;

            ui->comButton->setEnabled(true);

            ui->transmitBrowse->setEnabled(true);

            if(ui->transmitPath->text().isEmpty() != true)
            {
                ui->transmitButton->setEnabled(true);
            }

            ui->receiveBrowse->setEnabled(true);
            ui->receiveButton->setText(u8"接收");

            QMessageBox::warning(this, u8"失败", u8"文件接收失败！", u8"关闭");

            break;
        }

        case YmodemFileReceive::StatusTimeout:
        {
            receiveButtonStatus = false;

            ui->comButton->setEnabled(true);

            ui->transmitBrowse->setEnabled(true);

            if(ui->transmitPath->text().isEmpty() != true)
            {
                ui->transmitButton->setEnabled(true);
            }

            ui->receiveBrowse->setEnabled(true);
            ui->receiveButton->setText(u8"接收");

            QMessageBox::warning(this, u8"失败", u8"文件接收失败！", u8"关闭");

            break;
        }

        default:
        {
            receiveButtonStatus = false;

            ui->comButton->setEnabled(true);

            ui->transmitBrowse->setEnabled(true);

            if(ui->transmitPath->text().isEmpty() != true)
            {
                ui->transmitButton->setEnabled(true);
            }

            ui->receiveBrowse->setEnabled(true);
            ui->receiveButton->setText(u8"接收");

            QMessageBox::warning(this, u8"失败", u8"文件接收失败！", u8"关闭");
        }
    }
}

void Widget::on_transmitBrowse_imu_clicked()
{
    QString path;
    QSettings Settings(QCoreApplication::applicationDirPath() + "/UpgradeSettings.ini", QSettings::IniFormat);
    Settings.beginGroup("IMUSetting") ;
    path = Settings.value("Path").toString();//,ui->transmitPath_imu->text());
    Settings.endGroup();
    if(path == "")
        path = ".";
    ui->transmitPath_imu->setText(QFileDialog::getOpenFileName(this, u8"打开文件", path, "二进制文件 (*.bin)"));

    if(ui->transmitPath_imu->text().isEmpty() != true)
    {
        ui->transmitButton_imu->setEnabled(true);
        QSettings Settings(QCoreApplication::applicationDirPath() + "/UpgradeSettings.ini", QSettings::IniFormat);
        Settings.beginGroup("IMUSetting") ;
        Settings.setValue("Path",ui->transmitPath_imu->text());
        Settings.endGroup();
    }
    else
    {
        ui->transmitButton_imu->setDisabled(true);
    }
}


void Widget::on_transmitButton_imu_clicked()
{
    if(transmitButtonStatus == false)
    {
        serialPort->close();

        QString filePath = ui->transmitPath_imu->text();
        QString portName = ui->comPort->currentText();
        uint baudrate = 460800;//ui->comBaudRate->currentText().toInt();

        transmitButtonStatus = true;

        ui->comButton->setDisabled(true);

        ui->receiveBrowse->setDisabled(true);
        ui->receiveButton->setDisabled(true);

        ui->transmitBrowse_imu->setDisabled(true);
        ui->transmitButton_imu->setText("取消");

        ui->transmitProgress_imu->setValue(0);

        try {
            loadFirmware(filePath);

            g.port.setPortName(portName);
            g.port.setBaudRate(baudrate);
            g.port.setDataBits(QSerialPort::Data8);
            g.port.setParity(QSerialPort::NoParity);
            g.port.setStopBits(QSerialPort::OneStop);
            if (!g.port.open(QIODevice::ReadWrite))
                throw QString("无法打开串口: " + portName);
            qDebug()<<("已成功打开串口 " + portName);
            qDebug()<<("开始固件下载...");
            QObject::connect(&g.port, &QSerialPort::readyRead, &onReadyRead);

            g.state = State::Handshake;

            QTimer *handshakeTimer = new QTimer(this);
            connect(handshakeTimer, &QTimer::timeout, this, [&](){
                if (g.state == State::Handshake) sendCmd(UPDATE_START_ID);
                ui->transmitProgress_imu->setValue(g.persent);

            });
            handshakeTimer->start(50);

            /* 事件循环直到完成 */
            while (g.state!=State::Done && g.state!=State::Fail)
                QApplication::processEvents(QEventLoop::AllEvents, 1);

            g.port.close();
            if (g.state==State::Fail) qDebug()<<("下载失败!");
            else                       qDebug()<<("下载成功!");
        } catch (const QString &e) {
            qDebug() << "错误: ";
            QMessageBox::warning(this, u8"失败", u8"文件发送失败！", u8"关闭");
        }
        qDebug()<<("程序执行完毕");
        ui->transmitButton_imu->setText("发送");
        transmitButtonStatus = false;
        ui->comButton->setDisabled(false);
        ui->receiveBrowse->setDisabled(false);
        ui->receiveButton->setDisabled(false);
        ui->transmitBrowse_imu->setDisabled(false);
    }
    else
    {
        g.state =State::Fail;
        QMessageBox::warning(this, "失败", "文件取消发送！", "关闭");
    }
}


void Widget::on_transmitBrowse_nav_clicked()
{
    ui->transmitPath_nav->setText(QFileDialog::getOpenFileName(this, u8"打开文件", ".", "sou文件(*.sou)|*.sou|所有文件(*.*)|*.*"));

    if(ui->transmitPath_nav->text().isEmpty() != true)
    {
        ui->transmitButton_nav->setEnabled(true);
    }
    else
    {
        ui->transmitButton_nav->setDisabled(true);
    }
}


void Widget::on_transmitButton_nav_clicked()
{
    static bool runflag = false;
    if(transmitButtonStatus == false)
    {
        serialPort->close();

        QString filePath = ui->transmitPath_nav->text();
        QString portName = ui->comPort->currentText();
        uint baudrate =ui->comBaudRate->currentText().toInt();

        transmitButtonStatus = true;

        ui->comButton->setDisabled(true);

        ui->receiveBrowse->setDisabled(true);
        ui->receiveButton->setDisabled(true);

        ui->transmitBrowse_nav->setDisabled(true);
        ui->transmitButton_nav->setText("取消");

        ui->transmitProgress_nav->setValue(0);

        runflag = true;
        QFuture<void> ftt = QtConcurrent::run([&]()
        {
            YmodemUploader uploader;
            uploader.setPortName(portName);
            uploader.setFilePath(filePath);
            uploader.setBaudRate(baudrate);
            if (!uploader.openPort()) {
                qDebug() << "Failed to open port " << portName << "\n";
            }
            else
            {
                qDebug() << "Uploading " << filePath << " via " << portName << " ...\n";
                bool ok = uploader.ymodemUploadFile(runflag,ui->transmitProgress_nav);
                uploader.closePort();
                qDebug() << (ok ? "Upload succeeded.\n" : "Upload failed.\n");
            }
        });
        /* 局部事件循环：界面继续刷新，函数不返回 */
        QEventLoop loop;
        QFutureWatcher<void> watcher;
        connect(&watcher, &QFutureWatcher<void>::finished,
                &loop, &QEventLoop::quit);
        watcher.setFuture(ftt);
        /* 伪死等：循环退出即任务完成 */
        loop.exec();          // ← 这里会阻塞，但事件循环仍在跑

        qDebug()<<("程序执行完毕");
        ui->transmitButton_nav->setText("发送");
        transmitButtonStatus = false;
        ui->comButton->setDisabled(false);
        ui->receiveBrowse->setDisabled(false);
        ui->receiveButton->setDisabled(false);
        ui->transmitBrowse_nav->setDisabled(false);
    }
    else
    {
        runflag = false;
        QMessageBox::warning(this, "失败", "文件取消发送！", "关闭");
    }
}

