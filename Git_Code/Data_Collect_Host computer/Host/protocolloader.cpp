#include "protocolloader.h"
#include <QDir>
#include <QMessageBox>
#include <QDebug>
#include <QFile>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNodeList>
#include <cstring>
#include <limits>

// ===================== 文件内工具函数 =====================

// 归一化类型：把各种大小写/后缀归一成规范标识，返回默认字节数
static bool normalizeType(const QString &raw, QString &norm, int &defLen)
{
    QString t = raw.trimmed().toLower();
    if (t.endsWith("_t")) t.chop(2);   // uint16_t -> uint16
    t.remove(' ');
    norm = t;

    if (t == "byte" || t == "uint8" || t == "u8")            { norm = "u8";  defLen = 1; return true; }
    if (t == "sbyte" || t == "int8" || t == "i8" || t == "char") { norm = "i8"; defLen = 1; return true; }
    if (t == "uint16" || t == "u16")                         { norm = "u16"; defLen = 2; return true; }
    if (t == "int16" || t == "i16")                          { norm = "i16"; defLen = 2; return true; }
    if (t == "uint32" || t == "u32")                         { norm = "u32"; defLen = 4; return true; }
    if (t == "int32" || t == "i32")                          { norm = "i32"; defLen = 4; return true; }
    if (t == "uint64" || t == "u64")                         { norm = "u64"; defLen = 8; return true; }
    if (t == "int64" || t == "i64")                          { norm = "i64"; defLen = 8; return true; }
    if (t == "float" || t == "f32")                          { norm = "f32"; defLen = 4; return true; }
    if (t == "double" || t == "f64")                         { norm = "f64"; defLen = 8; return true; }
    if (t == "mbyte" || t == "i24")                          { norm = "i24"; defLen = 3; return true; }
    if (t == "string" || t == "str")                         { norm = "string"; defLen = 0; return true; }
    norm = t;
    defLen = 0;
    return false;
}

static bool isSignedType(const QString &norm)
{
    return norm == "i8" || norm == "i16" || norm == "i24" ||
           norm == "i32" || norm == "i64";
}

static bool isFloatType(const QString &norm)
{
    return norm == "f32" || norm == "f64";
}

// ---- 字节序安全读取 ----
static quint64 readUInt(const char *p, int n, bool little)
{
    quint64 v = 0;
    if (little) {
        for (int i = 0; i < n; i++) v |= quint64(quint8(p[i])) << (8 * i);
    } else {
        for (int i = 0; i < n; i++) v = (v << 8) | quint8(p[i]);
    }
    return v;
}

static qint64 readInt(const char *p, int n, bool little)
{
    quint64 u = readUInt(p, n, little);
    int bits = n * 8;
    if (bits >= 64) return qint64(u);
    quint64 sign = 1ULL << (bits - 1);
    return qint64((u ^ sign) - sign);
}

static double readFloat(const char *p, bool little)
{
    quint32 u = quint32(readUInt(p, 4, little));
    float f; std::memcpy(&f, &u, 4);
    return double(f);
}

static double readDouble(const char *p, bool little)
{
    quint64 u = readUInt(p, 8, little);
    double d; std::memcpy(&d, &u, 8);
    return d;
}

// ---- 数值格式化 ----
static QString formatNumber(double v)
{
    if (v == 0.0) return "0";
    double a = qAbs(v);
    if (a >= 1e-3 && a < 1e7) {
        QString s = QString::number(v, 'f', 6);
        if (s.contains('.')) {
            while (s.endsWith('0')) s.chop(1);
            if (s.endsWith('.')) s.chop(1);
        }
        return s;
    }
    return QString::number(v, 'g', 8);
}

// ---- CRC 计算（闭区间 [start, end]）----
static quint16 crc16Modbus(const QByteArray &f, int s, int e)
{
    quint16 crc = 0xFFFF;
    for (int i = s; i <= e; i++) {
        crc ^= quint8(f.at(i));
        for (int j = 0; j < 8; j++)
            crc = (crc & 1) ? (crc >> 1) ^ 0xA001 : (crc >> 1);
    }
    return crc;
}

static quint16 crc16Ccitt(const QByteArray &f, int s, int e)
{
    quint16 crc = 0xFFFF;
    for (int i = s; i <= e; i++) {
        crc ^= quint16(quint8(f.at(i))) << 8;
        for (int j = 0; j < 8; j++)
            crc = (crc & 0x8000) ? (crc << 1) ^ 0x1021 : (crc << 1);
    }
    return crc;
}

static quint32 crc32(const QByteArray &f, int s, int e)
{
    quint32 crc = 0xFFFFFFFF;
    for (int i = s; i <= e; i++) {
        crc ^= quint8(f.at(i));
        for (int j = 0; j < 8; j++)
            crc = (crc >> 1) ^ (0xEDB88320 & (0 - (crc & 1)));
    }
    return crc ^ 0xFFFFFFFF;
}

// ---- 单字段解码 ----
static bool decodeField(const ProtoField &f, const QByteArray &frame, int absOffset,
                        QString &out, double *numOut = nullptr)
{
    if (numOut) *numOut = std::numeric_limits<double>::quiet_NaN();

    int n = f.byteSize;
    if (n <= 0) { out = "--"; return false; }
    if (absOffset < 0 || absOffset + n > frame.size()) { out = "ERR"; return false; }

    const char *p = frame.constData() + absOffset;

    if (isFloatType(f.dataType)) {
        double v = 0;
        if (f.dataType == "f32" && n == 4)      v = readFloat(p, f.isLittle);
        else if (f.dataType == "f64" && n == 8) v = readDouble(p, f.isLittle);
        else                                    v = double(readInt(p, n, f.isLittle)); // 异常字节数退回整数
        v *= f.scale;
        out = formatNumber(v);
        if (numOut) *numOut = v;
        return true;
    }

    // 整数类型
    quint64 u = readUInt(p, n, f.isLittle);
    qint64  s = isSignedType(f.dataType) ? readInt(p, n, f.isLittle) : qint64(u);

    if (f.base == 16) {
        out = "0x" + QString("%1").arg(u, n * 2, 16, QLatin1Char('0')).toUpper();
        if (numOut) *numOut = double(u);
        return true;
    }
    if (f.base == 2) {
        out = "0b" + QString("%1").arg(u, n * 8, 2, QLatin1Char('0'));
        if (numOut) *numOut = double(u);
        return true;
    }
    if (qFuzzyCompare(f.scale, 1.0)) {
        out = isSignedType(f.dataType) ? QString::number(s) : QString::number(u);
        if (numOut) *numOut = isSignedType(f.dataType) ? double(s) : double(u);
        return true;
    }
    double v = (isSignedType(f.dataType) ? double(s) : double(u)) * f.scale;
    out = formatNumber(v);
    if (numOut) *numOut = v;
    return true;
}

// ===================== 类实现 =====================

ProtocolLoader::ProtocolLoader(QObject *parent)
    : QObject(parent)
{}

ProtoXmlModel g_GlobalProtoModel;

QList<QAction*> ProtocolLoader::loadProtocolToMenu(const QString &protoFolderPath,
                                                    QWidget     *parentWgt,
                                                    QMenu       *targetMenu)
{
    QList<QAction*> retActList;
    m_actionList.clear();
    m_protoFolderPath = protoFolderPath;

    QDir protocolDir(protoFolderPath);
    if (!protocolDir.exists())
    {
        qDebug() << "[ProtocolLoader] 协议文件夹不存在:" << protoFolderPath;
        return retActList;
    }

    protocolDir.setNameFilters({"*.xml"});
    protocolDir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
    QStringList protocolFiles = protocolDir.entryList();

    // 挂在"协议文件"子菜单下，避免菜单被几十个条目塞满
    QMenu* subMenu = nullptr;
    for (QObject* child : targetMenu->children())
    {
        QMenu* m = qobject_cast<QMenu*>(child);
        if (m && m->title() == "协议文件")
        {
            subMenu = m;
            break;
        }
    }
    if (!subMenu)
    {
        subMenu = new QMenu("协议文件", targetMenu);
        targetMenu->addMenu(subMenu);
    }

    for (const QString& fileName : protocolFiles)
    {
        QAction* protoAct = new QAction(fileName, parentWgt);
        protoAct->setCheckable(true);
        connect(protoAct, &QAction::triggered, this, &ProtocolLoader::onProtocolSelected);
        subMenu->addAction(protoAct);
        retActList.append(protoAct);
    }

    m_actionList = retActList;
    qDebug() << "[ProtocolLoader] 共生成" << retActList.size() << "个协议菜单项";
    return retActList;
}

void ProtocolLoader::onProtocolSelected()
{
    QAction* curAct = qobject_cast<QAction*>(sender());
    if (!curAct) return;

    // 单选互斥
    for (QAction* act : m_actionList)
        act->setChecked(act == curAct);

    QString fullPath = m_protoFolderPath + "/" + curAct->text();
    loadProtocolFile(fullPath);
}

void ProtocolLoader::loadProtocolFile(const QString &fullFilePath)
{
    ProtoXmlModel model = parseXmlFile(fullFilePath);
    if (model.packetList.isEmpty())
        return; // 解析失败已在 parseXmlFile 内提示

    g_GlobalProtoModel = model;
    emit signalLoadProtoFinish(g_GlobalProtoModel);
}

ProtoXmlModel ProtocolLoader::parseXmlFile(const QString &fullFilePath)
{
    ProtoXmlModel outModel;
    QFile file(fullFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::critical(nullptr, "文件打开失败", file.errorString());
        return outModel;
    }

    QDomDocument doc;
    QString errMsg;
    int errLine, errCol;
    if (!doc.setContent(&file, &errMsg, &errLine, &errCol))
    {
        file.close();
        QMessageBox::critical(nullptr, "XML解析错误",
            QString("行%1 列%2：%3").arg(errLine).arg(errCol).arg(errMsg));
        return outModel;
    }
    file.close();

    QDomElement root = doc.documentElement();
    if (root.tagName().compare("protocol", Qt::CaseInsensitive) != 0)
    {
        QMessageBox::warning(nullptr, "格式错误",
            "XML根节点必须为 <protocol>。\n(旧版 <ProtocolModelClass> 格式暂不支持)");
        return outModel;
    }

    QDomNodeList rootChilds = root.childNodes();
    for (int i = 0; i < rootChilds.size(); ++i)
    {
        QDomElement ele = rootChilds.at(i).toElement();
        if (ele.isNull()) continue;

        QString tag = ele.tagName();
        // 帧节点可能是 <pos> / <pos0> / <pos1> ...，跳过 <command> 等其它节点
        if (!tag.startsWith("pos", Qt::CaseInsensitive))
            continue;

        ProtoPacket pkt = parseSinglePosNode(ele);
        if (!pkt.fieldList.isEmpty())
            outModel.packetList.append(pkt);
    }

    qDebug() << "\n==== XML解析完成:" << fullFilePath
             << " 帧数量:" << outModel.packetList.size();
    for (int i = 0; i < outModel.packetList.size(); i++)
    {
        const ProtoPacket& pkt = outModel.packetList[i];
        qDebug() << "  [" << i << "]" << pkt.windowTitle
                 << " 帧头:" << pkt.headerHex
                 << " 字节序:" << (pkt.isLittle ? "小端" : "大端")
                 << " 字段数:" << pkt.fieldList.size()
                 << " 字段字节:" << pkt.calcTotalByte
                 << " 校验:" << crcTypeToStr(pkt.crcType)
                 << " 校验字节:" << pkt.crcResultByteCnt;
        if (pkt.isDynamicLen)
        {
            qDebug() << "      动态帧长: start=" << pkt.dynamicLenStart
                     << " size=" << pkt.dynamicLenSize
                     << " add=" << pkt.dynamicLenAdd;
        }
    }
    return outModel;
}

ProtoPacket ProtocolLoader::parseSinglePosNode(const QDomElement &posEle)
{
    ProtoPacket pkt;
    pkt.windowTitle = posEle.attribute("text");
    pkt.frameTotalByte = posEle.attribute("lenth").toInt();
    pkt.frameIsString = (posEle.attribute("isString").compare("true", Qt::CaseInsensitive) == 0);
    pkt.separator = posEle.attribute("separator");
    pkt.endMark = posEle.attribute("endMark");

    // 动态帧长属性
    int ls = posEle.attribute("lenthStart", "-1").toInt();
    int lsz = posEle.attribute("lenthSize", "0").toInt();
    int la = posEle.attribute("lenthAdd", "0").toInt();
    if (ls >= 0 && lsz > 0)
    {
        pkt.isDynamicLen = true;
        pkt.dynamicLenStart = ls;
        pkt.dynamicLenSize = lsz;
        pkt.dynamicLenAdd = la;
    }

    QString endian = posEle.attribute("byteEndian", "little").trimmed().toLower();
    pkt.isLittle = (endian != "big");

    int curOffset = 0;
    int calcTotalByte = 0;

    QDomNodeList subNodes = posEle.childNodes();
    for (int i = 0; i < subNodes.size(); ++i)
    {
        QDomElement subEle = subNodes.at(i).toElement();
        if (subEle.isNull()) continue;
        QString tag = subEle.tagName();

        if (tag.compare("HEADER", Qt::CaseInsensitive) == 0)
        {
            QString v = subEle.attribute("value");
            pkt.headerHex = v;
            if (pkt.frameIsString)
                pkt.headerRaw = v.toUtf8();           // ASCII 帧头（如 $GNGGA）
            else {
                QString h = v; h.remove(' ');
                pkt.headerRaw = QByteArray::fromHex(h.toUtf8());
            }
            pkt.headerLen = pkt.headerRaw.size();
            continue;
        }

        if (tag.compare("CHECKSUM", Qt::CaseInsensitive) == 0)
        {
            QString crcRawStr = subEle.attribute("CRC");
            pkt.crcResultByteCnt = subEle.attribute("count", "0").toInt();
            pkt.crcType = strToCrcType(crcRawStr);
            // 声明了 count 但未写算法 → 默认按 sum(ADD8)
            if (pkt.crcResultByteCnt > 0 && pkt.crcType == CrcAlgorithmType::None)
                pkt.crcType = CrcAlgorithmType::ADD8;

            QString endAttr = subEle.attribute("end", "");
            int rawStart = subEle.attribute("start", "0").toInt();
            int rawEnd = 0;
            bool endIsDynamic = false;

            // 检查 end 是否为动态表达式（如 "Lenth-2" 或 "Len-2"）
            if (!endAttr.isEmpty())
            {
                QString ea = endAttr.trimmed().toLower();
                if (ea.startsWith("lenth") || ea.startsWith("len"))
                {
                    // 动态表达式：以帧长为基础计算
                    endIsDynamic = true;
                }
                else
                {
                    rawEnd = endAttr.toInt();
                }
            }

            if (rawStart == 0 && rawEnd == 0 && !endIsDynamic)
            {
                // 未显式给出区间 → 全帧（不含校验字节），校验时按帧长计算
                pkt.crcStartOffset = 0;
                pkt.crcEndOffset = -1;
            }
            else if (endIsDynamic)
            {
                pkt.crcStartOffset = rawStart;
                pkt.crcEndOffset = -2; // -2 表示动态计算：帧长 - count - 1
            }
            else
            {
                pkt.crcStartOffset = rawStart;
                pkt.crcEndOffset = rawEnd;
            }
            continue;
        }

        // 其余节点一律视为字段
        ProtoField field = parseSingleFieldNode(subEle);
        field.byteOffset = curOffset;
        field.isLittle = pkt.isLittle;
        if (!field.isString)
        {
            curOffset += field.byteSize;
            calcTotalByte += field.byteSize;
        }
        pkt.fieldList.append(field);
    }

    pkt.calcTotalByte = calcTotalByte;

    // 可靠帧长计算
    if (pkt.isDynamicLen)
    {
        // 动态帧长：effectiveLen 只是一个最小估算值（用于等待足够数据）
        pkt.effectiveLen = pkt.headerLen + pkt.dynamicLenStart + pkt.dynamicLenSize + 256 + pkt.crcResultByteCnt;
        // 设置一个合理的最大值上限，实际长度由 computeFrameLen 动态计算
        // 先给一个基于字段累加的估算值作为最小等待长度
        int minLen = pkt.headerLen + pkt.dynamicLenStart + pkt.dynamicLenSize + 1 + pkt.crcResultByteCnt;
        if (pkt.effectiveLen < minLen) pkt.effectiveLen = minLen;
    }
    else if (pkt.crcResultByteCnt > 0 && pkt.crcEndOffset >= 0)
        pkt.effectiveLen = pkt.crcEndOffset + 1 + pkt.crcResultByteCnt;
    else if (pkt.frameTotalByte > 0)
        pkt.effectiveLen = pkt.frameTotalByte;
    else
        pkt.effectiveLen = pkt.headerLen + pkt.calcTotalByte + pkt.crcResultByteCnt;

    return pkt;
}

ProtoField ProtocolLoader::parseSingleFieldNode(const QDomElement &fieldEle)
{
    ProtoField f;
    f.tagName = fieldEle.tagName();
    f.showName = fieldEle.attribute("text");
    f.unit = fieldEle.attribute("unit");
    f.scale = fieldEle.attribute("scale", "1.0").toDouble();
    f.base = fieldEle.attribute("base", "10").toInt();
    if (f.base != 2 && f.base != 16) f.base = 10;

    QString norm;
    int defLen = 0;
    bool known = normalizeType(fieldEle.attribute("type"), norm, defLen);
    f.dataType = norm;
    f.byteSize = defLen;
    f.isString = (norm == "string");

    // 字段级 lenth 覆盖默认字节数（如 type="Int32" lenth="3" 三字节）
    int overrideLen = fieldEle.attribute("lenth", "0").toInt();
    if (overrideLen > 0)
        f.byteSize = overrideLen;

    if (!known && !f.isString && !f.dataType.isEmpty())
    {
        qDebug() << "[ProtocolLoader] 未知数据类型:" << f.dataType
                 << " 字段:" << f.tagName;
    }
    return f;
}

// ===================== 动态帧长计算 =====================
int ProtocolLoader::computeFrameLen(const ProtoPacket& pkt, const QByteArray& rawData)
{
    if (!pkt.isDynamicLen) return pkt.effectiveLen;

    int start = pkt.dynamicLenStart;
    int size = pkt.dynamicLenSize;
    int add = pkt.dynamicLenAdd;

    if (start < 0 || size <= 0) return pkt.effectiveLen;
    if (rawData.size() < start + size) return pkt.effectiveLen; // 数据不足，返回估算值

    quint64 lenVal = readUInt(rawData.constData() + start, size, pkt.isLittle);
    int totalLen = int(lenVal) + add;
    if (totalLen <= 0) return pkt.effectiveLen;
    return totalLen;
}

// ===================== 字节解析：原始帧 → 字段显示值 =====================
QStringList ProtocolLoader::parseFrameToValues(const ProtoPacket& pkt, const QByteArray& frameData)
{
    QStringList results;

    if (pkt.frameIsString)
    {
        QString s = QString::fromLatin1(frameData);
        if (!pkt.headerRaw.isEmpty())
        {
            QString h = QString::fromLatin1(pkt.headerRaw);
            if (s.startsWith(h)) s = s.mid(h.size());
        }
        while (s.endsWith('\n') || s.endsWith('\r')) s.chop(1);

        // NMEA 风格：* 之后是校验和，去掉
        int star = s.lastIndexOf('*');
        if (star >= 0) s = s.left(star);

        QChar sep = ',';
        if (!pkt.separator.isEmpty())
        {
            QString sp = pkt.separator.trimmed();
            if (!sp.isEmpty()) sep = sp.at(0);
        }
        const QStringList parts = s.split(sep, QString::KeepEmptyParts);
        for (int i = 0; i < pkt.fieldList.size(); i++)
            results << (i < parts.size() ? parts[i].trimmed() : QStringLiteral("--"));
        return results;
    }

    // 二进制定长协议
    for (const ProtoField& field : pkt.fieldList)
    {
        if (field.isString)
        {
            results << QStringLiteral("--");
            continue;
        }
        int absOffset = pkt.headerLen + field.byteOffset;
        QString out;
        decodeField(field, frameData, absOffset, out);
        results << out;
    }
    return results;
}

// ===================== 数值解析：原始帧 → 字段数值（供绘图） =====================
QVector<double> ProtocolLoader::parseFrameToDoubles(const ProtoPacket& pkt, const QByteArray& frameData)
{
    QVector<double> out;
    const double nan = std::numeric_limits<double>::quiet_NaN();

    if (pkt.frameIsString) {
        const QStringList values = parseFrameToValues(pkt, frameData);
        for (const QString& s : values) {
            bool ok = false;
            double d = s.toDouble(&ok);
            out << (ok ? d : nan);
        }
        return out;
    }

    for (const ProtoField& field : pkt.fieldList) {
        if (field.isString) {
            out << nan;
            continue;
        }
        int absOffset = pkt.headerLen + field.byteOffset;
        QString s;
        double d;
        decodeField(field, frameData, absOffset, s, &d);
        out << d;
    }
    return out;
}

// ===================== 校验 =====================
bool ProtocolLoader::verifyChecksum(const ProtoPacket& pkt, const QByteArray& frame)
{
    int cnt = pkt.crcResultByteCnt;
    if (pkt.crcType == CrcAlgorithmType::None || cnt <= 0)
        return true;

    int n = frame.size();
    if (n < cnt + 1) return false;

    int start = qMax(0, pkt.crcStartOffset);
    int end = pkt.crcEndOffset;
    if (end == -2)
        end = n - 2;  // 动态：帧长 - 2（对应 "Lenth-2" 表达式）
    else if (end < 0)
        end = n - cnt - 1;   // 默认全帧（不含校验字节）
    if (end > n - 1) end = n - 1;
    if (start > end) return false;

    QByteArray resultBytes = frame.right(cnt);
    QByteArray computed;

    switch (pkt.crcType)
    {
    case CrcAlgorithmType::ADD8_ZeroStart:
    case CrcAlgorithmType::ADD8: {
        quint8 s = 0;
        for (int i = start; i <= end; i++) s += quint8(frame.at(i));
        computed.append(char(s));
        break;
    }
    case CrcAlgorithmType::XOR8: {
        quint8 x = 0;
        for (int i = start; i <= end; i++) x ^= quint8(frame.at(i));
        computed.append(char(x));
        break;
    }
    case CrcAlgorithmType::ADD16: {
        quint16 s = 0;
        for (int i = start; i <= end; i++) s += quint8(frame.at(i));
        computed.append(char(s & 0xFF));
        computed.append(char((s >> 8) & 0xFF));
        break;
    }
    case CrcAlgorithmType::ModbusCRC16: {
        quint16 c = crc16Modbus(frame, start, end);
        computed.append(char(c & 0xFF));
        computed.append(char((c >> 8) & 0xFF));
        break;
    }
    case CrcAlgorithmType::CCITT_CRC16: {
        quint16 c = crc16Ccitt(frame, start, end);
        computed.append(char((c >> 8) & 0xFF));
        computed.append(char(c & 0xFF));
        break;
    }
    case CrcAlgorithmType::CRC32: {
        quint32 c = crc32(frame, start, end);
        computed.append(char(c & 0xFF));
        computed.append(char((c >> 8) & 0xFF));
        computed.append(char((c >> 16) & 0xFF));
        computed.append(char((c >> 24) & 0xFF));
        break;
    }
    default:
        return true;
    }

    if (computed.size() < cnt) return false;
    return computed.left(cnt) == resultBytes;
}
