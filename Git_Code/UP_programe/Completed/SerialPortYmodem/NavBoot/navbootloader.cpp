#include "navbootloader.h"

#include <QFileInfo>
#include <array>
#include <QApplication>
#include <QDebug>

using namespace Qt;

/* ---------- Ymodem 常量 ---------- */
static const quint8 SOH = 0x01;
static const quint8 STX = 0x02;
static const quint8 EOT = 0x04;
static const quint8 ACK = 0x06;
static const quint8 NAK = 0x15;
static const quint8 C   = 0x43;

static const int DATA_SIZE = 1024;
static const int CRC_SIZE  = 2;

/* ---------- CRC16-CCITT (与 C# 完全一致) ---------- */
class Crc16Ccitt {
public:
    enum Initial { Zeros = 0, NonZero1 = 0xffff, NonZero2 = 0x1D0F };
private:
    quint16 table[256];
    quint16 initial;
public:
    quint16 compute(const QByteArray &ba) {
        quint16 crc = initial;
        for (char c : ba) {
            crc = (crc << 8) ^ table[((crc >> 8) ^ (static_cast<quint8>(c) & 0xff))];
        }
        return crc;
    }
    // 恢复你最初的大端序（和你原来一致，不修改）
    QByteArray computeBytes(const QByteArray &ba) {
        quint16 crc = compute(ba);
        return QByteArray(reinterpret_cast<const char*>(std::array<quint8,2>{

            static_cast<quint8>(crc >> 8),  // 高位在前（你最初的逻辑）
            static_cast<quint8>(crc & 0x00ff) // 低位在后

        }.data()), 2);
        qDebug() << "CRC大端：" << QByteArray(reinterpret_cast<const char*>(&crc), 2).toHex();
    }
    explicit Crc16Ccitt(Initial init = Zeros) : initial(static_cast<quint16>(init)) {
        const quint16 poly = 0x1021;
        for (int i = 0; i < 256; ++i) {
            quint16 temp = 0;
            quint16 a = static_cast<quint16>(i << 8);
            for (int j = 0; j < 8; ++j) {
                if (((temp ^ a) & 0x8000) != 0)
                    temp = (temp << 1) ^ poly;
                else
                    temp <<= 1;
                a <<= 1;
            }
            table[i] = temp;
        }
    }
};

bool YmodemUploader::ymodemUploadFile(bool &runflag, QProgressBar* qprogress) {
    const int ACK_RETRY_MAX = 3;
    const int WAIT_TIMEOUT = 1000; // 恢复你最初的超时（2秒）
    const int SEND_DELAY = 30;     // 恢复你最初的间隔（100ms）
    const int DATA_SIZE = 1024;

    if (!port.isOpen()) { qDebug() << "[错误] 串口未打开"; return false; }
    port.clear();
    rxBuf.clear();

    QFile file(filePath);
    if (!file.open(QFile::ReadOnly)) { qDebug() << "[错误] 文件打开失败"; return false; }
    qint64 totalSize = file.size();

    // 1. 启动握手
    const QByteArray uploadCmd = QByteArrayLiteral("\xEA\x51\x00\x00\x08\x00\x00\x00\x00\x00\x08");
    qDebug() << "[发送] 启动指令：" << uploadCmd.toHex(' ');
    sendData(uploadCmd);
    QThread::msleep(2000);
    int initRetry = 0;
    bool gotC = false;
    while (initRetry <= 2 && runflag && !gotC) {
        if (waitForByte('C', WAIT_TIMEOUT)) {
            gotC = true;
            qDebug() << "[接收] 收到'C'，启动传输";
        } else {
            initRetry++;
            sendData(uploadCmd);
            qDebug() << "[重试] 启动指令（第" << initRetry << "次）";
            QThread::msleep(20);
        }
    }
    if (!gotC || !runflag) { file.close(); return false; }

    // 2. 初始包
    QString fileName = QFileInfo(file).fileName();
    qint64 fileSize = file.size();
    bool initialAcked = false;
    Crc16Ccitt crcInit(Crc16Ccitt::Zeros);
    for (int r = 0; r < ACK_RETRY_MAX && !initialAcked && runflag; r++) {
        sendInitialPacket(fileName, fileSize);  // 用你原来的sendInitialPacket（不修改）
        qDebug() << "[发送] 初始包（第" << r+1 << "次）：文件名=" << fileName << "，文件大小=" << fileSize;
        QThread::msleep(SEND_DELAY);

        if (waitForByte(ACK, WAIT_TIMEOUT)) {
            initialAcked = true;
            qDebug() << "[接收] 初始包收到ACK";
        } else {
            qDebug() << "[重试] 初始包（第" << r+1 << "次）";
        }
    }
    if (!initialAcked || !runflag) { file.close(); return false; }

    // 3. 等待初始包后的'C'
    bool gotCAfterInitial = false;
    for (int r = 0; r < ACK_RETRY_MAX && !gotCAfterInitial && runflag; r++) {
        if (waitForByte('C', WAIT_TIMEOUT)) {
            gotCAfterInitial = true;
            qDebug() << "[接收] 初始包后收到'C'，开始传输数据帧";
        } else {
            qDebug() << "[重试] 初始包后等待'C'（第" << r+1 << "次）";
        }
    }
    if (!gotCAfterInitial || !runflag) {qDebug() << "gaunbi";file.close(); return false; }

    // 4. 数据帧传输（）
    bool dataSuccess = true;
    int pktNo = 0;
    Crc16Ccitt crcData(Crc16Ccitt::Zeros);

    while (!file.atEnd() && runflag && dataSuccess) {
        QByteArray chunk = file.read(DATA_SIZE);
        if (chunk.size() < DATA_SIZE) {
            chunk.resize(DATA_SIZE); // 正确方式：扩容到 1024 字节，新增部分填 0
        }

//
        pktNo = (pktNo + 1) & 0xFF;
        int invPktNo = 255 - pktNo;
        QByteArray crc = crcData.computeBytes(chunk);
        bool pktAcked = false;
        for (int r = 0; r < ACK_RETRY_MAX && !pktAcked && runflag; r++) {
            sendYmodemPacket(STX, pktNo, chunk, crc);  // 用你原来的发送函数
            qDebug() << "[发送] 数据包" << pktNo << "（第" << r+1 << "次）";
            QThread::msleep(SEND_DELAY);

            if (waitForByte(ACK, WAIT_TIMEOUT)) {
                pktAcked = true;
                qDebug() << "[接收] 数据包" << pktNo << "收到ACK";
            } else {
                qDebug() << "[重试] 数据包" << pktNo << "（第" << r+1 << "次）";
            }
        }

        if (!pktAcked) {
            dataSuccess = false;
            qDebug() << "[失败] 数据包" << pktNo << "重发失败";
            // 进度条变红提示失败
            QMetaObject::invokeMethod(qprogress, "setStyleSheet", Qt::QueuedConnection,Q_ARG(QString, "QProgressBar::chunk { background-color: red; }"));
            break;
        }

        // 进度条
        qreal progress = (qreal)file.pos() / totalSize * 100.0;
        QMetaObject::invokeMethod(qprogress, "setValue", Qt::QueuedConnection, Q_ARG(int, qRound(progress)));
    }

    //空包发送
    if (dataSuccess && runflag) {
        QByteArray emptyChunk(DATA_SIZE, 0);
        pktNo = (pktNo + 1) & 0xFF; // 包号递增
        QByteArray crc = crcData.computeBytes(emptyChunk);
        bool pktAcked = false;
        for (int r = 0; r < ACK_RETRY_MAX && !pktAcked && runflag; r++) {
            sendYmodemPacket(STX, pktNo, emptyChunk, crc);
            QThread::msleep(SEND_DELAY);
            if (waitForByte(ACK, WAIT_TIMEOUT)) {
                pktAcked = true;
            }
        }
        if (!pktAcked) {
            dataSuccess = false;
        }
    }

    // 5. EOT和结束包
    if (!dataSuccess || !runflag) { qDebug() << " 数据jieshu"; file.close();   return false; }

    bool eotSuccess = false;
    for (int r = 0; r < ACK_RETRY_MAX && !eotSuccess && runflag; r++) {
        sendByte(EOT);
        qDebug() << "[发送] 第一次EOT";
        QThread::msleep(SEND_DELAY);

        if (waitForByte(NAK, WAIT_TIMEOUT)) {
            sendByte(EOT);
            qDebug() << "[发送] 第二次EOT";
            QThread::msleep(SEND_DELAY);
            if (waitForByte(ACK, WAIT_TIMEOUT)) {
                eotSuccess = true;
            }
        }
    }
    if (!eotSuccess) { file.close(); return false; }

    bool gotCAfterEOT = false;
    for (int r = 0; r < ACK_RETRY_MAX && !gotCAfterEOT && runflag; r++) {
        if (waitForByte(C, WAIT_TIMEOUT)) {
            gotCAfterEOT = true;
        }
    }
    if (!gotCAfterEOT) { file.close(); return false; }

    bool finalAcked = false;
    for (int r = 0; r < ACK_RETRY_MAX && !finalAcked && runflag; r++) {
        sendClosingPacket();  // 用你原来的结束包发送函数
        QThread::msleep(SEND_DELAY);
        if (waitForByte(ACK, WAIT_TIMEOUT)) {
            finalAcked = true;
        }
    }

    file.close();
    return finalAcked;
}


void YmodemUploader::sendYmodemPacket(quint8 header, int pktNo, const QByteArray &data, const QByteArray &crc) {
    char hdr = char(header);
    char np = char(pktNo);
    char inp = char(255 - pktNo);
    port.write(&hdr, 1);
    port.write(&np, 1);
    port.write(&inp, 1);
    port.write(data);
    port.write(crc);
    port.waitForBytesWritten(200);
}

void YmodemUploader::sendInitialPacket(const QString &fileName, qint64 fileSize) {
    QByteArray data(DATA_SIZE, 0);
    QByteArray fn = fileName.toLocal8Bit();
    QByteArray fs = QByteArray::number(fileSize);
    memcpy(data.data(), fn.constData(), fn.size());
    memcpy(data.data() + fn.size() + 1, fs.constData(), fs.size());

    Crc16Ccitt crc16(Crc16Ccitt::Zeros);
    QByteArray crc = crc16.computeBytes(data);
    sendYmodemPacket(STX, 0, data, crc);
}

void YmodemUploader::sendClosingPacket() {
    QByteArray data(DATA_SIZE, 0);
    Crc16Ccitt crc16(Crc16Ccitt::Zeros);
    QByteArray crc = crc16.computeBytes(data);
    sendYmodemPacket(STX, 0, data, crc);
}

bool YmodemUploader::waitForByte(char expected, int timeout) {
    QElapsedTimer t; t.start();
    while (!rxBuf.contains(expected)) {
        if (t.elapsed() > timeout) return false;
        port.waitForReadyRead(50);
    }
    rxBuf.remove(rxBuf.indexOf(expected), 1);
    return true;
}
