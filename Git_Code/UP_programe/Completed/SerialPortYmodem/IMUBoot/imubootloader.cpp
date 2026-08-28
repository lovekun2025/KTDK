#include "imubootloader.h"

#include <QCoreApplication>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QFile>
#include <QTimer>
#include <QCommandLineParser>
#include <QTextStream>
#include <QElapsedTimer>
#include <QDebug>

#include <vector>
#include <numeric>

using namespace Qt;





/* ========================= 全局变量 ========================= */
struct Ctx g;

/* ========================= 日志 ========================= */

/* ========================= 构造帧 ========================= */
QByteArray makeFrame(uint8_t id, uint8_t ext) {
    return QByteArray(reinterpret_cast<const char*>(
        std::vector<uint8_t>{HDR, UPDATE_COMM, id, ext, TAIL1, TAIL2}.data()), 6);
}
QByteArray makeLenFrame(uint32_t len) {
    std::vector<uint8_t> v = {HDR, UPDATE_COMM, PACK_LENGTH,
                              uint8_t(len), uint8_t(len>>8),
                              uint8_t(len>>16), uint8_t(len>>24), 0, TAIL1, TAIL2};
    v[7] = std::accumulate(v.begin()+3, v.begin()+7, uint8_t(0));
    return QByteArray(reinterpret_cast<const char*>(v.data()), int(v.size()));
}
QByteArray stuffFrame(const QByteArray &src) {
    QByteArray dst; dst.reserve(src.size()*2);
    dst.append(src[0]);
    for (int i=1;i<src.size()-2;++i) {
        dst.append(src[i]);
        if (static_cast<uint8_t>(src[i])==HDR) dst.append(HDR);
    }
    dst.append(src.mid(src.size()-2));
    return dst;
}
QByteArray makeDataFrame(size_t idx, uint8_t crc, const std::vector<uint8_t> &data)
{
    QByteArray f(1030, 0);
    f[0] = HDR; f[1] = UPDATE_COMM; f[2] = DATA_FRAME;
    memcpy(f.data()+3, data.data(), PKG_SIZE);
    f[1027] = char(crc);
    f[1028] = TAIL1; f[1029] = TAIL2;
    return stuffFrame(f);
}

/* ========================= 发送 ========================= */
void sendCmd(uint8_t id) { g.port.write(makeFrame(id)); }
void sendCmdLen() { g.port.write(makeLenFrame(uint32_t(g.pkgs.size()*PKG_SIZE))); }
void sendDataBlk() {
    if (g.currBlk >= g.pkgs.size()) { g.state = State::Done; qDebug()<<("下载已完成!"); return; }
    const auto &blk = g.pkgs[g.currBlk];
    uint32_t sum = std::accumulate(blk.begin(), blk.end(), 0u);
    auto frame = makeDataFrame(g.currBlk, sum & 0xFF, blk);
    g.port.write(frame);
    g.state = State::WaitDataAck;
    ++g.currBlk;
    double pct = double(g.currBlk)/g.pkgs.size()*100.0;
    g.persent = pct;
    qDebug()<<(QString("下载进度: %1%").arg(pct, 0, 'f', 2));
}

/* ========================= 解帧 ========================= */
bool extractFrame(QByteArray &buf, QByteArray &frame)
{
    int start = buf.indexOf(char(HDR));
    if (start<0) return false;
    for (int i=start+1;i+1<buf.size();++i) {
        if (static_cast<uint8_t>(buf[i])==TAIL1 && static_cast<uint8_t>(buf[i+1])==TAIL2) {
            frame = buf.mid(start, i+2-start);
            buf.remove(0, i+2);
            return true;
        }
    }
    return false;
}

/* ========================= 处理消息 ========================= */
void handleMsg(const QByteArray &f)
{

    if (f.size()<7) return;
    uint8_t id = f[2], stat = f[3];


    if (id==UPDATE_START_ANS_ID) {
        if (stat==SUCCEED) { g.state=State::CmdUpgrade; sendCmd(UPDATE_APP_ID); qDebug()<<("握手成功......"); }
        else { g.state=State::Fail; qDebug()<<("更新开始命令失败!"); }
    } else if (id==UPDATE_APP_ANS_ID) {
        if (stat==SUCCEED) { g.state=State::SendPkgLen; sendCmdLen(); qDebug()<<("更新指令接收成功......"); }
        else { g.state=State::Fail; qDebug()<<("更新应用命令失败!"); }
    } else if (id==PACK_LENGTH_ANS) {
        if (stat==SUCCEED) { g.state=State::SendData; g.currBlk=0; qDebug()<<("包长接收成功......"); }
        else { g.state=State::Fail; qDebug()<<("包长度命令失败!"); }
    } else if (id==DATA_FRAME_ANS) {
        if (stat==FAIL && g.currBlk>0) --g.currBlk;
        g.state=State::SendData;
    } else if (id==DATA_FRAME_FINAL && stat==SUCCEED) {
        g.state=State::Done; qDebug()<<("下载已完成!");
    }
    if (g.state==State::SendData) sendDataBlk();
}

/* ========================= 读取串口 ========================= */
void onReadyRead()
{
    g.rxBuf.append(g.port.readAll());
    QByteArray f;
    while (extractFrame(g.rxBuf, f)) handleMsg(f);
}

/* ========================= 加载固件 ========================= */
void loadFirmware(const QString &path)
{
    QFile file(path);
    if (!file.open(QFile::ReadOnly)) throw QString("无法打开固件文件");
    QByteArray whole = file.readAll();
    g.pkgs.clear();
    for (int i=0;i<whole.size();i+=PKG_SIZE) {
        std::vector<uint8_t> p(PKG_SIZE, 0xFF);
        int l = PKG_SIZE >= static_cast<size_t>(whole.size() - i)?static_cast<size_t>(whole.size() - i):PKG_SIZE;//std::min(PKG_SIZE, static_cast<size_t>(whole.size() - i));
        memcpy(p.data(), whole.constData()+i, l);
        g.pkgs.push_back(std::move(p));
    }
    qDebug()<<("固件路径: " + path + ", 总包数: " + QString::number(g.pkgs.size()));
}


