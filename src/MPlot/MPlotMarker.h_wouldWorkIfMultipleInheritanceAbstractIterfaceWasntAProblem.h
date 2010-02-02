#ifndef __MPlotMarker_H__
#define __MPlotMarker_H__

#include <QGraphicsItem>
#include <QAbstractGraphicsShapeItem>
#include <QGraphicsRectItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsPolygonItem>
#include <QGraphicsItemGroup>
#include <QList>

#include <math.h>

namespace MPlotMarkerShape {
	enum Shape { Square = 1, Circle = 2, Triangle = 4, VerticalBeam = 8, HorizontalBeam = 16, DiagDownLeft = 32, DiagDownRight = 64, Point = 128, Cross, CrossSquare, CrossCircle, X, XSquare, XCircle, Star, StarSquare, StarCircle, PointSquare, PointCircle };
}

// Abstract class: all plot markers must have this:
class MPlotAbstractMarker : public QGraphicsItem {

public:
	// Pure Virtual functions:
	virtual void setSize(double width) = 0;
	
	virtual QPen pen() const = 0;
    virtual void setPen(const QPen &pen) = 0;
	
    virtual QBrush brush() const = 0;
    virtual void setBrush(const QBrush &brush) = 0;
	
protected:
	MPlotAbstractMarker(QGraphicsItem* parent = 0) : QGraphicsItem(parent) {}
////////////////////

};



class MPlotMarkerSquare : public QGraphicsRectItem, public MPlotAbstractMarker {

public:
	MPlotMarkerSquare(QGraphicsItem * parent = 0 , double size = 6) : QGraphicsRectItem(QRectF(-size/2, -size/2, size, size), parent) {
		QGraphicsRectItem::setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
	}
	
	virtual void setSize(double width) {
		QGraphicsRectItem::setRect(-width/2, -width/2, width, width);
	}
	
	// Silly repetition due to the way c++ doesn't know how to be intelligent under multiple inheritance:
	QPen pen() const { return QGraphicsRectItem::pen(); }
    void setPen(const QPen &pen) { QGraphicsRectItem::setPen(pen); }
	
    QBrush brush() const { return QGraphicsRectItem::brush(); }
    void setBrush(const QBrush &brush) { QGraphicsRectItem::setBrush(brush); }
	///////////////
	virtual QRectF boundingRect() const { return QGraphicsRectItem::boundingRect(); }
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0) { QGraphicsRectItem::paint(painter, option, widget); }
	///////////////
	
};

class MPlotMarkerCircle : public QGraphicsEllipseItem, public MPlotAbstractMarker {

	
public:	
	MPlotMarkerCircle(QGraphicsItem * parent = 0 , double size = 6) : QGraphicsEllipseItem(QRectF(-size/2, -size/2, size, size), parent) {
		QGraphicsEllipseItem::setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
	}
	
	virtual void setSize(double width) {
		QGraphicsEllipseItem::setRect(-width/2, -width/2, width, width);
	}
	
	// Silly repetition due to the way c++ doesn't know how to be intelligent under multiple inheritance:
	QPen pen() const { return QGraphicsEllipseItem::pen(); }
    void setPen(const QPen &pen) { QGraphicsEllipseItem::setPen(pen); }
	
    QBrush brush() const { return QGraphicsEllipseItem::brush(); }
    void setBrush(const QBrush &brush) { QGraphicsEllipseItem::setBrush(brush); }
	///////////////
	virtual QRectF boundingRect() const { return QGraphicsEllipseItem::boundingRect(); }
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0) { QGraphicsEllipseItem::paint(painter, option, widget); }
	///////////////
};

class MPlotMarkerTriangle : public QGraphicsPolygonItem, public MPlotAbstractMarker {

	
public:	
	MPlotMarkerTriangle(QGraphicsItem * parent = 0 , double size = 6 ) : QGraphicsPolygonItem(parent) {
		QGraphicsPolygonItem::setFlag(QGraphicsItem::ItemIgnoresTransformations, true); 
		setSize(size);
	}
	
	virtual void setSize(double width) {
		QPolygonF triangle;
		triangle << QPointF(-width/2, width/2/sqrt(3));
		triangle << QPointF(width/2, width/2/sqrt(3));
		triangle << QPointF(0, -width/sqrt(3));
		triangle << QPointF(-width/2, width/2/sqrt(3));
		
		QGraphicsPolygonItem::setPolygon(triangle);
	}	
	// Silly repetition due to the way c++ doesn't know how to be intelligent under multiple inheritance:
	QPen pen() const { return QGraphicsPolygonItem::pen(); }
    void setPen(const QPen &pen) { QGraphicsPolygonItem::setPen(pen); }
	
    QBrush brush() const { return QGraphicsPolygonItem::brush(); }
    void setBrush(const QBrush &brush) { QGraphicsPolygonItem::setBrush(brush); }
	///////////////
	virtual QRectF boundingRect() const { return QGraphicsPolygonItem::boundingRect(); }
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0) { QGraphicsPolygonItem::paint(painter, option, widget); }
	///////////////
};

class MPlotMarkerVerticalBeam : public QGraphicsLineItem, public MPlotAbstractMarker {
public:
	MPlotMarkerVerticalBeam(QGraphicsItem * parent = 0 , double size = 6) : QGraphicsLineItem(0, size/2, 0, -size/2, parent) {
		QGraphicsLineItem::setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
		setSize(size);
	}
	
	virtual void setSize(double size) {
		setLine(0, size/2, 0, -size/2);
	}
	
	QPen pen() const { return QGraphicsLineItem::pen(); }
    void setPen(const QPen &pen) { QGraphicsLineItem::setPen(pen); }
	
    QBrush brush() const { return QBrush(); }
    void setBrush(const QBrush & /*brush*/) { }
	/////////////////
	virtual QRectF boundingRect() const { return QGraphicsLineItem::boundingRect(); }
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0) { QGraphicsLineItem::paint(painter, option, widget); }
	///////////////
};

class MPlotMarkerHorizontalBeam : public QGraphicsLineItem, public MPlotAbstractMarker {
public:
	MPlotMarkerHorizontalBeam(QGraphicsItem * parent = 0 , double size = 6) : QGraphicsLineItem(size/2, 0, -size/2, 0, parent) {
		QGraphicsLineItem::setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
		setSize(size);
	}
	
	virtual void setSize(double size) {
		setLine(size/2, 0, -size/2, 0);
	}
	
	QPen pen() const { return QGraphicsLineItem::pen(); }
    void setPen(const QPen &pen) { QGraphicsLineItem::setPen(pen); }
	
    QBrush brush() const { return QBrush(); }
    void setBrush(const QBrush & /*brush*/) { }
	/////////////////
	virtual QRectF boundingRect() const { return QGraphicsLineItem::boundingRect(); }
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0) { QGraphicsLineItem::paint(painter, option, widget); }
	///////////////
};

class MPlotMarkerPoint : public QGraphicsLineItem, public MPlotAbstractMarker {
public:
	MPlotMarkerPoint(QGraphicsItem * parent = 0 , double size = 6) : QGraphicsLineItem(0.1, 0, -0.1, 0, parent) {
		QGraphicsLineItem::setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
		setSize(size);
	}
	
	// Size is irrelevant for points
	virtual void setSize(double /*size*/) {
		setLine(0.1, 0, -0.1, 0);
	}
	
	QPen pen() const { return QGraphicsLineItem::pen(); }
    void setPen(const QPen &pen) { QGraphicsLineItem::setPen(pen); }
	
    QBrush brush() const { return QBrush(); }
    void setBrush(const QBrush & /*brush*/) { }
	/////////////////
	virtual QRectF boundingRect() const { return QGraphicsLineItem::boundingRect(); }
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0) { QGraphicsLineItem::paint(painter, option, widget); }
	///////////////
};

// TODO: continue here...
class MPlotMarkerDiagDownLeft : public QGraphicsLineItem, public MPlotAbstractMarker {
public:
	MPlotMarkerDiagDownLeft(QGraphicsItem * parent = 0 , double size = 6) : QGraphicsLineItem(size/2, -size/2, -size/2, size/2, parent) {
		QGraphicsLineItem::setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
	}
	
	virtual void setSize(double size) {
		setLine(size/2, -size/2, -size/2, size/2);
	}
	
	QPen pen() const { return QGraphicsLineItem::pen(); }
    void setPen(const QPen &pen) { QGraphicsLineItem::setPen(pen); }
	
    QBrush brush() const { return QBrush(); }
    void setBrush(const QBrush & /*brush*/) { }
	/////////////////
	virtual QRectF boundingRect() const { return QGraphicsLineItem::boundingRect(); }
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0) { QGraphicsLineItem::paint(painter, option, widget); }
	///////////////
};

class MPlotMarkerDiagDownRight : public QGraphicsLineItem, public MPlotAbstractMarker {
public:
	MPlotMarkerDiagDownRight(QGraphicsItem * parent = 0 , double size = 6) : QGraphicsLineItem(-size/2, -size/2, size/2, size/2, parent) {
		QGraphicsLineItem::setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
	}
	
	virtual void setSize(double size) {
		setLine(-size/2, -size/2, size/2, size/2);
	}
	
	QPen pen() const { return QGraphicsLineItem::pen(); }
    void setPen(const QPen &pen) { QGraphicsLineItem::setPen(pen); }
	
    QBrush brush() const { return QBrush(); }
    void setBrush(const QBrush & /*brush*/) { }
	/////////////////
	virtual QRectF boundingRect() const { return QGraphicsLineItem::boundingRect(); }
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0) { QGraphicsLineItem::paint(painter, option, widget); }
	///////////////
};
////////////////////////

class MPlotMarkerCombined : public MPlotAbstractMarker {
public:
	MPlotMarkerCombined(int shapeCode, QGraphicsItem* parent = 0, double size = 6) : MPlotAbstractMarker(parent) {
		QGraphicsItem::setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
		

		if(shapeCode & MPlotMarkerShape::Square) {
			elements_ << new MPlotMarkerSquare(this, size);
			//addToGroup( elements_.last() );
		}
		if(shapeCode & MPlotMarkerShape::Circle) {
			elements_ << new MPlotMarkerCircle(this, size);
			//addToGroup( elements_.last() );
		}
		if(shapeCode & MPlotMarkerShape::Triangle) {
			elements_ << new MPlotMarkerTriangle(this, size);
			//addToGroup( elements_.last() );
		}
		if(shapeCode & MPlotMarkerShape::VerticalBeam) {
			elements_ << new MPlotMarkerVerticalBeam(this, size);
			//addToGroup( elements_.last() );
		}
		if(shapeCode & MPlotMarkerShape::HorizontalBeam) {
			elements_ << new MPlotMarkerHorizontalBeam(this, size);
			//addToGroup( elements_.last() );
		}
		if(shapeCode & MPlotMarkerShape::DiagDownLeft) {
			elements_ << new MPlotMarkerDiagDownLeft(this, size);
			//addToGroup( elements_.last() );
		}
		if(shapeCode & MPlotMarkerShape::DiagDownRight) {
			elements_ << new MPlotMarkerDiagDownRight(this, size);
			//addToGroup( elements_.last() );
		}
		if(shapeCode & MPlotMarkerShape::Point) {
			elements_ << new MPlotMarkerPoint(this, size);
			//addToGroup( elements_.last() );
		}
			
	}
	
	virtual ~MPlotMarkerCombined() {
		while(!elements_.isEmpty())
			delete elements_.takeFirst();
	}
	
	virtual void setSize(double size) {
		// TODO: how do we get to all of our members?
	}
	
	QPen pen() const { return pen_; }
    void setPen(const QPen &pen) { pen_ = pen; /*TODO: change all*/ }
	
	QBrush brush() const { return brush_; }
    void setBrush(const QBrush &brush) { brush_ = brush; /*TODO: change all*/ }
	/////////////////
	virtual QRectF boundingRect() const { return childrenBoundingRect(); }
	virtual void paint(QPainter * /*painter*/, const QStyleOptionGraphicsItem * /*option*/, QWidget */*widget = 0*/) {} /*QGraphicsItemGroup::paint(painter, option, widget);*/
	///////////////
	
	
protected:
	QList<MPlotAbstractMarker*> elements_;
	QPen pen_;
	QBrush brush_; // TODO: set pen and brush on creation of objects
	
};

class MPlotMarker {
	
public:
	
	// Static creator function:
	static MPlotAbstractMarker* create(MPlotMarkerShape::Shape type = MPlotMarkerShape::Square, QGraphicsItem* parent = 0, double size = 6) {
		
		switch(type) {
			case MPlotMarkerShape::Square: return new MPlotMarkerSquare(parent, size);
				break;
			case MPlotMarkerShape::Circle: return new MPlotMarkerCircle(parent, size);
				break;
			case MPlotMarkerShape::Triangle: return new MPlotMarkerTriangle(parent, size);
				break;
			case MPlotMarkerShape::VerticalBeam: return new MPlotMarkerVerticalBeam(parent, size);
				break;
			case MPlotMarkerShape::HorizontalBeam: return new MPlotMarkerHorizontalBeam(parent, size);
				break;
			case MPlotMarkerShape::DiagDownLeft: return new MPlotMarkerDiagDownLeft(parent, size);
				break;
			case MPlotMarkerShape::DiagDownRight: return new MPlotMarkerDiagDownRight(parent, size);
				break;
			case MPlotMarkerShape::Cross: return new MPlotMarkerCombined(MPlotMarkerShape::VerticalBeam | MPlotMarkerShape::HorizontalBeam, parent, size);
				break;
			case MPlotMarkerShape::CrossSquare: return new MPlotMarkerCombined(MPlotMarkerShape::Square | MPlotMarkerShape::VerticalBeam | MPlotMarkerShape::HorizontalBeam, parent, size);
				break;
			case MPlotMarkerShape::CrossCircle: return new MPlotMarkerCombined(MPlotMarkerShape::Circle | MPlotMarkerShape::VerticalBeam | MPlotMarkerShape::HorizontalBeam, parent, size);
				break;
			case MPlotMarkerShape::X: return new MPlotMarkerCombined(MPlotMarkerShape::DiagDownLeft | MPlotMarkerShape::DiagDownRight, parent, size);
				break;
			case MPlotMarkerShape::XSquare: return new MPlotMarkerCombined(MPlotMarkerShape::DiagDownLeft | MPlotMarkerShape::DiagDownRight | MPlotMarkerShape::Square, parent, size);
				break;
			case MPlotMarkerShape::XCircle: return new MPlotMarkerCombined(MPlotMarkerShape::DiagDownLeft | MPlotMarkerShape::DiagDownRight | MPlotMarkerShape::Circle, parent, size);
				break;
			case MPlotMarkerShape::Star: return new MPlotMarkerCombined(MPlotMarkerShape::DiagDownLeft | MPlotMarkerShape::DiagDownRight | MPlotMarkerShape::VerticalBeam | MPlotMarkerShape::HorizontalBeam, parent, size);
				break;
			case MPlotMarkerShape::StarCircle: return new MPlotMarkerCombined(MPlotMarkerShape::DiagDownLeft | MPlotMarkerShape::DiagDownRight | MPlotMarkerShape::VerticalBeam | MPlotMarkerShape::HorizontalBeam | MPlotMarkerShape::Circle, parent, size);
				break;
			case MPlotMarkerShape::StarSquare: return new MPlotMarkerCombined(MPlotMarkerShape::DiagDownLeft | MPlotMarkerShape::DiagDownRight | MPlotMarkerShape::VerticalBeam | MPlotMarkerShape::HorizontalBeam | MPlotMarkerShape::Square, parent, size);
				break;
			case MPlotMarkerShape::Point: return new MPlotMarkerPoint(parent, size);
				break;
			case MPlotMarkerShape::PointSquare: return new MPlotMarkerCombined(MPlotMarkerShape::Point | MPlotMarkerShape::Square, parent, size);
				break;
			case MPlotMarkerShape::PointCircle: return new MPlotMarkerCombined(MPlotMarkerShape::Point | MPlotMarkerShape::Circle, parent, size);
				break;
				
			default: return new MPlotMarkerSquare(parent, size);
				
				break;
		}
		
	}
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
