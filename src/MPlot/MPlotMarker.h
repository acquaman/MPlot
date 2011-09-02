#ifndef __MPlotMarker_H__
#define __MPlotMarker_H__


#include <QPainter>
#include <QPolygonF>
#include <QLineF>
#include <QList>


#include <math.h>

/// Marker shape namespace.  Contains all the basic elements which can also be combined with bitwise OR.
namespace MPlotMarkerShape {
	enum Shape { None = 0, Square = 1, Circle = 2, Triangle = 4, VerticalBeam = 8, HorizontalBeam = 16, DiagDownLeft = 32, DiagDownRight = 64, DiagDownLeftR = 128, DiagDownRightR = 256, Point = 512, Cross, CrossSquare, CrossCircle, X, XSquare, XCircle, Star, StarSquare, StarCircle, PointSquare, PointCircle };
};


// We're using separate classes for each marker type instead of one class with a MarkerShape parameter. This is for performance reasons: a call to the virtual paint() event is faster than
// would be a switch() over the different marker types (inside a common paint() event.)    The paint() is called for every data point, so for large datasets it must be fast.

// Abstract class: all plot markers must have these:
class MPlotAbstractMarker {

public:
        /// Constructor.  Builds a default marker.
        /*!
          \param size holds the relative size of the marker.
          \param pen holds the pen to draw the marker.
          \param brush holds the brush used to paint the marker.
          */
	MPlotAbstractMarker(qreal size = 6, const QPen& pen = QPen(), const QBrush& brush = QBrush());
        /// Destructor.
        virtual ~MPlotAbstractMarker();

        /// Sets the size for the marker.
	virtual void setSize(qreal size) { size_ = size; }
        /// Returns the size of the marker.
	virtual qreal size() const { return size_; }

        /// Returns the pen used to draw the marker.
	virtual QPen pen() const { return pen_; }
        /// Sets the pen to draw the marker.
	virtual void setPen(const QPen &pen1) { pen_ = pen1; }

        /// Returns the brush used to paint the marker.
	virtual QBrush brush() const { return brush_; }
        /// Sets the brush to paint the marker.
	virtual void setBrush(const QBrush &brush) { brush_ = brush; }

        /// Paint function. Pure virtual because subclasses must define what they look like.
	virtual void paint(QPainter* painter) = 0;
	////////////////////

protected:
        /// Member holding the size of the marker in drawing coordinates.
        qreal size_;
        /// Member holding the pen for the marker.
	QPen pen_;
        /// Mebmer holding the brush for the marker.
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
        /// Constructor.  Builds a square plot marker.
	MPlotMarkerSquare(qreal size = 6, const QPen& pen = QPen(), const QBrush& brush = QBrush());
        /// Paints a square marker based on the current size, pen, and brush.
	virtual void paint(QPainter* painter);
};

class MPlotMarkerCircle : public MPlotAbstractMarker {

public:
        /// Constructor.  Builds a circle plot marker.
	MPlotMarkerCircle(qreal size = 6, const QPen& pen = QPen(), const QBrush& brush = QBrush());
        /// Paints a circle marker based on the current size, pen, and brush.
	virtual void paint(QPainter* painter);
};

class MPlotMarkerTriangle : public MPlotAbstractMarker {


public:
        /// Constructor.  Builds a triangle plot marker.
	MPlotMarkerTriangle(qreal size = 6, const QPen& pen = QPen(), const QBrush& brush = QBrush());

        /// Set the size.  Overloaded from abstract class because the triangle is slightly different.
	virtual void setSize(qreal width);
        /// Paints a triangle marker based on the current size, pen, and brush.
	virtual void paint(QPainter* painter);

protected:
        /// Member holding the geometry of the triangle.
	QPolygonF triangle_;
};

class MPlotMarkerVerticalBeam : public MPlotAbstractMarker {

public:
        /// Constructor.  Builds a vertical beam marker.
	MPlotMarkerVerticalBeam(qreal size = 6, const QPen& pen = QPen(), const QBrush& brush = QBrush());
        /// Set the size.  Overloaded from the abstract class because the geometry requirements are different.
	virtual void setSize(qreal width);
        /// Paints a vertical beam marker based on the current size, pen, and brush.
	virtual void paint(QPainter* painter);

protected:
        /// Member holding the line properties for the beam.
	QLineF line_;
};


class MPlotMarkerHorizontalBeam : public MPlotAbstractMarker {

public:
        /// Constructor.  Builds a horizontal beam marker.
	MPlotMarkerHorizontalBeam(qreal size = 6, const QPen& pen = QPen(), const QBrush& brush = QBrush());
        /// Sets the size.  Overloaded from the abstract class because the geometry requirements are different.
	virtual void setSize(qreal width);
        /// Paints a horizontal beam marker based on the current size, pen, and brush.
	virtual void paint(QPainter* painter);

protected:
        /// Member holding the line properties of the beam.
	QLineF line_;
};

class MPlotMarkerPoint : public MPlotAbstractMarker {

public:
        /// Constructor.  Builds a point marker.
	MPlotMarkerPoint(qreal size = 6, const QPen& pen = QPen(), const QBrush& brush = QBrush());
        /// Sets the size.  Overloaded from the abstract class because there are no geometry requirements.
	virtual void setSize(qreal);
        /// Paints a point marker based on the current size, pen, and brush.
	virtual void paint(QPainter* painter);

protected:
        /// Member holding the properties of the line. The line is somewhat arbitrary because the painted marker is just a point.
	QLineF line_;
};

class MPlotMarkerDiagDownLeft : public MPlotAbstractMarker {

public:
        /// Constructor.  Builds a downward left diagonal.  Think \ as a marker.
	MPlotMarkerDiagDownLeft(qreal size = 6, const QPen& pen = QPen(), const QBrush& brush = QBrush());
        /// Sets the size.  Overloaded from the abstract class because the shape needs to be updated.
	virtual void setSize(qreal size);
        /// Paints a downward left diagonal marker based on the current size, pen, and brush.
	virtual void paint(QPainter* painter);

protected:
        /// Member holding the geometry of the marker.
	QLineF line_;
};

// A shorter version... This one has a length of size instead of size*sqrt(2). (Matches dia. of a circle of 'size')
class MPlotMarkerDiagDownLeftR : public MPlotAbstractMarker {

public:
        /// Constructor.  Builds a downward left diagonal with length of size rather than size*sqrt(2).  Think \ as a marker.
	MPlotMarkerDiagDownLeftR(qreal size = 6, const QPen& pen = QPen(), const QBrush& brush = QBrush());
	virtual void setSize(qreal size);
	virtual void paint(QPainter* painter);

protected:
        /// Member holding the geometry of the marker.
	QLineF line_;
};

class MPlotMarkerDiagDownRight : public MPlotAbstractMarker {

public:
        /// Constructor.  Builds a downward right diagonal.  Think / as a marker.
	MPlotMarkerDiagDownRight(qreal size = 6, const QPen& pen = QPen(), const QBrush& brush = QBrush());
        /// Sets the size.  Overloaded from the abstract class because the shape needs to be updated.
	virtual void setSize(qreal size);
        /// Paints a downward right marker based on the current size, pen, and brush.
	virtual void paint(QPainter* painter);

protected:
        /// Member holding the geometery of the marker.
	QLineF line_;
};

class MPlotMarkerDiagDownRightR : public MPlotAbstractMarker {

public:
        /// Constructor.  Builds a downward right diagonal with a length of size and not size*sqrt(2).  Think / as a marker.
	MPlotMarkerDiagDownRightR(qreal size = 6, const QPen& pen = QPen(), const QBrush& brush = QBrush());
        /// Sets the size.  Overloaded from the abstract class because the shape needs to be updated.
	virtual void setSize(qreal size);
        /// Paints a downward right marker based on the current size, pen, and brush.
	virtual void paint(QPainter* painter);

protected:
        /// Member holding the geometry of the marker.
	QLineF line_;
};

class MPlotMarkerCombined : public MPlotAbstractMarker {
public:
        /// Constructor.  Builds a marker based on a bitwise OR of the different MPlotMarkerShapes.
	MPlotMarkerCombined(int shapeCode, qreal size = 6, const QPen& pen = QPen(), const QBrush& brush = QBrush());
        /// Destructor.
	virtual ~MPlotMarkerCombined();
        /// Sets the size.  Overloaded from the abstract class because it needs to update the size for every marker that makes up the combined marker.
	virtual void setSize(qreal size);
        /// Sets the pen.  Overloaded from the abstract class because it needs to update the size for every marker that makes up the combined marker.
	virtual void setPen(const QPen &pen);
        /// Sets the brush.  Overloaded from the abstract class because it needs to update the size for everyt marker that makes up the combined marker.
	virtual void setBrush(const QBrush &brush);
        /// Paints a combined marker based on the current size, pen, and brush.
	virtual void paint(QPainter* painter);

protected:
        /// Member holding the list of all the markers used to make the combined marker.
	QList<MPlotAbstractMarker*> elements_;
};

class MPlotMarker {

public:

        /// Static creator function.  Builds a marker based on the \param type, \param size, \param pen, and \param brush and returns an AMAbstractMarker.
	static MPlotAbstractMarker* create(MPlotMarkerShape::Shape type = MPlotMarkerShape::Square, qreal size = 6, const QPen& pen = QPen(), const QBrush& brush = QBrush());
};

#endif
