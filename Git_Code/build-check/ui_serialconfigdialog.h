/********************************************************************************
** Form generated from reading UI file 'serialconfigdialog.ui'
**
** Created by: Qt User Interface Compiler version 5.12.8
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SERIALCONFIGDIALOG_H
#define UI_SERIALCONFIGDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_SerialConfigDialog
{
public:
    QTabWidget *tabWidget;
    QWidget *tab_3;
    QHBoxLayout *horizontalLayout;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout_3;
    QGroupBox *groupBox_2;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label;
    QComboBox *comboBox;
    QGroupBox *groupBox_3;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label_2;
    QComboBox *comboBox_2;
    QGroupBox *groupBox_4;
    QHBoxLayout *horizontalLayout_4;
    QLabel *label_3;
    QComboBox *comboBox_3;
    QGroupBox *groupBox_5;
    QHBoxLayout *horizontalLayout_5;
    QLabel *label_4;
    QComboBox *comboBox_4;
    QGroupBox *groupBox_7;
    QHBoxLayout *horizontalLayout_6;
    QLabel *label_6;
    QComboBox *comboBox_6;
    QGroupBox *groupBox_6;
    QHBoxLayout *horizontalLayout_7;
    QLabel *label_5;
    QLineEdit *lineEdit;
    QCheckBox *checkBox;
    QGroupBox *groupBox_8;
    QHBoxLayout *horizontalLayout_8;
    QPushButton *pushButton;
    QPushButton *pushButton_2;
    QWidget *tab_5;
    QWidget *tab_4;
    QWidget *tab_6;

    void setupUi(QDialog *SerialConfigDialog)
    {
        if (SerialConfigDialog->objectName().isEmpty())
            SerialConfigDialog->setObjectName(QString::fromUtf8("SerialConfigDialog"));
        SerialConfigDialog->resize(468, 542);
        SerialConfigDialog->setStyleSheet(QString::fromUtf8("/* \345\205\250\345\261\200\347\252\227\345\217\243\357\274\232\346\237\224\345\222\214\351\235\222\347\273\277\344\273\216\344\270\212\345\210\260\344\270\213\346\270\220\345\217\230\357\274\214\344\270\215\345\210\272\347\234\274 */\n"
"QDialog {\n"
"    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,\n"
"        stop:0 #f8fffc,\n"
"        stop:1 #edf9f4);\n"
"}\n"
"\n"
"/* Tab\346\240\207\347\255\276\346\240\217 \344\277\235\346\214\201\345\271\262\345\207\200 */\n"
"QDialog QTabWidget::pane {\n"
"    background-color: #ffffff;\n"
"    border: none;\n"
"}\n"
"QDialog QTabBar::tab {\n"
"    background: #edf3f0;\n"
"    padding: 6px 20px;\n"
"    margin-right:3px;\n"
"    border-top-left-radius:4px;\n"
"    border-top-right-radius:4px;\n"
"}\n"
"QDialog QTabBar::tab:selected {\n"
"    background-color:#ffffff;\n"
"}\n"
"\n"
"/* \345\210\206\347\273\204\346\241\206 */\n"
"QDialog QGroupBox {\n"
"    border:none;\n"
"    background:transparent;\n"
"    font-size:10pt;\n"
"}\n"
"\n"
"/* \344\277\235\347\225"
                        "\231\344\275\240\345\216\237\346\234\254\344\270\215\345\217\230\347\232\204\346\240\207\347\255\276\346\240\267\345\274\217 */\n"
"QDialog QLabel {\n"
"    color: #222222;\n"
"    font-size: 11pt;\n"
"    min-width: 90px;\n"
"}\n"
"\n"
"/* \344\277\235\347\225\231\344\275\240\345\216\237\346\234\254\344\270\215\345\217\230\347\232\204\344\270\213\346\213\211\346\241\206\346\240\267\345\274\217 */\n"
"QDialog QComboBox {\n"
"    border: 1px solid #cccccc;\n"
"    border-radius: 3px;\n"
"    padding: 4px 8px;\n"
"    background: white;\n"
"    min-height: 22px;\n"
"    font-size: 11pt;\n"
"}\n"
"QDialog QComboBox::drop-down {\n"
"    border: none;\n"
"}\n"
"/* \344\270\213\346\213\211\345\261\225\345\274\200\351\200\211\344\270\255\351\241\271 \346\224\271\346\210\220\351\235\222\347\273\277\351\253\230\344\272\256 */\n"
"QDialog QComboBox QAbstractItemView {\n"
"    border: 1px solid #cccccc;\n"
"    selection-background-color: #d7f3e8;\n"
"    selection-color:#222222;\n"
"}\n"
"\n"
"/* \345\215\225\350\241\214"
                        "\350\276\223\345\205\245\346\241\206 \345\220\214\344\270\213\346\213\211\346\241\206\351\243\216\346\240\274 */\n"
"QDialog QLineEdit {\n"
"    border: 1px solid #cccccc;\n"
"    border-radius: 3px;\n"
"    padding:4px 6px;\n"
"    background:white;\n"
"    font-size:11pt;\n"
"}\n"
"\n"
"/* \344\277\235\345\255\230\345\244\215\351\200\211\346\241\206 \346\224\276\345\244\247\351\235\222\347\273\277\351\243\216\346\240\274 */\n"
"QDialog QCheckBox {\n"
"    font-size:12pt;\n"
"    spacing:8px;\n"
"}\n"
"\n"
"/* \346\214\211\351\222\256 \350\276\271\346\241\206\346\270\205\346\231\260\351\206\222\347\233\256 */\n"
"QDialog QPushButton {\n"
"    min-width:130px;\n"
"    min-height:38px;\n"
"    font-size:12pt;\n"
"    border-radius:4px;\n"
"    border:1.6px solid #99bbaa;\n"
"}\n"
"QDialog QPushButton:first-of-type {\n"
"    background:#f0f7f4;\n"
"    color:#333;\n"
"}\n"
"/* \347\241\256\345\256\232\344\270\273\346\214\211\351\222\256 \345\212\240\346\267\261\351\235\222\347\273\277\347\252\201\345\207\272 */\n"
""
                        "QDialog QPushButton:last-of-type {\n"
"    background:#ccecdc;\n"
"    border-color:#77bb99;\n"
"    color:#114433;\n"
"}"));
        tabWidget = new QTabWidget(SerialConfigDialog);
        tabWidget->setObjectName(QString::fromUtf8("tabWidget"));
        tabWidget->setGeometry(QRect(30, 10, 408, 509));
        tab_3 = new QWidget();
        tab_3->setObjectName(QString::fromUtf8("tab_3"));
        horizontalLayout = new QHBoxLayout(tab_3);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        groupBox = new QGroupBox(tab_3);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        groupBox->setStyleSheet(QString::fromUtf8(""));
        verticalLayout_3 = new QVBoxLayout(groupBox);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        groupBox_2 = new QGroupBox(groupBox);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        horizontalLayout_2 = new QHBoxLayout(groupBox_2);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        label = new QLabel(groupBox_2);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout_2->addWidget(label);

        comboBox = new QComboBox(groupBox_2);
        comboBox->setObjectName(QString::fromUtf8("comboBox"));

        horizontalLayout_2->addWidget(comboBox);


        verticalLayout_3->addWidget(groupBox_2);

        groupBox_3 = new QGroupBox(groupBox);
        groupBox_3->setObjectName(QString::fromUtf8("groupBox_3"));
        horizontalLayout_3 = new QHBoxLayout(groupBox_3);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        label_2 = new QLabel(groupBox_3);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        horizontalLayout_3->addWidget(label_2);

        comboBox_2 = new QComboBox(groupBox_3);
        comboBox_2->addItem(QString());
        comboBox_2->addItem(QString());
        comboBox_2->addItem(QString());
        comboBox_2->addItem(QString());
        comboBox_2->addItem(QString());
        comboBox_2->addItem(QString());
        comboBox_2->addItem(QString());
        comboBox_2->setObjectName(QString::fromUtf8("comboBox_2"));

        horizontalLayout_3->addWidget(comboBox_2);


        verticalLayout_3->addWidget(groupBox_3);

        groupBox_4 = new QGroupBox(groupBox);
        groupBox_4->setObjectName(QString::fromUtf8("groupBox_4"));
        horizontalLayout_4 = new QHBoxLayout(groupBox_4);
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        label_3 = new QLabel(groupBox_4);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        horizontalLayout_4->addWidget(label_3);

        comboBox_3 = new QComboBox(groupBox_4);
        comboBox_3->addItem(QString());
        comboBox_3->addItem(QString());
        comboBox_3->addItem(QString());
        comboBox_3->addItem(QString());
        comboBox_3->setObjectName(QString::fromUtf8("comboBox_3"));

        horizontalLayout_4->addWidget(comboBox_3);


        verticalLayout_3->addWidget(groupBox_4);

        groupBox_5 = new QGroupBox(groupBox);
        groupBox_5->setObjectName(QString::fromUtf8("groupBox_5"));
        horizontalLayout_5 = new QHBoxLayout(groupBox_5);
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        label_4 = new QLabel(groupBox_5);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        horizontalLayout_5->addWidget(label_4);

        comboBox_4 = new QComboBox(groupBox_5);
        comboBox_4->addItem(QString());
        comboBox_4->addItem(QString());
        comboBox_4->addItem(QString());
        comboBox_4->addItem(QString());
        comboBox_4->setObjectName(QString::fromUtf8("comboBox_4"));

        horizontalLayout_5->addWidget(comboBox_4);


        verticalLayout_3->addWidget(groupBox_5);

        groupBox_7 = new QGroupBox(groupBox);
        groupBox_7->setObjectName(QString::fromUtf8("groupBox_7"));
        horizontalLayout_6 = new QHBoxLayout(groupBox_7);
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        label_6 = new QLabel(groupBox_7);
        label_6->setObjectName(QString::fromUtf8("label_6"));

        horizontalLayout_6->addWidget(label_6);

        comboBox_6 = new QComboBox(groupBox_7);
        comboBox_6->addItem(QString());
        comboBox_6->addItem(QString());
        comboBox_6->addItem(QString());
        comboBox_6->setObjectName(QString::fromUtf8("comboBox_6"));

        horizontalLayout_6->addWidget(comboBox_6);


        verticalLayout_3->addWidget(groupBox_7);

        groupBox_6 = new QGroupBox(groupBox);
        groupBox_6->setObjectName(QString::fromUtf8("groupBox_6"));
        horizontalLayout_7 = new QHBoxLayout(groupBox_6);
        horizontalLayout_7->setObjectName(QString::fromUtf8("horizontalLayout_7"));
        label_5 = new QLabel(groupBox_6);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        horizontalLayout_7->addWidget(label_5);

        lineEdit = new QLineEdit(groupBox_6);
        lineEdit->setObjectName(QString::fromUtf8("lineEdit"));

        horizontalLayout_7->addWidget(lineEdit);

        checkBox = new QCheckBox(groupBox_6);
        checkBox->setObjectName(QString::fromUtf8("checkBox"));

        horizontalLayout_7->addWidget(checkBox);


        verticalLayout_3->addWidget(groupBox_6);

        groupBox_8 = new QGroupBox(groupBox);
        groupBox_8->setObjectName(QString::fromUtf8("groupBox_8"));
        horizontalLayout_8 = new QHBoxLayout(groupBox_8);
        horizontalLayout_8->setObjectName(QString::fromUtf8("horizontalLayout_8"));
        pushButton = new QPushButton(groupBox_8);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));

        horizontalLayout_8->addWidget(pushButton);

        pushButton_2 = new QPushButton(groupBox_8);
        pushButton_2->setObjectName(QString::fromUtf8("pushButton_2"));

        horizontalLayout_8->addWidget(pushButton_2);


        verticalLayout_3->addWidget(groupBox_8);


        horizontalLayout->addWidget(groupBox);

        tabWidget->addTab(tab_3, QString());
        tabWidget->setTabText(tabWidget->indexOf(tab_3), QString::fromUtf8("\344\270\262\345\217\243"));
        tab_5 = new QWidget();
        tab_5->setObjectName(QString::fromUtf8("tab_5"));
        tabWidget->addTab(tab_5, QString());
        tabWidget->setTabText(tabWidget->indexOf(tab_5), QString::fromUtf8("UDP"));
        tab_4 = new QWidget();
        tab_4->setObjectName(QString::fromUtf8("tab_4"));
        tabWidget->addTab(tab_4, QString());
        tabWidget->setTabText(tabWidget->indexOf(tab_4), QString::fromUtf8("TCP"));
        tab_6 = new QWidget();
        tab_6->setObjectName(QString::fromUtf8("tab_6"));
        tabWidget->addTab(tab_6, QString());
        tabWidget->setTabText(tabWidget->indexOf(tab_6), QString::fromUtf8("USB"));

        retranslateUi(SerialConfigDialog);

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(SerialConfigDialog);
    } // setupUi

    void retranslateUi(QDialog *SerialConfigDialog)
    {
        SerialConfigDialog->setWindowTitle(QApplication::translate("SerialConfigDialog", "\351\200\232\344\277\241\346\216\245\345\217\243", nullptr));
        groupBox->setTitle(QApplication::translate("SerialConfigDialog", "\344\270\262\345\217\243\346\216\245\345\217\243\351\205\215\347\275\256", nullptr));
        groupBox_2->setTitle(QString());
        label->setText(QApplication::translate("SerialConfigDialog", "\344\270\262\345\217\243\345\217\267\357\274\232", nullptr));
        groupBox_3->setTitle(QString());
        label_2->setText(QApplication::translate("SerialConfigDialog", "\346\263\242\347\211\271\347\216\207\357\274\232", nullptr));
        comboBox_2->setItemText(0, QApplication::translate("SerialConfigDialog", "9600", nullptr));
        comboBox_2->setItemText(1, QApplication::translate("SerialConfigDialog", "38400", nullptr));
        comboBox_2->setItemText(2, QApplication::translate("SerialConfigDialog", "57600", nullptr));
        comboBox_2->setItemText(3, QApplication::translate("SerialConfigDialog", "115200", nullptr));
        comboBox_2->setItemText(4, QApplication::translate("SerialConfigDialog", "230400", nullptr));
        comboBox_2->setItemText(5, QApplication::translate("SerialConfigDialog", "460800", nullptr));
        comboBox_2->setItemText(6, QApplication::translate("SerialConfigDialog", "921600", nullptr));

        groupBox_4->setTitle(QString());
        label_3->setText(QApplication::translate("SerialConfigDialog", "\346\225\260\346\215\256\344\275\215\357\274\232", nullptr));
        comboBox_3->setItemText(0, QApplication::translate("SerialConfigDialog", "8", nullptr));
        comboBox_3->setItemText(1, QApplication::translate("SerialConfigDialog", "7", nullptr));
        comboBox_3->setItemText(2, QApplication::translate("SerialConfigDialog", "6", nullptr));
        comboBox_3->setItemText(3, QApplication::translate("SerialConfigDialog", "5", nullptr));

        groupBox_5->setTitle(QString());
        label_4->setText(QApplication::translate("SerialConfigDialog", "\346\240\241\351\252\214\344\275\215\357\274\232", nullptr));
        comboBox_4->setItemText(0, QApplication::translate("SerialConfigDialog", "NONE", nullptr));
        comboBox_4->setItemText(1, QApplication::translate("SerialConfigDialog", "ODD", nullptr));
        comboBox_4->setItemText(2, QApplication::translate("SerialConfigDialog", "EVEN", nullptr));
        comboBox_4->setItemText(3, QApplication::translate("SerialConfigDialog", "SPACE", nullptr));

        groupBox_7->setTitle(QString());
        label_6->setText(QApplication::translate("SerialConfigDialog", "\345\201\234\346\255\242\344\275\215\357\274\232", nullptr));
        comboBox_6->setItemText(0, QApplication::translate("SerialConfigDialog", "1", nullptr));
        comboBox_6->setItemText(1, QApplication::translate("SerialConfigDialog", "1.5", nullptr));
        comboBox_6->setItemText(2, QApplication::translate("SerialConfigDialog", "2", nullptr));

        groupBox_6->setTitle(QString());
        label_5->setText(QApplication::translate("SerialConfigDialog", "\346\226\207\344\273\266\345\220\215\347\247\260\357\274\232", nullptr));
        checkBox->setText(QApplication::translate("SerialConfigDialog", "\344\277\235\345\255\230", nullptr));
        groupBox_8->setTitle(QString());
        pushButton->setText(QApplication::translate("SerialConfigDialog", "\345\217\226\346\266\210", nullptr));
        pushButton_2->setText(QApplication::translate("SerialConfigDialog", "\347\241\256\345\256\232", nullptr));
    } // retranslateUi

};

namespace Ui {
    class SerialConfigDialog: public Ui_SerialConfigDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SERIALCONFIGDIALOG_H
