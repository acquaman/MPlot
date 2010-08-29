#ifndef MPLOTITEM_CPP
#define MPLOTITEM_CPP

#include "MPlotItem.h"
#include "MPlot.h"

MPlotItemSignalSource::MPlotItemSignalSource(MPlotItem* parent )
	: QObject(0) {
	plotItem_  = parent;
}

/// Constructor calls base class (QGraphicsObject)
MPlotItem::MPlotItem() : QGraphicsItem() {
	setFlag(QGraphicsItem::ItemIsSelectable, false);	// We're implementing our own selection mechanism... ignoring QGraphicsView's selection system.
	isSelected_ = false;
	isSelectable_ = true;
	plot_ = 0;
	signalSource_ = new MPlotItemSignalSource(this);
}

/// \todo Someday (when this becomes a full library, with .cpp files)... have the destructor remove this item from it's plot_, if there is an assocated plot_.  Also, what about being connected to multiple plots?
MPlotItem::~MPlotItem() {
	if(plot())
		plot()->removeItem(this);
	delete signalSource_;
}


/// returns which y-axis this data should be plotted against
MPlotAxis::AxisID MPlotItem::yAxisTarget() {
	return yAxisTarget_;
}

/// set the y-axis this data should be plotted against
void MPlotItem::setYAxisTarget(MPlotAxis::AxisID axis) {
	yAxisTarget_ = axis;
}

/// tell this item that it is 'selected' within the plot
void MPlotItem::setSelected(bool selected) {
	bool updateNeeded = (selected != isSelected_);
	isSelected_ = selected;
	if(updateNeeded) {
		update();	// todo: maybe should move into subclasses; not all implementations will require update()?
		signalSource()->emitSelectedChanged(isSelected_);
	}
}
/// ask if this item is currently selected on the plot
bool MPlotItem::selected() {
	return isSelected_;
}

/// use this if you don't want a plot item to be selectable:
bool MPlotItem::selectable() {
	return isSelectable_;
}

void MPlotItem::setSelectable(bool selectable) {
	isSelectable_ = selectable;
}


/// Don't call this. Unfortunately public because it's required by MPlot::addItem and MPlot::removeItem.
void MPlotItem::setPlot(MPlot* plot) {
	plot_ = plot;
}

/// returns the plot we are attached to
MPlot* MPlotItem::plot() const {
	return plot_;
}

// Bounding rect: reported in our PlotItem coordinates, which are just the actual data coordinates. This is used by the graphics view system to figure out how much we cover/need to redraw.  Subclasses that draw selection borders or markers need to add their size on top of this.
QRectF MPlotItem::boundingRect() const {
	return dataRect();
}

/// return the active shape where clicking will select this object in the plot. Subclasses can re-implement for more accuracy.
QPainterPath MPlotItem::shape() const {
	QPainterPath shape;
	shape.addRect(boundingRect());
	return shape;
}


#endif // MPLOTITEM_H

