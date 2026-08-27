#include "serialconfigdialog.h"
#include "ui_serialconfigdialog.h"
#include <QDebug>

SerialConfigDialog::SerialConfigDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SerialConfigDialog)
{
    ui->setupUi(this);

    // 扫描可用串口填入下拉框
    refreshSerialPorts();

    // 补齐 .ui 里空着的数据位/校验位/停止位下拉框
    ui->comboBox_3->clear();
    ui->comboBox_3->addItems({"5", "6", "7", "8"});
    ui->comboBox_3->setCurrentText("8");

    ui->comboBox_4->clear();
    ui->comboBox_4->addItems({"NONE", "ODD", "EVEN", "SPACE"});
    ui->comboBox_4->setCurrentText("NONE");

    ui->comboBox_6->clear();
    ui->comboBox_6->addItems({"1", "1.5", "2"});
    ui->comboBox_6->setCurrentText("1");

    // 绑定确定/取消按钮
    connect(ui->pushButton_2, &QPushButton::clicked, this, &SerialConfigDialog::onAccept);
    connect(ui->pushButton,   &QPushButton::clicked, this, &SerialConfigDialog::onReject);
}

SerialConfigDialog::~SerialConfigDialog()
{
    delete ui;
}

void SerialConfigDialog::refreshSerialPorts()
{
    ui->comboBox->clear();
    const auto ports = QSerialPortInfo::availablePorts();
    for (const auto& info : ports)
    {
        ui->comboBox->addItem(info.portName());
    }
    if (ui->comboBox->count() > 0)
        ui->comboBox->setCurrentIndex(0);
}

void SerialConfigDialog::onAccept()
{
    SerialConfig config;
    config.portName = ui->comboBox->currentText();

    // 波特率
    config.baudRate = ui->comboBox_2->currentText().toInt();

    // 数据位
    int db = ui->comboBox_3->currentText().toInt();
    switch (db) {
    case 5:  config.dataBits = QSerialPort::Data5; break;
    case 6:  config.dataBits = QSerialPort::Data6; break;
    case 7:  config.dataBits = QSerialPort::Data7; break;
    default: config.dataBits = QSerialPort::Data8; break;
    }

    // 校验位
    QString par = ui->comboBox_4->currentText();
    if (par == "ODD")        config.parity = QSerialPort::OddParity;
    else if (par == "EVEN")  config.parity = QSerialPort::EvenParity;
    else if (par == "SPACE") config.parity = QSerialPort::SpaceParity;
    else                     config.parity = QSerialPort::NoParity;

    // 停止位
    QString sb = ui->comboBox_6->currentText();
    if (sb == "1.5")      config.stopBits = QSerialPort::OneAndHalfStop;
    else if (sb == "2")   config.stopBits = QSerialPort::TwoStop;
    else                  config.stopBits = QSerialPort::OneStop;

    qDebug() << "[SerialConfig] 串口配置确认："
             << config.portName << config.baudRate
             << db << par << sb;

    emit signalSerialConfig(config);
    accept();
}

void SerialConfigDialog::onReject()
{
    reject();
}
