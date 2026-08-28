#ifndef NAVBOOTLOADER_H
#define NAVBOOTLOADER_H

#include <QCoreApplication>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QFile>
#include <QTimer>
#include <QCommandLineParser>
#include <QTextStream>
#include <QElapsedTimer>
#include <vector>
#include <numeric>
#include <cstring>
#include <QThread>
#include <QProgressBar>
#include <QDebug>

/* ---------- Ymodem 上传类 ---------- */
class YmodemUploader : public QObject {
    Q_OBJECT
public:
    explicit YmodemUploader(QObject *parent = nullptr) : QObject(parent) {
        connect(&port, &QSerialPort::readyRead, this, &YmodemUploader::onReadyRead);
    }

    /* 对外接口：与 C# 保持一致 */
    void setPortName(const QString &name) { port.setPortName(name); }
    void setFilePath(const QString &path) { filePath = path; }
    void setBaudRate(uint BandRate) { port.setBaudRate(BandRate);}
    bool openPort() {
        if (port.isOpen()) port.close();
        //port.setBaudRate(230400);
        port.setDataBits(QSerialPort::Data8);
        port.setParity(QSerialPort::NoParity);
        port.setStopBits(QSerialPort::OneStop);
        return port.open(QIODevice::ReadWrite);
    }
    void closePort() { if (port.isOpen()) port.close(); }
    bool ymodemUploadFile(bool &runflag,QProgressBar*qproress);   // 主入口

private:
    QSerialPort port;
    QString filePath;
    QByteArray rxBuf;

    bool waitForByte(char expected, int timeout = 5000);
    void sendByte(char c) { port.write(&c, 1); port.waitForBytesWritten(100); }
    void sendData(const QByteArray &ba) { port.write(ba); port.waitForBytesWritten(2); }
    void onReadyRead() {
        rxBuf.append(port.readAll());
        qDebug()<<"R:"<<rxBuf;
    }

    /* 内部 Ymodem 发送 */
    void sendYmodemPacket(quint8 header, int pktNo, const QByteArray &data, const QByteArray &crc);
    void sendInitialPacket(const QString &fileName, qint64 fileSize);
    void sendClosingPacket();
};

#endif // NAVBOOTLOADER_H
