#ifndef MPLOTITEM_H
#define MPLOTITEM_H

#include <QGraphicsItem>
#include "MPlotAxis.h"
#include "MPlotObservable.h"

/// This is the color of the selection highlight
#define MPLOT_SELECTION_COLOR QColor(255, 210, 129)
/// The opacity level (0=transparent, 1=opaque) of selection rectangles:
#define MPLOT_SELECTION_OPACITY 0.35
/// This is the width of selection highlight lines
#define MPLOT_SELECTION_LINEWIDTH 10

class MPlot;

/// This class defines the interface for all data-representation objects which can be added to an MPlot (ex: series/curves, images and spectrograms, contour maps, etc.)
class MPlotItem : public QGraphicsItem, public MPlotObservable {


public:

	/// Constructor calls base class (QGraphicsObject)
	MPlotItem();

	/// \todo Someday (when this becomes a full library, with .cpp files)... have the destructor remove this item from it's plot_, if there is an assocated plot_.  Also, what about being connected to multiple plots?
	~MPlotItem();


	/// returns which y-axis this data should be plotted against
	MPlotAxis::AxisID yAxisTarget();

	/// set the y-axis this data should be plotted against
	void setYAxisTarget(MPlotAxis::AxisID axis);

	/// tell this item that it is 'selected' within the plot
	virtual void setSelected(bool selected = true);
	/// ask if this item is currently selected on the plot
	virtual bool selected();

	/// use this if you don't want a plot item to be selectable:
	virtual bool selectable();
	virtual void setSelectable(bool selectable = true);


	/// Don't call this. Unfortunately public because it's required by MPlot::addItem and MPlot::removeItem.
	void setPlot(MPlot* plot);
	/// returns the plot we are attached to
	MPlot* plot() const;


	// Bounding rect: reported in our PlotItem coordinates, which are just the actual data coordinates. This is used by the graphics view system to figure out how much we cover/need to redraw.  Subclasses that draw selection borders or markers need to add their size on top of this.
	virtual QRectF boundingRect() const;

	// Data rect: also reported in our PlotItem coordinates, which are the actual data coordinates. This is used by the auto-scaling to figure out the range of our data on an axis.
	virtual QRectF dataRect() const = 0;

	/// Paint: must be implemented in subclass.
	virtual void paint(QPainter* painter,
					   const QStyleOptionGraphicsItem* option,
					   QWidget* widget) = 0;

	/// return the active shape where clicking will select this object in the plot. Subclasses can re-implement for more accuracy.
	virtual QPainterPath shape() const;


/// signals: Implements MPlotObservable.  Will Emit(0, "dataChanged") when x- or y- data changes, so the plot might need to be re-autoscaled.  Will Emit(1, "selectedChanged", 1) when the selection state of the item changes to true, and Emit(1, "selectedChanged", 0) when the selection state ofthe item changes to false.


private:
	bool isSelected_, isSelectable_;
	MPlotAxis::AxisID yAxisTarget_;

	MPlot* plot_;
};

#endif // MPLOTITEM_H
