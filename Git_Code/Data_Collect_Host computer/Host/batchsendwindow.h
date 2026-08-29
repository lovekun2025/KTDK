#ifndef BATCHSENDWINDOW_H
#define BATCHSENDWINDOW_H

#include <QWidget>
#include <QListWidget>
#include <QTableWidget>
#include <QSpinBox>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QSerialPort>

// 批量发送窗口（独立页面，风格同绘图窗口）：
// 左侧页列表（添加/删除/重命名页），每页独立保存自己的 6 列表格：
//   编号(勾选) | 延迟(ms) | 重复次数 | 数据 | 备注 | 发送
// 底部：+ / x 增删行、自动回车、循环次数、启动批量发送（带指示灯）
// 所有页/行数据自动保存到本地文件，下次打开自动恢复
class BatchSendWindow : public QWidget
{
    Q_OBJECT
public:
    explicit BatchSendWindow(QSerialPort *port, QWidget *parent = nullptr);

signals:
    void signalTxBytes(qint64 bytes);   // 发送字节数（供主窗口同步到收发统计）
    void signalEchoData(const QString &text);   // 数据回传（发送到主窗口接收区）

protected:
    void closeEvent(QCloseEvent *e) override;

private:
    enum Col { ColCheck = 0, ColDelay, ColRepeat, ColData, ColRemark, ColSend, ColCount };

    struct Row {
        bool enabled = true;
        int delay = 200;
        int repeat = 1;
        QString data;
        QString remark;
    };
    struct Page {
        QString name;
        int loop = 1;
        QList<Row> rows;
    };

    QSerialPort *m_port;
    QListWidget *m_pageList;
    QTableWidget *m_table;
    QSpinBox *m_spLoop;
    QCheckBox *m_chkCR;
    QPushButton *m_btnStart, *m_btnStop;
    QLabel *m_led;
    QLabel *m_labStatus;
    QPushButton *m_btnAddRow, *m_btnDelRow;
    QPushButton *m_btnAddPage, *m_btnDelPage, *m_btnRenamePage;
    QPushButton *m_btnEcho;

    QList<Page> m_pages;
    int m_cur = -1;
    QString m_savePath;

    QTimer *m_sendTimer;
    QList<Row> m_sendRows;   // 当前轮要发送的行
    int m_rowIndex = 0;
    int m_repeatRemain = 0;
    int m_loopRemain = 0;
    bool m_sending = false;

    void addPage(const QString &name);
    void switchPage(int idx);
    void saveCurrentToPage();
    void loadPageToTable(int idx);
    void refreshList();

    void addRow();
    void addRow(const Row &row);
    void delRow();
    void renumber();
    void makeSendButton(int row);

    QByteArray encodeText(const QString &t);
    void sendData(const QByteArray &d);

    void onStartClicked();
    void sendNext();
    void setLed(bool on);

    void saveToFile();
    void loadFromFile();
};

#endif // BATCHSENDWINDOW_H
