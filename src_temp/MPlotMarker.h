#ifndef __MPlotMarker_H__
#define __MPlotMarker_H__

#include <QGraphicsItem>
#include <QAbstractGraphicsShapeItem>
#include <QGraphicsRectItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsPolygonItem>

#include <math.h>

// Abstract class: all plot markers must have this:
class MPlotAbstractMarker {

	
public:
	virtual void setSize(double width) = 0;
	
};



class MPlotMarkerSquare : public QGraphicsRectItem, public MPlotAbstractMarker {

	
public:
	MPlotMarkerSquare(QGraphicsItem * parent = 0 , double size = 6) : QGraphicsRectItem(QRectF(-size/2, -size/2, size, size), parent) {
		QGraphicsRectItem::setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
	}
	
	virtual void setSize(double width) {
		QGraphicsRectItem::setRect(-width/2, -width/2, width, width);
	}
	
};

class MPlotMarkerCircle : public QGraphicsEllipseItem, public MPlotAbstractMarker {

	
public:	
	MPlotMarkerCircle(QGraphicsItem * parent = 0 , double size = 6) : QGraphicsEllipseItem(QRectF(-size/2, -size/2, size, size), parent) {
		QGraphicsEllipseItem::setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
	}
	
	virtual void setSize(double width) {
		QGraphicsEllipseItem::setRect(-width/2, -width/2, width, width);
	}
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
};

class MPlotMarkerVerticalBeam : public QGraphicsPolygonItem, public MPlotAbstractMarker {
public:
	MPlotMarkerVerticalBeam(QGraphicsItem * parent = 0 , double size = 6) : QGraphicsPolygonItem(parent) {
		setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
		setSize(size);
	}
	
	virtual void setSize(double size) {
		QPolygonF line;
		line << QPointF(0, size/2);
		line << QPointF(0, -size/2);
		setPolygon(line);
	}
};

class MPlotMarkerHorizontalBeam : public QGraphicsPolygonItem, public MPlotAbstractMarker {
public:
	MPlotMarkerHorizontalBeam(QGraphicsItem * parent = 0 , double size = 6) : QGraphicsPolygonItem(parent) {
		setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
		setSize(size);
	}
	
	virtual void setSize(double size) {
		QPolygonF line;
		line << QPointF(size/2, 0);
		line << QPointF(-size/2, 0);
		setPolygon(line);
	}
};

class MPlotMarkerPoint : public QGraphicsPolygonItem, public MPlotAbstractMarker {
public:
	MPlotMarkerPoint(QGraphicsItem * parent = 0 , double size = 6) : QGraphicsPolygonItem(parent) {
		setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
		setSize(size);
	}
	
	// Size is irrelevant for points
	virtual void setSize(double /*size*/) {
		QPolygonF line;
		line << QPointF(0.1, 0);
		line << QPointF(-0.1, 0);
		setPolygon(line);
	}
};

// TODO: continue here...
class MPlotMarkerDiagDownLeft : public QGraphicsLineItem, public MPlotAbstractMarker {
public:
	MPlotMarkerDiagDownLeft(QGraphicsItem * parent = 0 , double size = 6) : QGraphicsLineItem(size/2, -size/2, -size/2, size/2, parent) {
		setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
	}
	
	virtual void setSize(double size) {
		setLine(size/2, -size/2, -size/2, size/2);
	}
};

class MPlotMarkerDiagDownRight : public QGraphicsLineItem, public MPlotAbstractMarker {
public:
	MPlotMarkerDiagDownRight(QGraphicsItem * parent = 0 , double size = 6) : QGraphicsLineItem(-size/2, -size/2, size/2, size/2, parent) {
		setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
	}
	
	virtual void setSize(double size) {
		setLine(-size/2, -size/2, size/2, size/2);
	}
};
////////////////////////

class MPlotMarkerCombined : public QAbstractGraphicsShapeItem, public MPlotAbstractMarker {
public:
	MPlotMarkerCombined(int shapeCode, QGraphicsItem* parent = 0, double size = 6) : QAbstractGraphicsShapeItem(parent) {
		setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
		
		if(shapeCode & MPlotMarker::Square)
			elements_ << new MPlotMarkerSquare(this, size);
		if(shapeCode & MPlotMarker::Circle)
			elements_ << new MPlotMarkerCircle(this, size);
		if(shapeCode & MPlotMarker::Triangle)
			elements_ << new MPlotMarkerTriangle(this, size);
		if(shapeCode & MPlotMarker::VerticalBeam)
			elements_ << new MPlotMarkerVerticalBeam(this, size);
		if(shapeCode & MPlotMarker::HorizontalBeam)
			elements_ << new MPlotMarkerHorizontalBeam(this, size);
		if(shapeCode & MPlotMarker::DiagDownLeft)
			elements_ << new MPlotMarkerDiagDownLeft(this, size);
		if(shapeCode & MPlotMarker::DiagDownRight)
			elements_ << new MPlotMarkerDiagDownRight(this, size);
		if(shapeCode & MPlotMarker::Point)
			elements_ << new MPlotMarkerPoint(this, size);
			
	}
	virtual ~MPlotMarkerCombined() {
		while(!elements_.isEmpty())
			delete elements_.takeFirst();
	}
	
	virtual void setSize(double size) {
		// problem... Again with the multiple inheritance. Can't call this on our MPlotAbstractMarkers
	}
	
protected:
	QList<QAbstractGraphicsShapeItem*> elements_;
	
};

class MPlotMarker {
	
public:
	enum Shape { Square = 1, Circle = 2, Triangle = 4, VerticalBeam = 8, HorizontalBeam = 16, DiagDownLeft = 32, DiagDownRight = 64, Point = 128, Cross, CrossSquare, CrossCircle, X, XSquare, XCircle, Star, StarSquare, StarCircle, PointSquare, PointCircle };
	
	static QAbstractGraphicsShapeItem* create(Shape type = Square, QGraphicsItem* parent = 0, double size = 6) {
		
		switch(type) {
			case Square: return new MPlotMarkerSquare(parent, size);
				break;
			case Circle: return new MPlotMarkerCircle(parent, size);
				break;
			case Triangle: return new MPlotMarkerTriangle(parent, size);
				break;
			case VerticalBeam: return new MPlotMarkerVerticalBeam(parent, size);
				break;
			case HorizontalBeam: return new MPlotMarkerHorizontalBeam(parent, size);
				break;
			case DiagDownLeft: return new MPlotMarkerDiagDownLeft(parent, size);
				break;
			case DiagDownRight: return new MPlotMarkerDiagDownRight(parent, size);
				break;
			case Cross: return new MPlotMarkerCombined(VerticalBeam | HorizontalBeam, parent, size);
				break;
			case CrossSquare: return new MPlotMarkerCombined(Square | VerticalBeam | HorizontalBeam, parent, size);
				break;
			case CrossCircle: return new MPlotMarkerCombined(Circle | VerticalBeam | HorizontalBeam, parent, size);
				break;
			case X: return new MPlotMarkerCombined(DiagDownLeft | DiagDownRight, parent, size);
				break;
			case XSquare: return new MPlotMarkerCombined(DiagDownLeft | DiagDownRight | Square, parent, size);
				break;
			case XCircle: return new MPlotMarkerCombined(DiagDownLeft | DiagDownRight | Circle, parent, size);
				break;
			case Star: return new MPlotMarkerCombined(DiagDownLeft | DiagDownRight | VerticalBeam | HorizontalBeam, parent, size);
				break;
			case StarCircle: return new MPlotMarkerCombined(DiagDownLeft | DiagDownRight | VerticalBeam | HorizontalBeam | Circle, parent, size);
				break;
			case StarSquare: return new MPlotMarkerCombined(DiagDownLeft | DiagDownRight | VerticalBeam | HorizontalBeam | Square, parent, size);
				break;
			case Point: return new MPlotMarkerPoint(parent, size);
				break;
			case PointSquare: return new MPlotMarkerCombined(Point | Square, parent, size);
				break;
			case PointCircle: return new MPlotMarkerCombined(Point | Circle, parent, size);
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
 
 setShape( MPlotMarker::Square );
 setSize( 6 );
 }
 
 public slots:
 
 virtual void setSize(double width);
 void setShape(Shape shapeType) {
 
 
 }
 
 protected:
 
 QGraphicsItem*
 
 /*
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
