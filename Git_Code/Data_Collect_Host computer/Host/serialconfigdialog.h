#ifndef SERIALCONFIGDIALOG_H
#define SERIALCONFIGDIALOG_H

#include <QDialog>
#include <QSerialPort>
#include <QSerialPortInfo>

namespace Ui {
class SerialConfigDialog;
}

// 串口配置参数结构体，传给 MainWindow 用于打开串口
struct SerialConfig
{
    QString portName;
    qint32 baudRate = 115200;
    QSerialPort::DataBits dataBits = QSerialPort::Data8;
    QSerialPort::Parity parity   = QSerialPort::NoParity;
    QSerialPort::StopBits stopBits = QSerialPort::OneStop;
};

class SerialConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SerialConfigDialog(QWidget *parent = nullptr);
    ~SerialConfigDialog();

signals:
    // 用户点"确定"后发射，携带完整串口配置
    void signalSerialConfig(const SerialConfig& config);

private slots:
    void onAccept();      // 确定按钮
    void onReject();      // 取消按钮

private:
    void refreshSerialPorts();  // 扫描可用串口填入下拉框
    Ui::SerialConfigDialog *ui;
};

#endif // SERIALCONFIGDIALOG_H
