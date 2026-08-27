#ifndef PROTOCOLLOADER_H
#define PROTOCOLLOADER_H

#include <QObject>
#include <QList>
#include <QMenu>
#include <QAction>
#include <QWidget>
#include <QString>
#include <QStringList>
#include <QByteArray>
#include <QVector>

// ===================== 校验算法枚举 =====================
enum class CrcAlgorithmType
{
    None,           // 无校验
    ModbusCRC16,
    CCITT_CRC16,
    CRC32,
    ADD8_ZeroStart, // 全帧累加8位
    ADD8,           // 指定区间累加8位
    XOR8,           // 指定区间异或8位
    ADD16           // 指定区间累加16位
};

// XML字符串 → 校验枚举转换（大小写不敏感，缺省/SUM 都归为 ADD8）
inline CrcAlgorithmType strToCrcType(const QString& crcStr)
{
    QString s = crcStr.trimmed().toUpper();
    if (s.isEmpty() || s == "NONE")
        return CrcAlgorithmType::None;
    if (s == "SUM" || s == "SUM8" || s == "ADD8")
        return CrcAlgorithmType::ADD8;
    if (s == "0-ADD8")
        return CrcAlgorithmType::ADD8_ZeroStart;
    if (s == "XOR8")
        return CrcAlgorithmType::XOR8;
    if (s == "ADD16")
        return CrcAlgorithmType::ADD16;
    if (s == "MODBUSCRC16" || s == "CRC16" || s == "CRC16-MODBUS" || s == "CRC16/MODBUS")
        return CrcAlgorithmType::ModbusCRC16;
    if (s == "CCITT-CRC16" || s == "CCITT" || s == "CRC16-CCITT")
        return CrcAlgorithmType::CCITT_CRC16;
    if (s == "CRC32")
        return CrcAlgorithmType::CRC32;
    return CrcAlgorithmType::None;
}

inline QString crcTypeToStr(CrcAlgorithmType type)
{
    switch (type)
    {
    case CrcAlgorithmType::None:            return "None 无校验";
    case CrcAlgorithmType::ModbusCRC16:    return "ModbusCRC16";
    case CrcAlgorithmType::CCITT_CRC16:    return "CCITT-CRC16";
    case CrcAlgorithmType::CRC32:          return "CRC32";
    case CrcAlgorithmType::ADD8_ZeroStart: return "0-ADD8 全帧累加8位";
    case CrcAlgorithmType::ADD8:           return "ADD8 累加8位";
    case CrcAlgorithmType::XOR8:           return "XOR8 异或8位";
    case CrcAlgorithmType::ADD16:          return "ADD16 累加16位";
    default: return "未知校验";
    }
}

// ===================== 数据模型定义 =====================
// 单个字段：区分定长二进制 / string字符串
struct ProtoField
{
    QString tagName;      // XML 节点名（原始）
    QString dataType;     // 归一化后的类型标识：u8/i8/u16/i16/u32/i32/u64/i64/f32/f64/i24/string
    QString showName;     // text 显示名
    QString unit;         // 单位
    double scale = 1.0;   // 缩放系数
    int byteSize = 0;     // 占用字节数（string 为 0）
    int byteOffset = 0;   // 相对帧头之后的字节偏移
    int base = 10;        // 显示进制：10 / 16 / 2
    bool isLittle = true; // 字节序（继承帧级 byteEndian）
    bool isString = false;
};

// 单个 pos 数据包
struct ProtoPacket
{
    QString windowTitle;
    int frameTotalByte = 0;   // XML lenth 属性（可能不准，仅作参考）
    QString headerHex;        // 原始帧头字符串（日志用）
    QByteArray headerRaw;     // 帧头字节（二进制 fromHex / 字符串 UTF-8）
    int headerLen = 0;
    int effectiveLen = 0;     // 可靠帧长：CHECKSUM end+count+1 优先，其次 lenth，再字段累加

    QList<ProtoField> fieldList;
    int calcTotalByte = 0;    // 字段累加字节（不含帧头/校验）
    bool frameIsString = false;
    bool isLittle = true;     // 帧级字节序（byteEndian）
    QString separator;        // 字符串协议分隔符（如 ", *"）
    QString endMark;          // 字符串协议帧尾（如 "0D 0A"）

    // 校验参数
    CrcAlgorithmType crcType = CrcAlgorithmType::None;
    int crcStartOffset = 0;
    int crcEndOffset = -1;    // <0 表示"全帧(不含校验字节)"
    int crcResultByteCnt = 0; // 校验结果占用字节数（在帧尾）
};

// XML整体解析结果容器
struct ProtoXmlModel
{
    QList<ProtoPacket> packetList;
};

class QDomElement;

class ProtocolLoader : public QObject
{
    Q_OBJECT
public:
    explicit ProtocolLoader(QObject *parent = nullptr);

    // 扫描协议文件夹生成菜单 Action（挂到指定菜单下）
    QList<QAction*> loadProtocolToMenu(const QString &protoFolderPath,
                                       QWidget     *parentWgt,
                                       QMenu       *targetMenu);

    // 解析指定 xml 文件返回完整模型（出错时内部会弹窗提示）
    ProtoXmlModel parseXmlFile(const QString& fullFilePath);

    // 统一加载入口：解析 → 覆盖全局模型 → 发信号（菜单项与文件对话框共用）
    void loadProtocolFile(const QString &fullFilePath);

    // 字节解析：把原始帧数据按字段定义转成显示值列表
    static QStringList parseFrameToValues(const ProtoPacket& pkt, const QByteArray& frameData);

    // 字节解析：返回各字段的数值（缩放后）；字符串/越界为 NaN，供绘图使用
    static QVector<double> parseFrameToDoubles(const ProtoPacket& pkt, const QByteArray& frameData);

    // 校验：按 pkt 的校验参数验证整帧是否合法（None/无校验字节时恒真）
    static bool verifyChecksum(const ProtoPacket& pkt, const QByteArray& frame);

signals:
    void signalLoadProtoFinish(const ProtoXmlModel& model);

private slots:
    void onProtocolSelected();

private:
    ProtoPacket parseSinglePosNode(const QDomElement& posEle);
    ProtoField  parseSingleFieldNode(const QDomElement& fieldEle);

    QString m_protoFolderPath;
    QList<QAction*> m_actionList;
};

extern ProtoXmlModel g_GlobalProtoModel;

#endif // PROTOCOLLOADER_H
