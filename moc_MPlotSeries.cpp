/****************************************************************************
** Meta object code from reading C++ file 'MPlotSeries.h'
**
** Created: Mon Jan 25 18:16:33 2010
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "src/MPlot/MPlotSeries.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'MPlotSeries.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_MPlotSeries[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      20,   13,   12,   12, 0x05,

 // slots: signature, parameters, type, tag, flags
      57,   46,   12,   12, 0x09,
      97,   46,   12,   12, 0x09,
     156,  136,   12,   12, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_MPlotSeries[] = {
    "MPlotSeries\0\0series\0dataChanged(MPlotSeries*)\0"
    ",start,end\0handleRowsInserted(QModelIndex,int,int)\0"
    "handleRowsRemoved(QModelIndex,int,int)\0"
    "topLeft,bottomRight\0"
    "handleDataChanged(QModelIndex,QModelIndex)\0"
};

const QMetaObject MPlotSeries::staticMetaObject = {
    { &QGraphicsObject::staticMetaObject, qt_meta_stringdata_MPlotSeries,
      qt_meta_data_MPlotSeries, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &MPlotSeries::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *MPlotSeries::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *MPlotSeries::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MPlotSeries))
        return static_cast<void*>(const_cast< MPlotSeries*>(this));
    return QGraphicsObject::qt_metacast(_clname);
}

int MPlotSeries::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QGraphicsObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: dataChanged((*reinterpret_cast< MPlotSeries*(*)>(_a[1]))); break;
        case 1: handleRowsInserted((*reinterpret_cast< const QModelIndex(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 2: handleRowsRemoved((*reinterpret_cast< const QModelIndex(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 3: handleDataChanged((*reinterpret_cast< const QModelIndex(*)>(_a[1])),(*reinterpret_cast< const QModelIndex(*)>(_a[2]))); break;
        default: ;
        }
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void MPlotSeries::dataChanged(MPlotSeries * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
