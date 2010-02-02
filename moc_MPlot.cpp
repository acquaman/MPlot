/****************************************************************************
** Meta object code from reading C++ file 'MPlot.h'
**
** Created: Mon Jan 25 18:46:35 2010
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "src/MPlot/MPlot.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'MPlot.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_MPlot[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      14,    7,    6,    6, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_MPlot[] = {
    "MPlot\0\0series\0handleDataChanged(MPlotSeries*)\0"
};

const QMetaObject MPlot::staticMetaObject = {
    { &QGraphicsScene::staticMetaObject, qt_meta_stringdata_MPlot,
      qt_meta_data_MPlot, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &MPlot::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *MPlot::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *MPlot::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MPlot))
        return static_cast<void*>(const_cast< MPlot*>(this));
    return QGraphicsScene::qt_metacast(_clname);
}

int MPlot::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QGraphicsScene::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: handleDataChanged((*reinterpret_cast< MPlotSeries*(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 1;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
