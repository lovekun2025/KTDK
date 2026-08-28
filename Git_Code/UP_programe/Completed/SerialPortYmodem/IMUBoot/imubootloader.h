#ifndef IMUBOOTLOADER_H
#define IMUBOOTLOADER_H

#include <QCoreApplication>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QFile>
#include <QTimer>
#include <QCommandLineParser>
#include <QTextStream>
#include <QElapsedTimer>


/* ========================= 常量 ========================= */
#define UPDATE_COMM       0x80
#define UPDATE_START_ID   0x55
#define UPDATE_START_ANS_ID 0xAA
#define UPDATE_APP_ID     0x0A
#define UPDATE_APP_ANS_ID 0x01
#define PACK_LENGTH       0xA0
#define PACK_LENGTH_ANS   0x02
#define DATA_FRAME        0x0D
#define DATA_FRAME_ANS    0x03
#define DATA_FRAME_FINAL  0x04
#define SUCCEED           0x00
#define FAIL              0x01
#define HDR               0x10
#define TAIL1             0x10
#define TAIL2             0x03
#define PKG_SIZE          1024

/* ========================= 状态机 ========================= */
enum class State { Idle, Handshake, CmdUpgrade, SendPkgLen,
                   SendData, WaitDataAck, Fail, Done };

struct Ctx {
    QSerialPort port;
    std::vector<std::vector<uint8_t>> pkgs;
    size_t currBlk = 0;
    State state = State::Idle;
    QByteArray rxBuf;
    int persent;
};

void sendCmd(uint8_t id);
QByteArray makeFrame(uint8_t id, uint8_t ext=0);
QByteArray makeLenFrame(uint32_t len);
QByteArray stuffFrame(const QByteArray &src);
QByteArray makeDataFrame(size_t idx, uint8_t crc, const std::vector<uint8_t> &data) ;
bool extractFrame(QByteArray &buf, QByteArray &frame) ;
void handleMsg(const QByteArray &f);
void onReadyRead() ;
void loadFirmware(const QString &path) ;


extern struct Ctx g;

#endif // IMUBOOTLOADER_H
