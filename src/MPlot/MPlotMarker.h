#ifndef __MPlotMarker_H__
#define __MPlotMarker_H__

#include <QGraphicsItem>
#include <QAbstractGraphicsShapeItem>
#include <QGraphicsRectItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsPolygonItem>
#include <QGraphicsItemGroup>
#include <QList>
#include <QFont>

#include <math.h>

#define MPLOTMARKER_ZVALUE 5

namespace MPlotMarkerShape {
	enum Shape { Square = 1, Circle = 2, Triangle = 4, VerticalBeam = 8, HorizontalBeam = 16, DiagDownLeft = 32, DiagDownRight = 64, DiagDownLeftR = 128, DiagDownRightR = 256, Point = 512, Cross, CrossSquare, CrossCircle, X, XSquare, XCircle, Star, StarSquare, StarCircle, PointSquare, PointCircle };
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
	MPlotAbstractMarker(QGraphicsItem* parent = 0) : QGraphicsItem(parent) {
		setZValue(MPLOTMARKER_ZVALUE);	// Markers have a Z-value of 5.  Lines have a Z-value (default) of 0. This puts markers on top of lines.
	}
////////////////////

};



class MPlotMarkerSquare : public MPlotAbstractMarker {

public:
	MPlotMarkerSquare(QGraphicsItem * parent = 0 , double size = 6) : MPlotAbstractMarker(parent), marker_(QRectF(-size/2, -size/2, size, size), this) {
		setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
		setFlag(QGraphicsItem::ItemHasNoContents, true);// all painting done by children
	}
	
	virtual void setSize(double width) {
		marker_.setRect(-width/2, -width/2, width, width);
	}
	

	/////////////////////
	QPen pen() const { return marker_.pen(); }
    void setPen(const QPen &pen) { marker_.setPen(pen); }
	
    QBrush brush() const { return marker_.brush(); }
    void setBrush(const QBrush &brush) { marker_.setBrush(brush); }
	///////////////
	virtual QRectF boundingRect() const { return marker_.boundingRect(); }
	virtual void paint(QPainter * /*painter*/, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget = 0*/) {}
	virtual QPainterPath shape () const { return marker_.shape(); }
	virtual bool contains ( const QPointF & point ) const { return marker_.contains(point); }
	virtual bool isObscuredBy ( const QGraphicsItem * item ) const { return marker_.isObscuredBy(item); }
	virtual QPainterPath opaqueArea () const { return marker_.opaqueArea(); }
	///////////////
	
protected:
	QGraphicsRectItem marker_;
	
};

class MPlotMarkerCircle : public MPlotAbstractMarker {
	
public:	
	MPlotMarkerCircle(QGraphicsItem * parent = 0 , double size = 6) : MPlotAbstractMarker(parent), marker_(QRectF(-size/2, -size/2, size, size), this) {
		setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
		setFlag(QGraphicsItem::ItemHasNoContents, true);// all painting done by children
	}
	
	virtual void setSize(double width) {
		marker_.setRect(-width/2, -width/2, width, width);
	}
	
	/////////////////////
	QPen pen() const { return marker_.pen(); }
    void setPen(const QPen &pen) { marker_.setPen(pen); }
	
    QBrush brush() const { return marker_.brush(); }
    void setBrush(const QBrush &brush) { marker_.setBrush(brush); }
	///////////////
	virtual QRectF boundingRect() const { return marker_.boundingRect(); }
	virtual void paint(QPainter * /*painter*/, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget = 0*/) {}
	virtual QPainterPath shape () const { return marker_.shape(); }
	virtual bool contains ( const QPointF & point ) const { return marker_.contains(point); }
	virtual bool isObscuredBy ( const QGraphicsItem * item ) const { return marker_.isObscuredBy(item); }
	virtual QPainterPath opaqueArea () const { return marker_.opaqueArea(); }
	///////////////
	
protected:
	QGraphicsEllipseItem marker_;
};

class MPlotMarkerTriangle : public MPlotAbstractMarker {

	
public:	
	MPlotMarkerTriangle(QGraphicsItem * parent = 0 , double size = 6 ) : MPlotAbstractMarker(parent), marker_(this) {
		setFlag(QGraphicsItem::ItemIgnoresTransformations, true); 
		setFlag(QGraphicsItem::ItemHasNoContents, true);// all painting done by children
		setSize(size);
	}
	
	virtual void setSize(double width) {
		
		triangle_ << QPointF(-width/2, width/2/sqrt(3));
		triangle_ << QPointF(width/2, width/2/sqrt(3));
		triangle_ << QPointF(0, -width/sqrt(3));
		triangle_ << QPointF(-width/2, width/2/sqrt(3));
		
		marker_.setPolygon(triangle_);
	}	
	/////////////////////
	QPen pen() const { return marker_.pen(); }
    void setPen(const QPen &pen) { marker_.setPen(pen); }
	
    QBrush brush() const { return marker_.brush(); }
    void setBrush(const QBrush &brush) { marker_.setBrush(brush); }
	///////////////
	virtual QRectF boundingRect() const { return marker_.boundingRect(); }
	virtual void paint(QPainter * /*painter*/, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget = 0*/) {}
	virtual QPainterPath shape () const { return marker_.shape(); }
	virtual bool contains ( const QPointF & point ) const { return marker_.contains(point); }
	virtual bool isObscuredBy ( const QGraphicsItem * item ) const { return marker_.isObscuredBy(item); }
	virtual QPainterPath opaqueArea () const { return marker_.opaqueArea(); }
	///////////////
	
protected:	
	QPolygonF triangle_;
	QGraphicsPolygonItem marker_;
};

class MPlotMarkerVerticalBeam : public MPlotAbstractMarker {
public:
	MPlotMarkerVerticalBeam(QGraphicsItem * parent = 0 , double size = 6) : MPlotAbstractMarker(parent), marker_(0, size/2, 0, -size/2, this) {
		setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
		setFlag(QGraphicsItem::ItemHasNoContents, true);// all painting done by children
	}
	
	virtual void setSize(double size) {
		marker_.setLine(0, size/2, 0, -size/2);
	}
	
	/////////////////////
	QPen pen() const { return marker_.pen(); }
    void setPen(const QPen &pen) { marker_.setPen(pen); }
	
    QBrush brush() const { return QBrush(); }
    void setBrush(const QBrush & /*brush*/) {}
	///////////////
	virtual QRectF boundingRect() const { return marker_.boundingRect(); }
	virtual void paint(QPainter * /*painter*/, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget = 0*/) {}
	virtual QPainterPath shape () const { return marker_.shape(); }
	virtual bool contains ( const QPointF & point ) const { return marker_.contains(point); }
	virtual bool isObscuredBy ( const QGraphicsItem * item ) const { return marker_.isObscuredBy(item); }
	virtual QPainterPath opaqueArea () const { return marker_.opaqueArea(); }
	///////////////
	
protected:	
	QGraphicsLineItem marker_;
};

// More simple lines: inherit VerticalBeam
class MPlotMarkerHorizontalBeam : public MPlotMarkerVerticalBeam {
public:
	MPlotMarkerHorizontalBeam(QGraphicsItem * parent = 0 , double size = 6) : MPlotMarkerVerticalBeam(parent, size) {
		setSize(size);
	}
	
	virtual void setSize(double size) {
		marker_.setLine(size/2, 0, -size/2, 0);
	}
};

class MPlotMarkerPoint : public MPlotMarkerVerticalBeam {
public:
	MPlotMarkerPoint(QGraphicsItem * parent = 0 , double size = 6) : MPlotMarkerVerticalBeam(parent, size) {
		setSize(size);
	}
	
	virtual void setSize(double /*size*/) {
		marker_.setLine(0.1, 0, -0.1, 0);
	}
};


class MPlotMarkerDiagDownLeft : public MPlotMarkerVerticalBeam {
public:
	MPlotMarkerDiagDownLeft(QGraphicsItem * parent = 0 , double size = 6) : MPlotMarkerVerticalBeam(parent, size) {
		setSize(size);
	}
	
	virtual void setSize(double size) {
		marker_.setLine(-size/2, size/2, size/2, -size/2);
	}
};

// A shorter version... This one has a length of size instead of size*sqrt(2). (Matches dia. of a circle of 'size')
class MPlotMarkerDiagDownLeftR : public MPlotMarkerVerticalBeam {
public:
	MPlotMarkerDiagDownLeftR(QGraphicsItem * parent = 0 , double size = 6) : MPlotMarkerVerticalBeam(parent, size) {
		setSize(size);
	}
	
	virtual void setSize(double size) {
		double lo2 = size/2/sqrt(2);
		marker_.setLine(-lo2, lo2, lo2, -lo2);
	}
};

class MPlotMarkerDiagDownRight : public MPlotMarkerVerticalBeam {
public:
	MPlotMarkerDiagDownRight(QGraphicsItem * parent = 0 , double size = 6) : MPlotMarkerVerticalBeam(parent, size) {
		setSize(size);
	}
	
	virtual void setSize(double size) {
		marker_.setLine(-size/2, -size/2, size/2, size/2);
	}
};

class MPlotMarkerDiagDownRightR : public MPlotMarkerVerticalBeam {
public:
	MPlotMarkerDiagDownRightR(QGraphicsItem * parent = 0 , double size = 6) : MPlotMarkerVerticalBeam(parent, size) {
		setSize(size);
	}
	
	virtual void setSize(double size) {
		double lo2 = size/2/sqrt(2);
		marker_.setLine(-lo2, -lo2, lo2, lo2);
	}
};

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
    void setBrush(const QBrush & /*brush*/) {}
	///////////////
	virtual QRectF boundingRect() const { return marker_.boundingRect(); }
	virtual void paint(QPainter * /*painter*/, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget = 0*/) {}
	virtual QPainterPath shape () const { return marker_.shape(); }
	virtual bool contains ( const QPointF & point ) const { return marker_.contains(point); }
	virtual bool isObscuredBy ( const QGraphicsItem * item ) const { return marker_.isObscuredBy(item); }
	virtual QPainterPath opaqueArea () const { return marker_.opaqueArea(); }
	///////////////
	
protected:	
	QGraphicsTextItem marker_;
	QFont font_;
	
};


class MPlotMarkerCombined : public MPlotAbstractMarker {
public:
	MPlotMarkerCombined(int shapeCode, QGraphicsItem* parent = 0, double size = 6) : MPlotAbstractMarker(parent) {
		setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
		setFlag(QGraphicsItem::ItemHasNoContents, true);// all painting done by children
		

		if(shapeCode & MPlotMarkerShape::Square) {
			elements_ << new MPlotMarkerSquare(this, size);
		}
		if(shapeCode & MPlotMarkerShape::Circle) {
			elements_ << new MPlotMarkerCircle(this, size);
		}
		if(shapeCode & MPlotMarkerShape::Triangle) {
			elements_ << new MPlotMarkerTriangle(this, size);
		}
		if(shapeCode & MPlotMarkerShape::VerticalBeam) {
			elements_ << new MPlotMarkerVerticalBeam(this, size);
		}
		if(shapeCode & MPlotMarkerShape::HorizontalBeam) {
			elements_ << new MPlotMarkerHorizontalBeam(this, size);
		}
		if(shapeCode & MPlotMarkerShape::DiagDownLeft) {
			elements_ << new MPlotMarkerDiagDownLeft(this, size);
		}
		if(shapeCode & MPlotMarkerShape::DiagDownRight) {
			elements_ << new MPlotMarkerDiagDownRight(this, size);
		}
		if(shapeCode & MPlotMarkerShape::DiagDownLeftR) {
			elements_ << new MPlotMarkerDiagDownLeftR(this, size);
		}
		if(shapeCode & MPlotMarkerShape::DiagDownRightR) {
			elements_ << new MPlotMarkerDiagDownRightR(this, size);
		}
		if(shapeCode & MPlotMarkerShape::Point) {
			elements_ << new MPlotMarkerPoint(this, size);
		}
		
		this->setPen(pen_);
		this->setBrush(brush_);
			
	}
	
	virtual ~MPlotMarkerCombined() {
		while(!elements_.isEmpty())
			delete elements_.takeFirst();
	}
	
	virtual void setSize(double size) {
		foreach(MPlotAbstractMarker* element, elements_) {
			element->setSize(size);
		}
	}
	
	QPen pen() const { return pen_; }
    void setPen(const QPen &pen) { 
		pen_ = pen;
		foreach(MPlotAbstractMarker* element, elements_) {
			element->setPen(pen);
		}
	}
	
	QBrush brush() const { return brush_; }
    void setBrush(const QBrush &brush) { 
		brush_ = brush;
		foreach(MPlotAbstractMarker* element, elements_) {
			element->setBrush(brush);
		}
	}
	
	/////////////////
	virtual QRectF boundingRect() const { return childrenBoundingRect(); }
	virtual void paint(QPainter * /*painter*/, const QStyleOptionGraphicsItem * /*option*/, QWidget */*widget = 0*/) {} /*QGraphicsItemGroup::paint(painter, option, widget);*/
	///////////////
	
	
protected:
	QList<MPlotAbstractMarker*> elements_;
	QPen pen_;
	QBrush brush_;
	
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
			case MPlotMarkerShape::XCircle: return new MPlotMarkerCombined(MPlotMarkerShape::DiagDownLeftR | MPlotMarkerShape::DiagDownRightR | MPlotMarkerShape::Circle, parent, size);
				break;
			case MPlotMarkerShape::Star: return new MPlotMarkerCombined(MPlotMarkerShape::DiagDownLeftR | MPlotMarkerShape::DiagDownRightR | MPlotMarkerShape::VerticalBeam | MPlotMarkerShape::HorizontalBeam, parent, size);
				break;
			case MPlotMarkerShape::StarCircle: return new MPlotMarkerCombined(MPlotMarkerShape::DiagDownLeftR | MPlotMarkerShape::DiagDownRightR | MPlotMarkerShape::VerticalBeam | MPlotMarkerShape::HorizontalBeam | MPlotMarkerShape::Circle, parent, size);
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
