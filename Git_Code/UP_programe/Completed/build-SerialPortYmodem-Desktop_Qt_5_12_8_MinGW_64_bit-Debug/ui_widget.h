/********************************************************************************
** Form generated from reading UI file 'widget.ui'
**
** Created by: Qt User Interface Compiler version 5.12.8
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WIDGET_H
#define UI_WIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Widget
{
public:
    QGridLayout *gridLayout;
    QGroupBox *groupBox_5;
    QVBoxLayout *verticalLayout_4;
    QHBoxLayout *horizontalLayout_8;
    QLabel *label_9;
    QLineEdit *transmitPath_imu;
    QPushButton *transmitBrowse_imu;
    QPushButton *transmitButton_imu;
    QHBoxLayout *horizontalLayout_9;
    QLabel *label_10;
    QProgressBar *transmitProgress_imu;
    QGroupBox *groupBox_2;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_3;
    QLineEdit *transmitPath;
    QPushButton *transmitBrowse;
    QPushButton *transmitButton;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label_5;
    QProgressBar *transmitProgress;
    QGroupBox *groupBox;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QComboBox *comPort;
    QLabel *label_2;
    QComboBox *comBaudRate;
    QPushButton *comButton;
    QGroupBox *groupBox_3;
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout_4;
    QLabel *label_4;
    QLineEdit *receivePath;
    QPushButton *receiveBrowse;
    QPushButton *receiveButton;
    QHBoxLayout *horizontalLayout_5;
    QLabel *label_6;
    QProgressBar *receiveProgress;
    QGroupBox *groupBox_4;
    QVBoxLayout *verticalLayout_3;
    QHBoxLayout *horizontalLayout_6;
    QLabel *label_7;
    QLineEdit *transmitPath_nav;
    QPushButton *transmitBrowse_nav;
    QPushButton *transmitButton_nav;
    QHBoxLayout *horizontalLayout_7;
    QLabel *label_8;
    QProgressBar *transmitProgress_nav;

    void setupUi(QWidget *Widget)
    {
        if (Widget->objectName().isEmpty())
            Widget->setObjectName(QString::fromUtf8("Widget"));
        Widget->resize(600, 400);
        Widget->setMinimumSize(QSize(600, 400));
        Widget->setMaximumSize(QSize(16777215, 16777215));
        gridLayout = new QGridLayout(Widget);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        groupBox_5 = new QGroupBox(Widget);
        groupBox_5->setObjectName(QString::fromUtf8("groupBox_5"));
        verticalLayout_4 = new QVBoxLayout(groupBox_5);
        verticalLayout_4->setSpacing(6);
        verticalLayout_4->setContentsMargins(11, 11, 11, 11);
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        horizontalLayout_8 = new QHBoxLayout();
        horizontalLayout_8->setSpacing(6);
        horizontalLayout_8->setObjectName(QString::fromUtf8("horizontalLayout_8"));
        label_9 = new QLabel(groupBox_5);
        label_9->setObjectName(QString::fromUtf8("label_9"));

        horizontalLayout_8->addWidget(label_9);

        transmitPath_imu = new QLineEdit(groupBox_5);
        transmitPath_imu->setObjectName(QString::fromUtf8("transmitPath_imu"));
        transmitPath_imu->setMinimumSize(QSize(80, 0));
        transmitPath_imu->setReadOnly(true);

        horizontalLayout_8->addWidget(transmitPath_imu);

        transmitBrowse_imu = new QPushButton(groupBox_5);
        transmitBrowse_imu->setObjectName(QString::fromUtf8("transmitBrowse_imu"));
        transmitBrowse_imu->setEnabled(false);

        horizontalLayout_8->addWidget(transmitBrowse_imu);

        transmitButton_imu = new QPushButton(groupBox_5);
        transmitButton_imu->setObjectName(QString::fromUtf8("transmitButton_imu"));
        transmitButton_imu->setEnabled(false);

        horizontalLayout_8->addWidget(transmitButton_imu);


        verticalLayout_4->addLayout(horizontalLayout_8);

        horizontalLayout_9 = new QHBoxLayout();
        horizontalLayout_9->setSpacing(6);
        horizontalLayout_9->setObjectName(QString::fromUtf8("horizontalLayout_9"));
        label_10 = new QLabel(groupBox_5);
        label_10->setObjectName(QString::fromUtf8("label_10"));

        horizontalLayout_9->addWidget(label_10);

        transmitProgress_imu = new QProgressBar(groupBox_5);
        transmitProgress_imu->setObjectName(QString::fromUtf8("transmitProgress_imu"));
        transmitProgress_imu->setValue(0);

        horizontalLayout_9->addWidget(transmitProgress_imu);


        verticalLayout_4->addLayout(horizontalLayout_9);


        gridLayout->addWidget(groupBox_5, 1, 0, 1, 2);

        groupBox_2 = new QGroupBox(Widget);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        groupBox_2->setMaximumSize(QSize(16777215, 0));
        verticalLayout = new QVBoxLayout(groupBox_2);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        label_3 = new QLabel(groupBox_2);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        horizontalLayout_2->addWidget(label_3);

        transmitPath = new QLineEdit(groupBox_2);
        transmitPath->setObjectName(QString::fromUtf8("transmitPath"));
        transmitPath->setMinimumSize(QSize(80, 0));
        transmitPath->setReadOnly(true);

        horizontalLayout_2->addWidget(transmitPath);

        transmitBrowse = new QPushButton(groupBox_2);
        transmitBrowse->setObjectName(QString::fromUtf8("transmitBrowse"));
        transmitBrowse->setEnabled(false);

        horizontalLayout_2->addWidget(transmitBrowse);

        transmitButton = new QPushButton(groupBox_2);
        transmitButton->setObjectName(QString::fromUtf8("transmitButton"));
        transmitButton->setEnabled(false);

        horizontalLayout_2->addWidget(transmitButton);


        verticalLayout->addLayout(horizontalLayout_2);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setSpacing(6);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        label_5 = new QLabel(groupBox_2);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        horizontalLayout_3->addWidget(label_5);

        transmitProgress = new QProgressBar(groupBox_2);
        transmitProgress->setObjectName(QString::fromUtf8("transmitProgress"));
        transmitProgress->setValue(0);

        horizontalLayout_3->addWidget(transmitProgress);


        verticalLayout->addLayout(horizontalLayout_3);


        gridLayout->addWidget(groupBox_2, 3, 0, 1, 1);

        groupBox = new QGroupBox(Widget);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        groupBox->setMaximumSize(QSize(16777215, 80));
        horizontalLayout = new QHBoxLayout(groupBox);
        horizontalLayout->setSpacing(6);
        horizontalLayout->setContentsMargins(11, 11, 11, 11);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label = new QLabel(groupBox);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout->addWidget(label);

        comPort = new QComboBox(groupBox);
        comPort->setObjectName(QString::fromUtf8("comPort"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(comPort->sizePolicy().hasHeightForWidth());
        comPort->setSizePolicy(sizePolicy);
        comPort->setMaxVisibleItems(12);

        horizontalLayout->addWidget(comPort);

        label_2 = new QLabel(groupBox);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        horizontalLayout->addWidget(label_2);

        comBaudRate = new QComboBox(groupBox);
        comBaudRate->addItem(QString());
        comBaudRate->addItem(QString());
        comBaudRate->addItem(QString());
        comBaudRate->addItem(QString());
        comBaudRate->addItem(QString());
        comBaudRate->addItem(QString());
        comBaudRate->addItem(QString());
        comBaudRate->addItem(QString());
        comBaudRate->addItem(QString());
        comBaudRate->addItem(QString());
        comBaudRate->addItem(QString());
        comBaudRate->setObjectName(QString::fromUtf8("comBaudRate"));
        sizePolicy.setHeightForWidth(comBaudRate->sizePolicy().hasHeightForWidth());
        comBaudRate->setSizePolicy(sizePolicy);
        comBaudRate->setMaxVisibleItems(12);

        horizontalLayout->addWidget(comBaudRate);

        comButton = new QPushButton(groupBox);
        comButton->setObjectName(QString::fromUtf8("comButton"));

        horizontalLayout->addWidget(comButton);


        gridLayout->addWidget(groupBox, 0, 0, 1, 2);

        groupBox_3 = new QGroupBox(Widget);
        groupBox_3->setObjectName(QString::fromUtf8("groupBox_3"));
        groupBox_3->setMaximumSize(QSize(16777215, 0));
        verticalLayout_2 = new QVBoxLayout(groupBox_3);
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setContentsMargins(11, 11, 11, 11);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setSpacing(6);
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        label_4 = new QLabel(groupBox_3);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        horizontalLayout_4->addWidget(label_4);

        receivePath = new QLineEdit(groupBox_3);
        receivePath->setObjectName(QString::fromUtf8("receivePath"));
        receivePath->setMinimumSize(QSize(140, 0));
        receivePath->setReadOnly(true);

        horizontalLayout_4->addWidget(receivePath);

        receiveBrowse = new QPushButton(groupBox_3);
        receiveBrowse->setObjectName(QString::fromUtf8("receiveBrowse"));
        receiveBrowse->setEnabled(false);

        horizontalLayout_4->addWidget(receiveBrowse);

        receiveButton = new QPushButton(groupBox_3);
        receiveButton->setObjectName(QString::fromUtf8("receiveButton"));
        receiveButton->setEnabled(false);

        horizontalLayout_4->addWidget(receiveButton);


        verticalLayout_2->addLayout(horizontalLayout_4);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setSpacing(6);
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        label_6 = new QLabel(groupBox_3);
        label_6->setObjectName(QString::fromUtf8("label_6"));

        horizontalLayout_5->addWidget(label_6);

        receiveProgress = new QProgressBar(groupBox_3);
        receiveProgress->setObjectName(QString::fromUtf8("receiveProgress"));
        receiveProgress->setValue(0);

        horizontalLayout_5->addWidget(receiveProgress);


        verticalLayout_2->addLayout(horizontalLayout_5);


        gridLayout->addWidget(groupBox_3, 4, 0, 1, 2);

        groupBox_4 = new QGroupBox(Widget);
        groupBox_4->setObjectName(QString::fromUtf8("groupBox_4"));
        verticalLayout_3 = new QVBoxLayout(groupBox_4);
        verticalLayout_3->setSpacing(6);
        verticalLayout_3->setContentsMargins(11, 11, 11, 11);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setSpacing(6);
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        label_7 = new QLabel(groupBox_4);
        label_7->setObjectName(QString::fromUtf8("label_7"));

        horizontalLayout_6->addWidget(label_7);

        transmitPath_nav = new QLineEdit(groupBox_4);
        transmitPath_nav->setObjectName(QString::fromUtf8("transmitPath_nav"));
        transmitPath_nav->setMinimumSize(QSize(80, 0));
        transmitPath_nav->setReadOnly(true);

        horizontalLayout_6->addWidget(transmitPath_nav);

        transmitBrowse_nav = new QPushButton(groupBox_4);
        transmitBrowse_nav->setObjectName(QString::fromUtf8("transmitBrowse_nav"));
        transmitBrowse_nav->setEnabled(false);

        horizontalLayout_6->addWidget(transmitBrowse_nav);

        transmitButton_nav = new QPushButton(groupBox_4);
        transmitButton_nav->setObjectName(QString::fromUtf8("transmitButton_nav"));
        transmitButton_nav->setEnabled(false);

        horizontalLayout_6->addWidget(transmitButton_nav);


        verticalLayout_3->addLayout(horizontalLayout_6);

        horizontalLayout_7 = new QHBoxLayout();
        horizontalLayout_7->setSpacing(6);
        horizontalLayout_7->setObjectName(QString::fromUtf8("horizontalLayout_7"));
        label_8 = new QLabel(groupBox_4);
        label_8->setObjectName(QString::fromUtf8("label_8"));

        horizontalLayout_7->addWidget(label_8);

        transmitProgress_nav = new QProgressBar(groupBox_4);
        transmitProgress_nav->setObjectName(QString::fromUtf8("transmitProgress_nav"));
        transmitProgress_nav->setValue(0);

        horizontalLayout_7->addWidget(transmitProgress_nav);


        verticalLayout_3->addLayout(horizontalLayout_7);


        gridLayout->addWidget(groupBox_4, 2, 0, 1, 1);


        retranslateUi(Widget);

        comBaudRate->setCurrentIndex(3);


        QMetaObject::connectSlotsByName(Widget);
    } // setupUi

    void retranslateUi(QWidget *Widget)
    {
        Widget->setWindowTitle(QApplication::translate("Widget", "SerialPortYmodem", nullptr));
        groupBox_5->setTitle(QApplication::translate("Widget", "IMU\345\215\207\347\272\247(\346\263\242\347\211\271\347\216\207\345\233\272\345\256\232\357\274\232460800)", nullptr));
        label_9->setText(QApplication::translate("Widget", "\346\226\207\344\273\266\350\267\257\345\276\204\357\274\232", nullptr));
        transmitBrowse_imu->setText(QApplication::translate("Widget", "\346\265\217\350\247\210...", nullptr));
        transmitButton_imu->setText(QApplication::translate("Widget", "\345\217\221\351\200\201", nullptr));
        label_10->setText(QApplication::translate("Widget", "\344\274\240\350\276\223\350\277\233\345\272\246\357\274\232", nullptr));
        groupBox_2->setTitle(QApplication::translate("Widget", "Ymode\346\226\207\344\273\266\345\217\221\351\200\201(\346\226\207\344\273\266\344\270\262\345\217\243\344\272\222\344\274\240)", nullptr));
        label_3->setText(QApplication::translate("Widget", "\346\226\207\344\273\266\350\267\257\345\276\204\357\274\232", nullptr));
        transmitBrowse->setText(QApplication::translate("Widget", "\346\265\217\350\247\210...", nullptr));
        transmitButton->setText(QApplication::translate("Widget", "\345\217\221\351\200\201", nullptr));
        label_5->setText(QApplication::translate("Widget", "\344\274\240\350\276\223\350\277\233\345\272\246\357\274\232", nullptr));
        groupBox->setTitle(QApplication::translate("Widget", "\344\270\262\345\217\243\351\205\215\347\275\256", nullptr));
        label->setText(QApplication::translate("Widget", "\347\253\257\345\217\243\357\274\232", nullptr));
        label_2->setText(QApplication::translate("Widget", "\346\263\242\347\211\271\347\216\207\357\274\232", nullptr));
        comBaudRate->setItemText(0, QApplication::translate("Widget", "921600", nullptr));
        comBaudRate->setItemText(1, QApplication::translate("Widget", "460800", nullptr));
        comBaudRate->setItemText(2, QApplication::translate("Widget", "230400", nullptr));
        comBaudRate->setItemText(3, QApplication::translate("Widget", "115200", nullptr));
        comBaudRate->setItemText(4, QApplication::translate("Widget", "57600", nullptr));
        comBaudRate->setItemText(5, QApplication::translate("Widget", "38400", nullptr));
        comBaudRate->setItemText(6, QApplication::translate("Widget", "19200", nullptr));
        comBaudRate->setItemText(7, QApplication::translate("Widget", "9600", nullptr));
        comBaudRate->setItemText(8, QApplication::translate("Widget", "4800", nullptr));
        comBaudRate->setItemText(9, QApplication::translate("Widget", "2400", nullptr));
        comBaudRate->setItemText(10, QApplication::translate("Widget", "1200", nullptr));

        comBaudRate->setCurrentText(QApplication::translate("Widget", "115200", nullptr));
        comButton->setText(QApplication::translate("Widget", "\346\211\223\345\274\200", nullptr));
        groupBox_3->setTitle(QApplication::translate("Widget", "Ymode\346\226\207\344\273\266\346\216\245\346\224\266\357\274\210\346\226\207\344\273\266\344\270\262\345\217\243\344\272\222\344\274\240\357\274\211", nullptr));
        label_4->setText(QApplication::translate("Widget", "\346\226\207\344\273\266\350\267\257\345\276\204\357\274\232", nullptr));
        receiveBrowse->setText(QApplication::translate("Widget", "\346\265\217\350\247\210...", nullptr));
        receiveButton->setText(QApplication::translate("Widget", "\346\216\245\346\224\266", nullptr));
        label_6->setText(QApplication::translate("Widget", "\344\274\240\350\276\223\350\277\233\345\272\246\357\274\232", nullptr));
        groupBox_4->setTitle(QApplication::translate("Widget", "\345\257\274\350\210\252\346\227\247\347\211\210\345\215\207\347\272\247\357\274\210\346\263\242\347\211\271\347\216\207\345\233\272\345\256\232\357\274\232115200\357\274\211", nullptr));
        label_7->setText(QApplication::translate("Widget", "\346\226\207\344\273\266\350\267\257\345\276\204\357\274\232", nullptr));
        transmitBrowse_nav->setText(QApplication::translate("Widget", "\346\265\217\350\247\210...", nullptr));
        transmitButton_nav->setText(QApplication::translate("Widget", "\345\217\221\351\200\201", nullptr));
        label_8->setText(QApplication::translate("Widget", "\344\274\240\350\276\223\350\277\233\345\272\246\357\274\232", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Widget: public Ui_Widget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WIDGET_H
