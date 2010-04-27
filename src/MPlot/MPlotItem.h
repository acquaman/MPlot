#ifndef MPLOTITEM_H
#define MPLOTITEM_H

#include <QGraphicsObject>
#include "MPlotAxis.h"

/// This class defines the interface for all data-representation objects which can be added to an MPlot (ex: series/curves, images and spectrograms, contour maps, etc.)
class MPlotItem : public QGraphicsObject {
	Q_OBJECT
public:

	/// Constructor calls base class (QGraphicsObject)
	MPlotItem() : QGraphicsObject() {}

	/// returns which y-axis this data should be plotted against
	MPlotAxis::AxisID yAxisTarget() { return yAxisTarget_;}

	/// set the y-axis this data should be plotted against
	void setYAxisTarget(MPlotAxis::AxisID axis) { yAxisTarget_ = axis; }

	/// tell this item that it is 'selected' within the plot
	virtual void setSelected(bool selected = true) {
		bool updateNeeded = (selected != isSelected_);
		isSelected_ = selected;
		if(updateNeeded) {
			update();	// todo: maybe should move into subclasses; not all implementations will require update()?
			emit selectedChanged(isSelected_);
		}
	}
	/// ask if this item is currently selected on the plot
	virtual bool selected() { return isSelected_; }


	// Bounding rect: reported in our PlotItem coordinates, which are just the actual data coordinates. This is used by the graphics view system to figure out how much we cover/need to redraw.  Subclasses that draw selection borders or markers need to add their size on top of this.
	virtual QRectF boundingRect() const { return dataRect(); }

	// Data rect: also reported in our PlotItem coordinates, which are the actual data coordinates. This is used by the auto-scaling to figure out the range of our data on an axis.
	virtual QRectF dataRect() const = 0;

	// Paint: must be implemented in subclass.
	virtual void paint(QPainter* painter,
					   const QStyleOptionGraphicsItem* option,
					   QWidget* widget) = 0;

	// return the active shape where clicking willselect this object in the plot
	virtual QPainterPath shape() const {
		QPainterPath shape;
		shape.addRect(boundingRect());
		return shape;
	}


signals:

	void dataChanged(MPlotItem* item);	// listen to this if you want to auto-scale on changes.
	void selectedChanged(bool isSelected);


protected:
	bool isSelected_;
	MPlotAxis::AxisID yAxisTarget_;
};

#endif // MPLOTITEM_H
