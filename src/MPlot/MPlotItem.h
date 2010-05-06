#ifndef MPLOTITEM_H
#define MPLOTITEM_H

#include <QGraphicsObject>
#include "MPlotAxis.h"

/// This is the color of the selection highlight
#define MPLOT_SELECTION_COLOR QColor(255, 210, 129)
/// The opacity level (0=transparent, 1=opaque) of selection rectangles:
#define MPLOT_SELECTION_OPACITY 0.35
/// This is the width of selection highlight lines
#define MPLOT_SELECTION_LINEWIDTH 10

class MPlot;

/// This class defines the interface for all data-representation objects which can be added to an MPlot (ex: series/curves, images and spectrograms, contour maps, etc.)
class MPlotItem : public QGraphicsObject {
	Q_OBJECT

	Q_PROPERTY(bool selected READ selected WRITE setSelected NOTIFY selectedChanged);

public:

	/// Constructor calls base class (QGraphicsObject)
	MPlotItem() : QGraphicsObject() {
		setFlag(QGraphicsItem::ItemIsSelectable, false);	// We're implementing our own selection mechanism... ignoring QGraphicsView's selection system.
		isSelected_ = false;
	}

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

	/// use this if you don't want a plot item to be selectable:
	virtual bool selectable() { return isSelectable_; }
	virtual void setSelectable(bool selectable = true) { isSelectable_ = selectable; }


	/// Don't call this. Unfortunately public because it's required by MPlot::addItem and MPlot::removeItem.
	void setPlot(MPlot* plot) { plot_ = plot; }
	/// returns the plot we are attached to
	MPlot* plot() const { return plot_; }


	// Bounding rect: reported in our PlotItem coordinates, which are just the actual data coordinates. This is used by the graphics view system to figure out how much we cover/need to redraw.  Subclasses that draw selection borders or markers need to add their size on top of this.
	virtual QRectF boundingRect() const { return dataRect(); }

	// Data rect: also reported in our PlotItem coordinates, which are the actual data coordinates. This is used by the auto-scaling to figure out the range of our data on an axis.
	virtual QRectF dataRect() const = 0;

	/// Paint: must be implemented in subclass.
	virtual void paint(QPainter* painter,
					   const QStyleOptionGraphicsItem* option,
					   QWidget* widget) = 0;

	/// return the active shape where clicking will select this object in the plot. Subclasses can re-implement for more accuracy.
	virtual QPainterPath shape() const {
		QPainterPath shape;
		shape.addRect(boundingRect());
		return shape;
	}


signals:

	/// emitted if the x- or y- data changes, so that the plot might need to be re-auto-scaled.
	void dataChanged(MPlotItem* item);
	/// emitted when the selection state of the item changes
	void selectedChanged(bool isSelected);


private:
	bool isSelected_, isSelectable_;
	MPlotAxis::AxisID yAxisTarget_;

	MPlot* plot_;
};

#endif // MPLOTITEM_H
