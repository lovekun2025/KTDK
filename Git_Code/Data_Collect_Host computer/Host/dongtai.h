#ifndef DONGTAI_H
#define DONGTAI_H

#include <QWidget>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QResizeEvent>
#include <QStringList>

// 自动栅格控件：每个 Dock 内展示"字段名 / 数值 / 单位"的卡片网格。
// 卡片自动填满一行、放不下自动换行；帧内天青渐变、白色卡片边框、青蓝色数值。
class AutoGridWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AutoGridWidget(QWidget *parent = nullptr) : QWidget(parent)
    {
        setAttribute(Qt::WA_StyledBackground);
        setStyleSheet(
            "AutoGridWidget { background:qlineargradient(x1:0,y1:0,x2:0,y2:1,"
            " stop:0 #063f52, stop:1 #2fa4bf); }");

        mainV = new QVBoxLayout(this);
        mainV->setContentsMargins(0, 0, 0, 0);
        mainV->setSpacing(0);

        headerRow = new QHBoxLayout();
        headerRow->setContentsMargins(10, 6, 10, 4);
        headerRow->setSpacing(6);

        led = new QFrame();
        led->setFixedSize(9, 9);
        headerRow->addWidget(led);

        headerLabel = new QLabel("数据");
        headerLabel->setStyleSheet("color:#ffffff; font-size:14px; font-weight:bold; background:transparent; border:none;");
        headerLabel->setMinimumWidth(0);
        headerRow->addWidget(headerLabel);

        headerRow->addStretch(1);

        statusLabel = new QLabel("空闲");
        statusLabel->setMinimumWidth(0);
        headerRow->addWidget(statusLabel);

        mainV->addLayout(headerRow);

        grid = new QGridLayout();
        grid->setSpacing(6);
        grid->setContentsMargins(8, 2, 8, 8);
        mainV->addLayout(grid, 0);

        mainV->addStretch(1);   // 顶部对齐

        setStatus(false, 0);
    }

    void setHeaderTitle(const QString &title) { headerLabel->setText(title); }

    void setItems(const QStringList &items)
    {
        itemList = items;
        m_values.clear();
        unitList.clear();
        for (int i = 0; i < items.size(); i++) {
            m_values << "0";
            unitList << QString();
        }
        createItems();
    }

    void setUnits(const QStringList &units)
    {
        unitList = units;
        while (unitList.size() < itemList.size()) unitList << QString();
        createItems();
    }

    void setValue(int index, const QString &value)
    {
        if (index < 0 || index >= m_values.size()) return;
        m_values[index] = value;
        if (index < valueLabelList.size())
            valueLabelList[index]->setText(value);
    }

    // 设置状态：active 是否接收中；hz 为当前帧频（Hz，active 时显示）
    void setStatus(bool active, int hz)
    {
        led->setStyleSheet(active
            ? "background:#2ecc71; border-radius:4px;"
            : "background:#bfe9f2; border-radius:4px;");
        statusLabel->setText(active ? QString("接收中：%1Hz").arg(hz) : "空闲");
        statusLabel->setStyleSheet(QString(
            "color:%1; font-size:12px; background:transparent; border:none;")
            .arg(active ? "#ffffff" : "#e0f6fb"));
    }

    int getItemCount() const { return itemList.size(); }

protected:
    void resizeEvent(QResizeEvent *event) override
    {
        QWidget::resizeEvent(event);
        // 每列最小约130px(卡片130+间距6)：宽度变化时自动增减列数，子框架实时换行
        int newCols = qMax(1, (width() - 16 + 6) / (130 + 6)); // 16=左右边距,6=列间距
        if (newCols != cols) {
            cols = newCols;
            createItems();
        }
    }

    // 关键：Dock 最小宽度只按"一个卡片宽+边距"计算，避免被当前列数总宽卡住，
    // 这样缩小时列数才能持续减少、行数自动恢复
    // 高度最小=一行卡片+header；内容超高由外层 QScrollArea 滚动，帧框不会顶到串口号
    QSize minimumSizeHint() const override
    {
        int minH = 26 + 10 + 62; // header约26 + 上下边距10 + 一行卡片高
        return QSize(130 + 16, qMax(100, minH));
    }

private:
    QVBoxLayout *mainV;
    QHBoxLayout *headerRow;
    QGridLayout *grid;
    QFrame *led;
    QLabel *headerLabel;
    QLabel *statusLabel;

    QStringList itemList;
    QStringList unitList;
    QStringList m_values;
    QList<QLabel*> valueLabelList;
    int cols = 1;

    void createItems()
    {
        QLayoutItem *item;
        while ((item = grid->takeAt(0)) != nullptr) {
            delete item->widget();
            delete item;
        }
        valueLabelList.clear();

        for (int i = 0; i < itemList.size(); i++) {
            int row = i / cols;
            int col = i % cols;

            QWidget *box = new QWidget();
            box->setMinimumSize(130, 62);
            box->setStyleSheet(
                "QWidget { background-color:#0f3a46; border:2px solid #ffffff; border-radius:4px; }");

            QVBoxLayout *vbox = new QVBoxLayout(box);
            vbox->setContentsMargins(4, 3, 4, 3);
            vbox->setSpacing(0);

            QLabel *title = new QLabel(itemList[i]);
            title->setWordWrap(false);
            title->setStyleSheet("background:transparent; border:none; color:#d8f2f8; font-size:14px;");
            title->setAlignment(Qt::AlignCenter);

            QLabel *val = new QLabel(m_values.value(i, "0"));
            val->setWordWrap(false);
            val->setStyleSheet("background:transparent; border:none; color:#52dcff; font-size:16px; font-weight:bold;");
            val->setAlignment(Qt::AlignCenter);
            val->setTextInteractionFlags(Qt::TextSelectableByMouse);

            QLabel *unitLab = new QLabel(unitList.value(i));
            unitLab->setWordWrap(false);
            unitLab->setStyleSheet("background:transparent; border:none; color:#a8dce6; font-size:12px;");
            unitLab->setAlignment(Qt::AlignCenter);

            valueLabelList.append(val);
            vbox->addWidget(title);
            vbox->addWidget(val);
            vbox->addWidget(unitLab);
            grid->addWidget(box, row, col);
        }

        updateGeometry(); // 行数/列数变化后，让 Dock 重新应用最小高度
    }
};

#endif // DONGTAI_H
