/****************************************************************************
** Meta object code from reading C++ file 'MainWindow.h'
**
** Created: Mon Sep 7 12:13:40 2015
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "MainWindow.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'MainWindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_MainWindow[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      17,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      12,   11,   11,   11, 0x08,
      36,   11,   11,   11, 0x08,
      74,   11,   11,   11, 0x08,
     109,   11,   11,   11, 0x08,
     151,  145,   11,   11, 0x08,
     177,  145,   11,   11, 0x08,
     203,  145,   11,   11, 0x08,
     229,  145,   11,   11, 0x08,
     268,  145,   11,   11, 0x08,
     297,  145,   11,   11, 0x08,
     316,  145,   11,   11, 0x08,
     337,   11,   11,   11, 0x08,
     360,  145,   11,   11, 0x08,
     390,  145,   11,   11, 0x08,
     429,  417,   11,   11, 0x08,
     471,   11,   11,   11, 0x08,
     486,  417,   11,   11, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_MainWindow[] = {
    "MainWindow\0\0onActionOpenTriggered()\0"
    "onActionRenderAllSnapshotsTriggered()\0"
    "onActionRenderAllSlicesTriggered()\0"
    "onActionSwitchPropertiesTriggered()\0"
    "value\0onSliderValueChanged(int)\0"
    "onSliceIValueChanged(int)\0"
    "onSliceJValueChanged(int)\0"
    "onVerticalScaleSliderValueChanged(int)\0"
    "onROISliderValueChanged(int)\0"
    "onROIToggled(bool)\0onSliceToggled(bool)\0"
    "onRenderStyleChanged()\0"
    "onCoordinateGridToggled(bool)\0"
    "onPerspectiveToggled(bool)\0item,column\0"
    "onItemDoubleClicked(QTreeWidgetItem*,int)\0"
    "onShowGLInfo()\0"
    "onTreeWidgetItemChanged(QTreeWidgetItem*,int)\0"
};

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        MainWindow *_t = static_cast<MainWindow *>(_o);
        switch (_id) {
        case 0: _t->onActionOpenTriggered(); break;
        case 1: _t->onActionRenderAllSnapshotsTriggered(); break;
        case 2: _t->onActionRenderAllSlicesTriggered(); break;
        case 3: _t->onActionSwitchPropertiesTriggered(); break;
        case 4: _t->onSliderValueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: _t->onSliceIValueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: _t->onSliceJValueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 7: _t->onVerticalScaleSliderValueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 8: _t->onROISliderValueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 9: _t->onROIToggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 10: _t->onSliceToggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 11: _t->onRenderStyleChanged(); break;
        case 12: _t->onCoordinateGridToggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 13: _t->onPerspectiveToggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 14: _t->onItemDoubleClicked((*reinterpret_cast< QTreeWidgetItem*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 15: _t->onShowGLInfo(); break;
        case 16: _t->onTreeWidgetItemChanged((*reinterpret_cast< QTreeWidgetItem*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData MainWindow::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject MainWindow::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_MainWindow,
      qt_meta_data_MainWindow, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &MainWindow::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MainWindow))
        return static_cast<void*>(const_cast< MainWindow*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 17)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 17;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
