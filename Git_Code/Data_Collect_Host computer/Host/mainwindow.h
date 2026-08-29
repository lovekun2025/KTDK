#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>
#include <QElapsedTimer>
#include <QByteArray>
#include <QList>
#include <QHash>
#include <QSet>
#include <QVector>
#include <QComboBox>
#include <QPushButton>
#include <QLineEdit>
#include <QCheckBox>
#include <QLabel>
#include <QIcon>
#include <QFile>
#include <QAction>
#include <QMenu>
#include <QDockWidget>
#include <QMessageBox>
#include "serialconfigdialog.h"
#include "dongtai.h"
#include "protocolloader.h"
#include "ccyclebuffer.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class PlotWindow;
class QProcess;
class QPlainTextEdit;

// 一帧待刷新的数据（显示值 + 数值）
struct FrameUpdate
{
    QStringList values;
    QVector<double> nums;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void signalFrameUpdate(int pktIdx, const QVector<double>& nums); // 帧数据（绘图用）
    void signalProtoChanged();                                       // 协议已切换

private slots:
    void onOpenProtocolFile();                    // 文件→打开协议文件
    void onOpenConfigDialog();                    // 工具→串口设置
    void onLaunchBootApp();                       // 工具→固件升级（外部调用 BootApp.exe）
    void onConvertBinToSou();                     // 工具→bin转sou（调用 CreateSoul.exe）
    void openBatchSendWindow();                   // 工具→批量发送（新窗口）
    void toggleSerial();                          // 顶栏 打开/关闭串口
    void onSerialConfig(const SerialConfig& config);
    void onSerialReadyRead();
    void onSerialError(QSerialPort::SerialPortError err);
    void onProtocolLoaded(const ProtoXmlModel& model);
    void toggleSave();                            // 保存/取消 实时存储
    void flushUi();                               // 节流刷新界面
    void updateStats();                           // 每秒统计

private:
    Ui::MainWindow *ui;
    ProtocolLoader *m_protocolLoader = nullptr;
    QSerialPort *m_serialPort = nullptr;
    QByteArray m_rxBuffer;          // 帧解析缓冲（从环形缓冲抽取后）
    CCycleBuffer *m_rxCycle = nullptr;  // 串口接收环形缓冲

    QList<QDockWidget*> m_docks;
    QList<AutoGridWidget*> m_grids;
    QList<QAction*> m_viewActions;

    QComboBox *m_comboBaud = nullptr;
    QPushButton *m_btnOpenProto = nullptr;
    QPushButton *m_btnSave = nullptr;   // 保存/取消
    QLineEdit *m_saveNameEdit = nullptr; // 保存文件夹备注名

    QLabel *m_ledSerial = nullptr;
    QLabel *m_ledRx = nullptr;
    QLabel *m_ledTx = nullptr;
    QLabel *m_labStats = nullptr;

    QTimer *m_uiTimer = nullptr;
    QTimer *m_statsTimer = nullptr;
    QHash<int, FrameUpdate> m_pending;   // 待刷新数据（帧索引 -> 值 + 数值）
    QElapsedTimer m_ledElapsed;          // 帧指示灯计时
    QHash<int, qint64> m_lastRxMs;       // 每帧最近收包时间（用于1s灭灯）
    QHash<int, int> m_frameHz;           // 每帧当前频率（Hz）
    QHash<int, quint64> m_frameHzCounts; // 本秒每帧计数
    bool m_gotData = false;

    quint64 m_fpsFrames = 0;             // 本秒帧计数
    quint64 m_rxTotal = 0;               // 累计接收字节
    quint64 m_txTotal = 0;               // 累计发送字节
    quint64 m_rxThisSec = 0;             // 本秒接收字节
    quint64 m_txThisSec = 0;             // 本秒发送字节
    quint64 m_errCount = 0;              // 校验失败/重同步计数
    QString m_lastDir;

    // 实时存储
    bool m_saving = false;
    QString m_saveDir;
    QList<QFile*> m_saveFiles;           // 与 m_grids 一一对应

    QProcess *m_bootProc = nullptr;      // 固件升级子进程（BootApp.exe）
    QPlainTextEdit *m_rxView = nullptr;  // 接收区（数据回传显示）

    void setupMenus();
    void buildTopBar();
    void buildStatusBar();
    void applyStylesheet();
    void initializeGlobal();

    void clearFrameDocks();
    void refreshViewMenu();
    void refreshPortList();
    void openSerial(const SerialConfig& config);
    void setSerialConnected(bool on);
    void parseRxBuffer();
    void drainRxCycle();
    void openPlotWindow();
    void startSave();
    void stopSave();
    void saveFrameLine(int pktIdx, const QStringList& values);
    void updateGridDisplay(int pktIdx, const QStringList& values);
    QIcon makeFrameIcon(int idx, const QString& name);
    static void setLedColor(QLabel *led, const QString &color);
};

#endif // MAINWINDOW_H
