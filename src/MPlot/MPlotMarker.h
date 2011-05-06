#ifndef __MPlotMarker_H__
#define __MPlotMarker_H__


#include <QPainter>
#include <QPolygonF>
#include <QLineF>
#include <QList>


#include <math.h>


namespace MPlotMarkerShape {
	enum Shape { None = 0, Square = 1, Circle = 2, Triangle = 4, VerticalBeam = 8, HorizontalBeam = 16, DiagDownLeft = 32, DiagDownRight = 64, DiagDownLeftR = 128, DiagDownRightR = 256, Point = 512, Cross, CrossSquare, CrossCircle, X, XSquare, XCircle, Star, StarSquare, StarCircle, PointSquare, PointCircle };
};



// We're using separate classes for each marker type instead of one class with a MarkerShape parameter. This is for performance reasons: a call to the virtual paint() event is faster than
// would be a switch() over the different marker types (inside a common paint() event.)    The paint() is called for every data point, so for large datasets it must be fast.

// Abstract class: all plot markers must have these:
class MPlotAbstractMarker {

public:
	MPlotAbstractMarker(qreal size = 6, const QPen& pen = QPen(), const QBrush& brush = QBrush());
	virtual ~MPlotAbstractMarker();

	virtual void setSize(qreal size) { size_ = size; }
	virtual qreal size() const { return size_; }

	virtual QPen pen() const { return pen_; }
	virtual void setPen(const QPen &pen1) { pen_ = pen1; }

	virtual QBrush brush() const { return brush_; }
	virtual void setBrush(const QBrush &brush) { brush_ = brush; }


	virtual void paint(QPainter* painter) = 0;
	////////////////////

protected:
	qreal size_;	///< size, in drawing coordinates (usually, real pixels, unless you have a transformed painter)
	QPen pen_;
	QBrush brush_;
};

/*
class MPlotMarkerNone : public MPlotAbstractMarker {
public:
	MPlotMarkerNone(qreal size = 6, const QPen& pen = QPen(), const QBrush& brush = QBrush()) : MPlotAbstractMarker(size, pen, brush) {}
	virtual void paint(QPainter*) {}
};*/

class MPlotMarkerSquare : public MPlotAbstractMarker {

public:
	MPlotMarkerSquare(qreal size = 6, const QPen& pen = QPen(), const QBrush& brush = QBrush());

	virtual void paint(QPainter* painter);
};

class MPlotMarkerCircle : public MPlotAbstractMarker {

public:
	MPlotMarkerCircle(qreal size = 6, const QPen& pen = QPen(), const QBrush& brush = QBrush());
	virtual void paint(QPainter* painter);
};

class MPlotMarkerTriangle : public MPlotAbstractMarker {


public:
	MPlotMarkerTriangle(qreal size = 6, const QPen& pen = QPen(), const QBrush& brush = QBrush());

	virtual void setSize(qreal width);

	virtual void paint(QPainter* painter);

protected:
	QPolygonF triangle_;
};

class MPlotMarkerVerticalBeam : public MPlotAbstractMarker {
public:
	MPlotMarkerVerticalBeam(qreal size = 6, const QPen& pen = QPen(), const QBrush& brush = QBrush());

	virtual void setSize(qreal width);

	virtual void paint(QPainter* painter);

protected:
	QLineF line_;
};


class MPlotMarkerHorizontalBeam : public MPlotAbstractMarker {
public:
	MPlotMarkerHorizontalBeam(qreal size = 6, const QPen& pen = QPen(), const QBrush& brush = QBrush());

	virtual void setSize(qreal width);

	virtual void paint(QPainter* painter);

protected:
	QLineF line_;
};

class MPlotMarkerPoint : public MPlotAbstractMarker {
public:
	MPlotMarkerPoint(qreal size = 6, const QPen& pen = QPen(), const QBrush& brush = QBrush());

	virtual void setSize(qreal);

	virtual void paint(QPainter* painter);

protected:
	QLineF line_;
};


class MPlotMarkerDiagDownLeft : public MPlotAbstractMarker {
public:
	MPlotMarkerDiagDownLeft(qreal size = 6, const QPen& pen = QPen(), const QBrush& brush = QBrush());

	virtual void setSize(qreal size);

	virtual void paint(QPainter* painter);

protected:
	QLineF line_;
};


// A shorter version... This one has a length of size instead of size*sqrt(2). (Matches dia. of a circle of 'size')
class MPlotMarkerDiagDownLeftR : public MPlotAbstractMarker {
public:
	MPlotMarkerDiagDownLeftR(qreal size = 6, const QPen& pen = QPen(), const QBrush& brush = QBrush());

	virtual void setSize(qreal size);

	virtual void paint(QPainter* painter);

protected:
	QLineF line_;
};


class MPlotMarkerDiagDownRight : public MPlotAbstractMarker {
public:
	MPlotMarkerDiagDownRight(qreal size = 6, const QPen& pen = QPen(), const QBrush& brush = QBrush());

	virtual void setSize(qreal size);

	virtual void paint(QPainter* painter);

protected:
	QLineF line_;
};


class MPlotMarkerDiagDownRightR : public MPlotAbstractMarker {
public:
	MPlotMarkerDiagDownRightR(qreal size = 6, const QPen& pen = QPen(), const QBrush& brush = QBrush());

	virtual void setSize(qreal size);

	virtual void paint(QPainter* painter);

protected:
	QLineF line_;
};



class MPlotMarkerCombined : public MPlotAbstractMarker {
public:
	MPlotMarkerCombined(int shapeCode, qreal size = 6, const QPen& pen = QPen(), const QBrush& brush = QBrush());

	virtual ~MPlotMarkerCombined();
	virtual void setSize(qreal size);

	virtual void setPen(const QPen &pen);

	virtual void setBrush(const QBrush &brush);



	virtual void paint(QPainter* painter);

protected:
	QList<MPlotAbstractMarker*> elements_;

};

class MPlotMarker {

public:

	// Static creator function:
	static MPlotAbstractMarker* create(MPlotMarkerShape::Shape type = MPlotMarkerShape::Square, qreal size = 6, const QPen& pen = QPen(), const QBrush& brush = QBrush());

};

#endif
