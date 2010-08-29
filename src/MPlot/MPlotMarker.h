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
	MPlotAbstractMarker(double size = 6, const QPen& pen = QPen(), const QBrush& brush = QBrush());
	~MPlotAbstractMarker();

	virtual void setSize(double size);
	virtual double size();

	virtual QPen pen() const;
	virtual void setPen(const QPen &pen1);

	virtual QBrush brush() const;
	virtual void setBrush(const QBrush &brush);


	virtual void paint(QPainter* painter) = 0;
	////////////////////

protected:
	double size_;	// size, in real pixels
	QPen pen_;
	QBrush brush_;
};

/*
class MPlotMarkerNone : public MPlotAbstractMarker {
public:
	MPlotMarkerNone(double size = 6, const QPen& pen = QPen(), const QBrush& brush = QBrush()) : MPlotAbstractMarker(size, pen, brush) {}
	virtual void paint(QPainter*) {}
};*/

class MPlotMarkerSquare : public MPlotAbstractMarker {

public:
	MPlotMarkerSquare(double size = 6, const QPen& pen = QPen(), const QBrush& brush = QBrush());

	virtual void paint(QPainter* painter);
};

class MPlotMarkerCircle : public MPlotAbstractMarker {

public:
	MPlotMarkerCircle(double size = 6, const QPen& pen = QPen(), const QBrush& brush = QBrush());
	virtual void paint(QPainter* painter);
};

class MPlotMarkerTriangle : public MPlotAbstractMarker {


public:
	MPlotMarkerTriangle(double size = 6, const QPen& pen = QPen(), const QBrush& brush = QBrush());

	virtual void setSize(double width);

	virtual void paint(QPainter* painter);

protected:
	QPolygonF triangle_;
};

class MPlotMarkerVerticalBeam : public MPlotAbstractMarker {
public:
	MPlotMarkerVerticalBeam(double size = 6, const QPen& pen = QPen(), const QBrush& brush = QBrush());

	virtual void setSize(double width);

	virtual void paint(QPainter* painter);

protected:
	QLineF line_;
};


class MPlotMarkerHorizontalBeam : public MPlotAbstractMarker {
public:
	MPlotMarkerHorizontalBeam(double size = 6, const QPen& pen = QPen(), const QBrush& brush = QBrush());

	virtual void setSize(double width);

	virtual void paint(QPainter* painter);

protected:
	QLineF line_;
};

class MPlotMarkerPoint : public MPlotAbstractMarker {
public:
	MPlotMarkerPoint(double size = 6, const QPen& pen = QPen(), const QBrush& brush = QBrush());

	virtual void setSize(double);

	virtual void paint(QPainter* painter);

protected:
	QLineF line_;
};


class MPlotMarkerDiagDownLeft : public MPlotAbstractMarker {
public:
	MPlotMarkerDiagDownLeft(double size = 6, const QPen& pen = QPen(), const QBrush& brush = QBrush());

	virtual void setSize(double size);

	virtual void paint(QPainter* painter);

protected:
	QLineF line_;
};


// A shorter version... This one has a length of size instead of size*sqrt(2). (Matches dia. of a circle of 'size')
class MPlotMarkerDiagDownLeftR : public MPlotAbstractMarker {
public:
	MPlotMarkerDiagDownLeftR(double size = 6, const QPen& pen = QPen(), const QBrush& brush = QBrush());

	virtual void setSize(double size);

	virtual void paint(QPainter* painter);

protected:
	QLineF line_;
};


class MPlotMarkerDiagDownRight : public MPlotAbstractMarker {
public:
	MPlotMarkerDiagDownRight(double size = 6, const QPen& pen = QPen(), const QBrush& brush = QBrush());

	virtual void setSize(double size);

	virtual void paint(QPainter* painter);

protected:
	QLineF line_;
};


class MPlotMarkerDiagDownRightR : public MPlotAbstractMarker {
public:
	MPlotMarkerDiagDownRightR(double size = 6, const QPen& pen = QPen(), const QBrush& brush = QBrush());

	virtual void setSize(double size);

	virtual void paint(QPainter* painter);

protected:
	QLineF line_;
};

/*
class MPlotMarkerText : public MPlotAbstractMarker {
public:
	MPlotMarkerText(const QString& text, QGraphicsItem* parent = 0, double size = 12) : MPlotAbstractMarker(parent), marker_(text, this), font_("Helvetica", size) {
		setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
		setFlag(QGraphicsItem::ItemHasNoContents, true);// all painting done by children
		setSize(size);
	}

	virtual void setSize(double size) {
		font_.setPointSize(size);
		marker_.setFont(font_);
	}

	/////////////////////
	QPen pen() const { return QPen(marker_.defaultTextColor()); }
	void setPen(const QPen &pen) { marker_.setDefaultTextColor(pen.color()); }

	QBrush brush() const { return QBrush(); }
	void setBrush(const QBrush & brush) {}
	///////////////
	virtual QRectF boundingRect() const { return marker_.boundingRect(); }
	virtual void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0) {}
	virtual QPainterPath shape () const { return marker_.shape(); }
	virtual bool contains ( const QPointF & point ) const { return marker_.contains(point); }
	virtual bool isObscuredBy ( const QGraphicsItem * item ) const { return marker_.isObscuredBy(item); }
	virtual QPainterPath opaqueArea () const { return marker_.opaqueArea(); }
	///////////////

protected:
	QGraphicsTextItem marker_;
	QFont font_;

};
*/

class MPlotMarkerCombined : public MPlotAbstractMarker {
public:
	MPlotMarkerCombined(int shapeCode, double size = 6, const QPen& pen = QPen(), const QBrush& brush = QBrush());

	virtual ~MPlotMarkerCombined();
	virtual void setSize(double size);

	virtual void setPen(const QPen &pen);

	virtual void setBrush(const QBrush &brush);



	virtual void paint(QPainter* painter);

protected:
	QList<MPlotAbstractMarker*> elements_;

};

class MPlotMarker {

public:

	// Static creator function:
	static MPlotAbstractMarker* create(MPlotMarkerShape::Shape type = MPlotMarkerShape::Square, double size = 6, const QPen& pen = QPen(), const QBrush& brush = QBrush());

};


///////////////// Old junk...

/*
 class MPlotMarker : public QGraphicsItem {

 enum Shape { Square, Circle, Triangle, Cross, X, CrossSquare, CrossCircle, XSquare, XCircle };

 public:
 MPlotMarker(QGraphicsItem * parent = 0 ) {
 setFlag(QGraphicsItem::ItemIgnoresTransformations, true);	// this makes the item always in pixel coordinates. Good or bad?

 setShape( MPlotMarkerShape::Square );
 setSize( 6 );
 }

 public slots:

 virtual void setSize(double width);
 void setShape(Shape shapeType) {


 }

 protected:

 QGraphicsItem*


 // We want our size to remain correct, always in logical coordinates (ie: percentage of a window size)
 // Therefore, pick up changes when moving to a new scene, or when the scene is scaled...
 QVariant itemChange(GraphicsItemChange change, const QVariant &value)
 {
 if (change == QGraphicsItem::ItemSceneChange || change == QGraphicsItem::ItemSceneHasChanged ) {

 adjustScale();
 }
 return QGraphicsItem::itemChange(change, value);
 }

 // TODO: This runs too early... sceneTransform doesn't pick up the new scene change.
 void adjustScale() {
 // Determine the scaling from the scene:

 QTransform sceneTr, rescaleTr;
 sceneTr = sceneTransform();
 double rx = 1/sceneTr.m11();
 double ry = 1/sceneTr.m22();
 rescaleTr.scale(rx, ry);
 setTransform(rescaleTr);

 }



 };*/

#endif
