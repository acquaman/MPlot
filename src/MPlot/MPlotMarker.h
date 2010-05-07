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
	MPlotAbstractMarker(double size = 6, const QPen& pen = QPen(), const QBrush& brush = QBrush()) : size_(size), pen_(pen), brush_(brush) {}
	
	virtual void setSize(double size) { size_ = size; }
	virtual double size() { return size_; }
	
	virtual QPen pen() const { return pen_; }
    virtual void setPen(const QPen &pen1) { pen_ = pen1; pen_.setCosmetic(true); }
	
    virtual QBrush brush() const { return brush_; }
    virtual void setBrush(const QBrush &brush) { brush_ = brush; }

	
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
	MPlotMarkerSquare(double size = 6, const QPen& pen = QPen(), const QBrush& brush = QBrush()) : MPlotAbstractMarker(size, pen, brush) {}
	
	virtual void paint(QPainter* painter) {
		QRectF me(-size_/2, -size_/2, size_, size_);
		painter->drawRect(me);
	}	
};

class MPlotMarkerCircle : public MPlotAbstractMarker {
	
public:	
	MPlotMarkerCircle(double size = 6, const QPen& pen = QPen(), const QBrush& brush = QBrush()) : MPlotAbstractMarker(size, pen, brush) {}
	virtual void paint(QPainter* painter) {
		painter->drawEllipse(-size_/2, -size_/2, size_, size_);
	}	
};

class MPlotMarkerTriangle : public MPlotAbstractMarker {

	
public:	
	MPlotMarkerTriangle(double size = 6, const QPen& pen = QPen(), const QBrush& brush = QBrush()) : MPlotAbstractMarker(size, pen, brush) {
		setSize(size);
	}
	
	virtual void setSize(double width) {
		size_ = width;
		triangle_ << QPointF(-width/2, width/2/sqrt(3));
		triangle_ << QPointF(width/2, width/2/sqrt(3));
		triangle_ << QPointF(0, -width/sqrt(3));
		triangle_ << QPointF(-width/2, width/2/sqrt(3));
	}	
	
	virtual void paint(QPainter* painter) {
		painter->drawPolygon(triangle_);
	}	
	
protected:	
	QPolygonF triangle_;
};

class MPlotMarkerVerticalBeam : public MPlotAbstractMarker {
public:
	MPlotMarkerVerticalBeam(double size = 6, const QPen& pen = QPen(), const QBrush& brush = QBrush()) : MPlotAbstractMarker(size, pen, brush) {
		setSize(size);
	}
	
	virtual void setSize(double width) {
		size_ = width;
		line_.setLine(0, size_/2, 0, -size_/2);
	}	
	
	virtual void paint(QPainter* painter) {
		painter->drawLine(line_);
	}	
	
protected:	
	QLineF line_;
};


class MPlotMarkerHorizontalBeam : public MPlotAbstractMarker {
public:
	MPlotMarkerHorizontalBeam(double size = 6, const QPen& pen = QPen(), const QBrush& brush = QBrush()) : MPlotAbstractMarker(size, pen, brush) {
		setSize(size);
	}
	
	virtual void setSize(double width) {
		size_ = width;
		line_.setLine(size_/2, 0, -size_/2, 0);
	}	
	
	virtual void paint(QPainter* painter) {
		painter->drawLine(line_);
	}	
	
protected:	
	QLineF line_;
};

class MPlotMarkerPoint : public MPlotAbstractMarker {
public:
	MPlotMarkerPoint(double size = 6, const QPen& pen = QPen(), const QBrush& brush = QBrush()) : MPlotAbstractMarker(size, pen, brush) {
		setSize(size);
	}
	
	virtual void setSize(double) {
		size_ = 0.1;
		line_.setLine(size_/2, 0, -size_/2, 0);
	}	
	
	virtual void paint(QPainter* painter) {
		painter->drawLine(line_);
	}	
	
protected:	
	QLineF line_;
};


class MPlotMarkerDiagDownLeft : public MPlotAbstractMarker {
public:
	MPlotMarkerDiagDownLeft(double size = 6, const QPen& pen = QPen(), const QBrush& brush = QBrush()) : MPlotAbstractMarker(size, pen, brush) {
		setSize(size);
	}
	
	virtual void setSize(double size) {
		size_ = size;
		line_.setLine(-size/2, size/2, size/2, -size/2);
	}	
	
	virtual void paint(QPainter* painter) {
		painter->drawLine(line_);
	}	
	
protected:	
	QLineF line_;
};


// A shorter version... This one has a length of size instead of size*sqrt(2). (Matches dia. of a circle of 'size')
class MPlotMarkerDiagDownLeftR : public MPlotAbstractMarker {
public:
	MPlotMarkerDiagDownLeftR(double size = 6, const QPen& pen = QPen(), const QBrush& brush = QBrush()) : MPlotAbstractMarker(size, pen, brush) {
		setSize(size);
	}
	
	virtual void setSize(double size) {
		size_ = size;
		double lo2 = size_/2/sqrt(2);
		line_.setLine(-lo2, lo2, lo2, -lo2);
	}
	
	virtual void paint(QPainter* painter) {
		painter->drawLine(line_);
	}	
	
protected:	
	QLineF line_;
};


class MPlotMarkerDiagDownRight : public MPlotAbstractMarker {
public:
	MPlotMarkerDiagDownRight(double size = 6, const QPen& pen = QPen(), const QBrush& brush = QBrush()) : MPlotAbstractMarker(size, pen, brush) {
		setSize(size);
	}
	
	virtual void setSize(double size) {
		size_ = size;
		line_.setLine(-size/2, -size/2, size/2, size/2);
	}
	
	virtual void paint(QPainter* painter) {
		painter->drawLine(line_);
	}	
	
protected:	
	QLineF line_;
};


class MPlotMarkerDiagDownRightR : public MPlotAbstractMarker {
public:
	MPlotMarkerDiagDownRightR(double size = 6, const QPen& pen = QPen(), const QBrush& brush = QBrush()) : MPlotAbstractMarker(size, pen, brush) {
		setSize(size);
	}
	
	virtual void setSize(double size) {
		size_ = size;
		double lo2 = size_/2/sqrt(2);
		line_.setLine(-lo2, -lo2, lo2, lo2);
	}
	
	virtual void paint(QPainter* painter) {
		painter->drawLine(line_);
	}	
	
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
	MPlotMarkerCombined(int shapeCode, double size = 6, const QPen& pen = QPen(), const QBrush& brush = QBrush()) : MPlotAbstractMarker(size, pen, brush) {

		if(shapeCode & MPlotMarkerShape::Square) {
			elements_ << new MPlotMarkerSquare(size, pen, brush);
		}
		if(shapeCode & MPlotMarkerShape::Circle) {
			elements_ << new MPlotMarkerCircle(size, pen, brush);
		}
		if(shapeCode & MPlotMarkerShape::Triangle) {
			elements_ << new MPlotMarkerTriangle(size, pen, brush);
		}
		if(shapeCode & MPlotMarkerShape::VerticalBeam) {
			elements_ << new MPlotMarkerVerticalBeam(size, pen, brush);
		}
		if(shapeCode & MPlotMarkerShape::HorizontalBeam) {
			elements_ << new MPlotMarkerHorizontalBeam(size, pen, brush);
		}
		if(shapeCode & MPlotMarkerShape::DiagDownLeft) {
			elements_ << new MPlotMarkerDiagDownLeft(size, pen, brush);
		}
		if(shapeCode & MPlotMarkerShape::DiagDownRight) {
			elements_ << new MPlotMarkerDiagDownRight(size, pen, brush);
		}
		if(shapeCode & MPlotMarkerShape::DiagDownLeftR) {
			elements_ << new MPlotMarkerDiagDownLeftR(size, pen, brush);
		}
		if(shapeCode & MPlotMarkerShape::DiagDownRightR) {
			elements_ << new MPlotMarkerDiagDownRightR(size, pen, brush);
		}
		if(shapeCode & MPlotMarkerShape::Point) {
			elements_ << new MPlotMarkerPoint(size, pen, brush);
		}

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
	
    virtual void setPen(const QPen &pen) { 
		pen_ = pen;
		foreach(MPlotAbstractMarker* element, elements_) {
			element->setPen(pen);
		}
	}
	
    virtual void setBrush(const QBrush &brush) { 
		brush_ = brush;
		foreach(MPlotAbstractMarker* element, elements_) {
			element->setBrush(brush);
		}
	}

	
	
	virtual void paint(QPainter* painter) {
		foreach(MPlotAbstractMarker* element, elements_) {
			element->paint(painter);
		}
	}
	
protected:
	QList<MPlotAbstractMarker*> elements_;
	
};

class MPlotMarker {
	
public:
	
	// Static creator function:
	static MPlotAbstractMarker* create(MPlotMarkerShape::Shape type = MPlotMarkerShape::Square, double size = 6, const QPen& pen = QPen(), const QBrush& brush = QBrush()) {
		
		switch(type) {
		case MPlotMarkerShape::None: return 0;
			break;
		case MPlotMarkerShape::Square: return new MPlotMarkerSquare(size, pen, brush);
			break;
		case MPlotMarkerShape::Circle: return new MPlotMarkerCircle(size, pen, brush);
			break;
		case MPlotMarkerShape::Triangle: return new MPlotMarkerTriangle(size, pen, brush);
			break;
		case MPlotMarkerShape::VerticalBeam: return new MPlotMarkerVerticalBeam(size, pen, brush);
			break;
		case MPlotMarkerShape::HorizontalBeam: return new MPlotMarkerHorizontalBeam(size, pen, brush);
			break;
		case MPlotMarkerShape::DiagDownLeft: return new MPlotMarkerDiagDownLeft(size, pen, brush);
			break;
		case MPlotMarkerShape::DiagDownRight: return new MPlotMarkerDiagDownRight(size, pen, brush);
			break;
		case MPlotMarkerShape::DiagDownLeftR: return new MPlotMarkerDiagDownLeftR(size, pen, brush);
			break;
		case MPlotMarkerShape::DiagDownRightR: return new MPlotMarkerDiagDownRightR(size, pen, brush);
			break;
		case MPlotMarkerShape::Cross: return new MPlotMarkerCombined(MPlotMarkerShape::VerticalBeam | MPlotMarkerShape::HorizontalBeam, size, pen, brush);
			break;
		case MPlotMarkerShape::CrossSquare: return new MPlotMarkerCombined(MPlotMarkerShape::Square | MPlotMarkerShape::VerticalBeam | MPlotMarkerShape::HorizontalBeam, size, pen, brush);
			break;
		case MPlotMarkerShape::CrossCircle: return new MPlotMarkerCombined(MPlotMarkerShape::Circle | MPlotMarkerShape::VerticalBeam | MPlotMarkerShape::HorizontalBeam, size, pen, brush);
			break;
		case MPlotMarkerShape::X: return new MPlotMarkerCombined(MPlotMarkerShape::DiagDownLeft | MPlotMarkerShape::DiagDownRight, size, pen, brush);
			break;
		case MPlotMarkerShape::XSquare: return new MPlotMarkerCombined(MPlotMarkerShape::DiagDownLeft | MPlotMarkerShape::DiagDownRight | MPlotMarkerShape::Square, size, pen, brush);
			break;
		case MPlotMarkerShape::XCircle: return new MPlotMarkerCombined(MPlotMarkerShape::DiagDownLeftR | MPlotMarkerShape::DiagDownRightR | MPlotMarkerShape::Circle, size, pen, brush);
			break;
		case MPlotMarkerShape::Star: return new MPlotMarkerCombined(MPlotMarkerShape::DiagDownLeftR | MPlotMarkerShape::DiagDownRightR | MPlotMarkerShape::VerticalBeam | MPlotMarkerShape::HorizontalBeam, size, pen, brush);
			break;
		case MPlotMarkerShape::StarCircle: return new MPlotMarkerCombined(MPlotMarkerShape::DiagDownLeftR | MPlotMarkerShape::DiagDownRightR | MPlotMarkerShape::VerticalBeam | MPlotMarkerShape::HorizontalBeam | MPlotMarkerShape::Circle, size, pen, brush);
			break;
		case MPlotMarkerShape::StarSquare: return new MPlotMarkerCombined(MPlotMarkerShape::DiagDownLeft | MPlotMarkerShape::DiagDownRight | MPlotMarkerShape::VerticalBeam | MPlotMarkerShape::HorizontalBeam | MPlotMarkerShape::Square, size, pen, brush);
			break;
		case MPlotMarkerShape::Point: return new MPlotMarkerPoint(size, pen, brush);
			break;
		case MPlotMarkerShape::PointSquare: return new MPlotMarkerCombined(MPlotMarkerShape::Point | MPlotMarkerShape::Square, size, pen, brush);
			break;
		case MPlotMarkerShape::PointCircle: return new MPlotMarkerCombined(MPlotMarkerShape::Point | MPlotMarkerShape::Circle, size, pen, brush);
			break;

		default: return new MPlotMarkerSquare(size, pen, brush);

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
