#ifndef MPLOTPOINT_H
#define MPLOTPOINT_H

#include "MPlotMarker.h"
#include "MPlotAxis.h"
#include "MPlotItem.h"

/// This Plot Item is useful for displaying a single data point on a plot.
class MPlotPoint : public MPlotItem {

public:

	/// Default constructor. Override setDefaults() for a custom look off the bat.
	MPlotPoint(const QPointF& value = QPointF(0,0));

	virtual ~MPlotPoint();

	virtual int rank() const {
		 return 0;
	}

	// Returns the current marker, which can be used to access it's pen, brush, and size.
		// If the plot has no marker (or MPlotMarkerShape::None), then this will be a null pointer. Must check before setting.
	virtual MPlotAbstractMarker* marker() const { return marker_; }

	virtual void setMarker(MPlotMarkerShape::Shape shape, qreal size = 6, const QPen& pen = QPen(QColor(Qt::red)), const QBrush& brush = QBrush());

	/// returns the point location of this marker:
	QPointF value() const { return point_; }


	/// set the point location of this marker:
	void setValue(const QPointF& point);




	// Required functions:
	//////////////////////////
	// Bounding rect: reported in our PlotSeries coordinates, which are just the actual data coordinates. This is used by the graphics view system to figure out how much we cover/need to redraw.  Subclasses that draw selection borders or markers need to add their size on top of this.
	virtual QRectF boundingRect() const;

	/// Data rect: reported in actual data coordinates. This is used by the auto-scaling to figure out the range of our data on an axis.
	virtual QRectF dataRect() const;

	/// Paint: implemented from MPlotItem to draw a single marker
	virtual void paint(QPainter* painter,
					   const QStyleOptionGraphicsItem* option,
					   QWidget* widget);


	/// Re-implemented from MPlotItem to provide our line color as the legend color:
	virtual QBrush legendColor() const { if (marker_) return QBrush(marker_->pen().color()); else return QBrush(QColor(127, 127, 127)); }

protected:
	QPen selectedPen_;
	MPlotAbstractMarker* marker_;

	QPointF point_;

	virtual void setDefaults();


};


#endif // MPLOTPOINT_H
