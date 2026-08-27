/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../Host/mainwindow.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#include <QtCore/QVector>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_MainWindow_t {
    QByteArrayData data[23];
    char stringdata0[296];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MainWindow_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MainWindow_t qt_meta_stringdata_MainWindow = {
    {
QT_MOC_LITERAL(0, 0, 10), // "MainWindow"
QT_MOC_LITERAL(1, 11, 17), // "signalFrameUpdate"
QT_MOC_LITERAL(2, 29, 0), // ""
QT_MOC_LITERAL(3, 30, 6), // "pktIdx"
QT_MOC_LITERAL(4, 37, 15), // "QVector<double>"
QT_MOC_LITERAL(5, 53, 4), // "nums"
QT_MOC_LITERAL(6, 58, 18), // "signalProtoChanged"
QT_MOC_LITERAL(7, 77, 18), // "onOpenProtocolFile"
QT_MOC_LITERAL(8, 96, 18), // "onOpenConfigDialog"
QT_MOC_LITERAL(9, 115, 12), // "toggleSerial"
QT_MOC_LITERAL(10, 128, 14), // "onSerialConfig"
QT_MOC_LITERAL(11, 143, 12), // "SerialConfig"
QT_MOC_LITERAL(12, 156, 6), // "config"
QT_MOC_LITERAL(13, 163, 17), // "onSerialReadyRead"
QT_MOC_LITERAL(14, 181, 13), // "onSerialError"
QT_MOC_LITERAL(15, 195, 28), // "QSerialPort::SerialPortError"
QT_MOC_LITERAL(16, 224, 3), // "err"
QT_MOC_LITERAL(17, 228, 16), // "onProtocolLoaded"
QT_MOC_LITERAL(18, 245, 13), // "ProtoXmlModel"
QT_MOC_LITERAL(19, 259, 5), // "model"
QT_MOC_LITERAL(20, 265, 10), // "toggleSave"
QT_MOC_LITERAL(21, 276, 7), // "flushUi"
QT_MOC_LITERAL(22, 284, 11) // "updateStats"

    },
    "MainWindow\0signalFrameUpdate\0\0pktIdx\0"
    "QVector<double>\0nums\0signalProtoChanged\0"
    "onOpenProtocolFile\0onOpenConfigDialog\0"
    "toggleSerial\0onSerialConfig\0SerialConfig\0"
    "config\0onSerialReadyRead\0onSerialError\0"
    "QSerialPort::SerialPortError\0err\0"
    "onProtocolLoaded\0ProtoXmlModel\0model\0"
    "toggleSave\0flushUi\0updateStats"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MainWindow[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   74,    2, 0x06 /* Public */,
       6,    0,   79,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       7,    0,   80,    2, 0x08 /* Private */,
       8,    0,   81,    2, 0x08 /* Private */,
       9,    0,   82,    2, 0x08 /* Private */,
      10,    1,   83,    2, 0x08 /* Private */,
      13,    0,   86,    2, 0x08 /* Private */,
      14,    1,   87,    2, 0x08 /* Private */,
      17,    1,   90,    2, 0x08 /* Private */,
      20,    0,   93,    2, 0x08 /* Private */,
      21,    0,   94,    2, 0x08 /* Private */,
      22,    0,   95,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int, 0x80000000 | 4,    3,    5,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 11,   12,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 15,   16,
    QMetaType::Void, 0x80000000 | 18,   19,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MainWindow *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->signalFrameUpdate((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< const QVector<double>(*)>(_a[2]))); break;
        case 1: _t->signalProtoChanged(); break;
        case 2: _t->onOpenProtocolFile(); break;
        case 3: _t->onOpenConfigDialog(); break;
        case 4: _t->toggleSerial(); break;
        case 5: _t->onSerialConfig((*reinterpret_cast< const SerialConfig(*)>(_a[1]))); break;
        case 6: _t->onSerialReadyRead(); break;
        case 7: _t->onSerialError((*reinterpret_cast< QSerialPort::SerialPortError(*)>(_a[1]))); break;
        case 8: _t->onProtocolLoaded((*reinterpret_cast< const ProtoXmlModel(*)>(_a[1]))); break;
        case 9: _t->toggleSave(); break;
        case 10: _t->flushUi(); break;
        case 11: _t->updateStats(); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 0:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 1:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QVector<double> >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (MainWindow::*)(int , const QVector<double> & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MainWindow::signalFrameUpdate)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (MainWindow::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MainWindow::signalProtoChanged)) {
                *result = 1;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject MainWindow::staticMetaObject = { {
    &QMainWindow::staticMetaObject,
    qt_meta_stringdata_MainWindow.data,
    qt_meta_data_MainWindow,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MainWindow.stringdata0))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 12)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 12)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    }
    return _id;
}

// SIGNAL 0
void MainWindow::signalFrameUpdate(int _t1, const QVector<double> & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void MainWindow::signalProtoChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
