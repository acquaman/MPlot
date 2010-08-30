
#ifndef __MPlot_CPP__
#define __MPlot_CPP__

#include "MPlot.h"

/// Defines the minimum distance between min- and max- values for the range of an axis. Without this check, calling setXDataRange(3, 3) or set___DataRange(f, g=f) will cause a segfault within Qt's drawing functions... it can't handle a clipPath with a width of 0.
#define MPLOT_MIN_AXIS_RANGE 1e-60



MPlotSignalHandler::MPlotSignalHandler(MPlot* parent)
	: QObject(0) {
	plot_ = parent;
}

void MPlotSignalHandler::onBoundsChanged() {
	MPlotItemSignalSource* source = qobject_cast<MPlotItemSignalSource*>(sender());
	if(source)
		plot_->onBoundsChanged(source->plotItem());
}

void MPlotSignalHandler::onSelectedChanged(bool isSelected) {
	MPlotItemSignalSource* source = qobject_cast<MPlotItemSignalSource*>(sender());
	if(source)
		plot_->onSelectedChanged(source->plotItem(), isSelected);
}

void MPlotSignalHandler::doDelayedAutoscale() {
	plot_->doDelayedAutoScale();
}


/// This class provides plotting capabilities within a QGraphicsItem that can be added to any QGraphicsScene,
MPlot::MPlot(QRectF rect, QGraphicsItem* parent) :
		QGraphicsItem(parent), rect_(rect)
{
	signalHandler_ = new MPlotSignalHandler(this);

	setFlags(QGraphicsItem::ItemHasNoContents);

	// Create background rectangle of the given size, as a child of this QGraphicsObject.
	// The background coordinate system is in scene coordinates.
	background_ = new QGraphicsRectItem(rect_, this);

	// Create the plot area rectangle.  All plot items, and axes, will be children of plotArea1_
	plotArea_ = new QGraphicsRectItem(QRectF(0, 0, 1, 1), background_);		// The plotArea1_ coordinates are from lower-left (0,0) to upper-right (1,1). It gets transformed to fill the actual plot area within the margins on this plot.
	dataArea_ = new QGraphicsRectItem(QRectF(0,0,1,1), plotArea_);// The plotArea2_ has the same extent and coordinates, but it clips its children to keep plots within the proper borders.
	dataArea_->setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);

	// Create axes (Axes are children of plotArea, to use plotArea_ coordinates from (0,0) to (1,1))
	axes_[MPlotAxis::Left] = new MPlotAxis(MPlotAxis::Left, "y1", plotArea_);
	axes_[MPlotAxis::Right] = new MPlotAxis(MPlotAxis::Right, "y2", plotArea_);
	axes_[MPlotAxis::Bottom] = new MPlotAxis(MPlotAxis::Bottom, "x", plotArea_);
	axes_[MPlotAxis::Top] = new MPlotAxis(MPlotAxis::Top, "", plotArea_);


	// Create Legend:
	/// \todo
	legend_ = new MPlotLegend();
	legend_->setParentItem(plotArea_);


	// Set apperance defaults (override for custom plots)
	setDefaults();

	// Place
	setRect(rect_);

	/// No auto-scale scheduled, and no axes requiring it:
	autoScaleScheduled_ = false;
	axesNeedingAutoScale_ = 0;
	gettingDeleted_ = false;

}


MPlot::~MPlot() {

	gettingDeleted_ = true;
	delete signalHandler_;
	signalHandler_ = 0;
}

/// Required paint function. (All painting is done by children)
void MPlot::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
	Q_UNUSED(painter)
	Q_UNUSED(option)
	Q_UNUSED(widget)
	}

QRectF MPlot::boundingRect() const {
	return rect_;
}

/// Use this to add a new data-item to a plot:
void MPlot::addItem(MPlotItem* newItem) {
	newItem->setParentItem(dataArea_);
	items_ << newItem;
	newItem->setPlot(this);

	// hook up "signals"
	QObject::connect(newItem->signalSource(), SIGNAL(boundsChanged()), signalHandler_, SLOT(onBoundsChanged()));
	QObject::connect(newItem->signalSource(), SIGNAL(selectedChanged(bool)), signalHandler_, SLOT(onSelectedChanged()));

	// if autoscaling is active already, could need to rescale already
	onBoundsChanged(newItem);

	// Apply transforms as needed
	placeItem(newItem);
}

/// Remove a data-item from a plot. (Note: Does not delete the item...)
bool MPlot::removeItem(MPlotItem* removeMe) {
	// optimization: speeds up the ~MPlot() destructor, which will eventually call delete on all child plot items, which will call removeItem() on their plot... us! Don't bother with this whole process.
	if(gettingDeleted_)
		return true;

	if(items_.contains(removeMe)) {
		removeMe->setPlot(0);
		if(scene())
			scene()->removeItem(removeMe);
		else
			removeMe->setParentItem(0);
		items_.removeAll(removeMe);
		// remove signals
		QObject::disconnect(removeMe->signalSource(), 0, signalHandler_, 0);
		// this might need to trigger a re-scale... for ex: if removeMe had the largest/smallest bounds of all plots associated with an auto-scaling axis.
		onBoundsChanged(removeMe);
		return true;
	}
	else
		return false;
}

QList<MPlotItem*> MPlot::plotItems() const {
	return items_;
}

/// Add a tool to the plot:
void MPlot::addTool(MPlotAbstractTool* newTool) {
	newTool->setParentItem(plotArea_);
	tools_ << newTool;

	// placeTool(newTool);
	newTool->setPlot(this);
}

/// Remove a tool from a plot. (Note: Does not delete the tool...)
bool MPlot::removeTool(MPlotAbstractTool* removeMe) {
	if(tools_.contains(removeMe)) {
		removeMe->setPlot(0);
		removeMe->setParentItem(0);
		if(scene())
			scene()->removeItem(removeMe);
		tools_.removeAll(removeMe);
		return true;
	}
	else
		return false;
}

QGraphicsRectItem* MPlot::plotArea() const {
	return plotArea_;
}

// access elements of the canvas:
MPlotAxis* MPlot::axisBottom() {
	return axes_[MPlotAxis::Bottom];
}

MPlotAxis* MPlot::axisTop() {
	return axes_[MPlotAxis::Top];
}

MPlotAxis* MPlot::axisLeft() {
	return axes_[MPlotAxis::Left];
}

MPlotAxis* MPlot::axisRight() {
	return axes_[MPlotAxis::Right];
}


// Access properties of the Canvas: (TODO: add to property system)
QGraphicsRectItem* MPlot::background() {
	return background_;
}

/// returns the rectangle filled by this plot (in scene or parent QGraphicsItem coordinates)
QRectF MPlot::rect() const {
	return rect_;
}

/// Sets the rectangle to be filled by this plot (in scene or parent QGraphicsItem coordinates).
/*! Also rescales and re-applies the margins and transform for the plotArea). Can call with setRect(rect()) to re-compute margins.)*/
void MPlot::setRect(const QRectF& rect) {

	rect_ = rect;

	// margins and dimensions of the plotArea in scene coordinates:
	double left, bottom, w, h;
	left = marginLeft()/100*rect_.width();
	bottom = (1-marginBottom()/100)*rect_.height();
	w = rect_.width()*(1 - marginLeft()/100 - marginRight()/100);
	h = rect_.height()*(1 - marginBottom()/100 - marginTop()/100);

	// scale the background to correct size:
	background_->setRect(rect_);

	// This transform is applied to the plotArea_ to it occupy the correct amount of the scene.
	// It now believes to be drawing itself in a cartesian (right-handed) 0,0 -> 1,1 box.
	plotArea_->setTransform(QTransform::fromTranslate(left, bottom).scale(w,-h));

}


// Margins: are set in logical coordinates (ie: as a percentage of the chart width or chart height);
double MPlot::margin(MPlotAxis::AxisID margin) const {
	return margins_[margin];
}

double MPlot::marginLeft() const {
	return margins_[MPlotAxis::Left];
}

double MPlot::marginRight() const {
	return margins_[MPlotAxis::Right];
}

double MPlot::marginTop() const {
	return margins_[MPlotAxis::Top];
}

double MPlot::marginBottom() const {
	return margins_[MPlotAxis::Bottom];
}

void MPlot::setMargin(MPlotAxis::AxisID margin, double value) {
	margins_[margin] = value; setRect(rect_);
}

void MPlot::setMarginLeft(double value) {
	setMargin(MPlotAxis::Left, value);
}

void MPlot::setMarginRight(double value) {
	setMargin(MPlotAxis::Right, value);
}

void MPlot::setMarginTop(double value) {
	setMargin(MPlotAxis::Top, value);
}

void MPlot::setMarginBottom(double value) {
	setMargin(MPlotAxis::Bottom, value);
}


double MPlot::xMin() {
	return xmin_;
}

double MPlot::xMax() {
	return xmax_;
}

double MPlot::yLeftMin() {
	return yleftmin_;
}

double MPlot::yRightMin() {
	return yrightmin_;
}

double MPlot::yLeftMax() {
	return yleftmax_;
}

double MPlot::yRightMax() {
	return yrightmax_;
}

QTransform MPlot::leftAxisTransform() {
	return leftAxisTransform_;
}

QTransform MPlot::rightAxisTransform() {
	return rightAxisTransform_;
}


void MPlot::enableAutoScaleBottom(bool autoScaleOn) {
	/// \todo defer autoscaling using the delay-to-even-loop method
	if((autoScaleBottomEnabled_ = autoScaleOn))
		setXDataRange(0, 0, true);
}

void MPlot::enableAutoScaleLeft(bool autoScaleOn) {
	/// \todo defer autoscaling using the delay-to-even-loop method
	if((autoScaleLeftEnabled_ = autoScaleOn))
		setYDataRangeLeft(0, 0, true);
}

void MPlot::enableAutoScaleRight(bool autoScaleOn) {
	/// \todo defer autoscaling using the delay-to-even-loop method
	if((autoScaleRightEnabled_ = autoScaleOn))
		setYDataRangeRight(0, 0, true);
}

void MPlot::enableAutoScale(int axisFlags) {
	enableAutoScaleBottom(axisFlags & MPlotAxis::Bottom);
	enableAutoScaleLeft(axisFlags & MPlotAxis::Left);
	enableAutoScaleRight(axisFlags & MPlotAxis::Right);
}



void MPlot::setScalePadding(double percent) {
	scalePadding_ = percent/100;
	// re-scale axis if needed:
	setXDataRangeImp(xmin_, xmax_);
	setYDataRangeLeftImp(yleftmin_, yleftmax_);
	setYDataRangeRightImp(yrightmin_, yrightmax_);

	foreach(MPlotItem* item, items_)
		placeItem(item);
}

double MPlot::scalePadding() {
	return scalePadding_ * 100;
}

void MPlot::setXDataRange(double min, double max, bool autoscale, bool applyPadding) {

	if(autoscale) {
		axesNeedingAutoScale_ |= MPlotAxis::Bottom;
		scheduleDelayedAutoScale();
	}

	else {
		setXDataRangeImp(qMin(min, max), qMax(min, max), autoscale, applyPadding);

		// We have new transforms.  Need to apply them to all item:
		foreach(MPlotItem* item, items_) {
			placeItem(item);
		}
	}

}

void MPlot::setYDataRangeLeft(double min, double max, bool autoscale, bool applyPadding) {

	if(autoscale) {
		axesNeedingAutoScale_ |= MPlotAxis::Left;
		scheduleDelayedAutoScale();
	}

	else {

		setYDataRangeLeftImp(qMin(min, max), qMax(min, max), autoscale, applyPadding);

		// We have new transforms.  Need to apply them:
		foreach(MPlotItem* item, items_) {
			placeItem(item);
		}
	}
}

void MPlot::setYDataRangeRight(double min, double max, bool autoscale, bool applyPadding) {

	if(autoscale) {
		axesNeedingAutoScale_ |= MPlotAxis::Right;
		scheduleDelayedAutoScale();
	}

	else {
		setYDataRangeRightImp(qMin(min, max), qMax(min, max), autoscale, applyPadding);

		// Apply new transforms:
		foreach(MPlotItem* item, items_)
			placeItem(item);
	}
}


#include <QTimer>
void MPlot::onBoundsChanged(MPlotItem *source) {
	bool autoScaleNeeded = false;

	if(autoScaleBottomEnabled_) {
		autoScaleNeeded = true;
		axesNeedingAutoScale_ |= MPlotAxis::Bottom;
	}

	if(autoScaleLeftEnabled_ && (source==0 || source->yAxisTarget() == MPlotAxis::Left)) {
		autoScaleNeeded = true;
		axesNeedingAutoScale_ |= MPlotAxis::Left;
	}

	if(autoScaleRightEnabled_ && (source==0 || source->yAxisTarget() == MPlotAxis::Right)){
		autoScaleNeeded = true;
		axesNeedingAutoScale_ |= MPlotAxis::Right;
	}

	if(autoScaleNeeded)
		scheduleDelayedAutoScale();

}

void MPlot::scheduleDelayedAutoScale() {
	if(!autoScaleScheduled_) {
		autoScaleScheduled_ = true;
		QTimer::singleShot(0, signalHandler_, SLOT(doDelayedAutoscale()));
	}
}

void MPlot::onSelectedChanged(MPlotItem *source, bool isSelected) {
	Q_UNUSED(source)
	Q_UNUSED(isSelected)
	// no action required for now...
	}

void MPlot::doDelayedAutoScale() {

	bool scalesModified = false;

	if( (axesNeedingAutoScale_ & MPlotAxis::Bottom) ) {
		scalesModified = true;
		setXDataRangeImp(0, 0, true);
	}

	if( (axesNeedingAutoScale_ & MPlotAxis::Left) ) {
		scalesModified = true;
		setYDataRangeLeftImp(0, 0, true);
	}

	if( (axesNeedingAutoScale_ & MPlotAxis::Right) ) {
		scalesModified = true;
		setYDataRangeRightImp(0, 0, true);
	}

	// We have new transforms.  Need to apply them:
	if(scalesModified) {
		foreach(MPlotItem* item, items_)
			placeItem(item);
	}


	// Clear this flag... we're completing this scheduled autoscale right now
	autoScaleScheduled_ = false;
	axesNeedingAutoScale_ = 0;
}




/// Applies the leftAxis or rightAxis transformation matrix (depending on the yAxis target)
void MPlot::placeItem(MPlotItem* theItem) {
	if(theItem->yAxisTarget() == MPlotAxis::Right) {
		theItem->setTransform(rightAxisTransform_);
	}
	else {
		theItem->setTransform(leftAxisTransform_);
	}
}




/// Sets the defaults for the drawing options: margins, scale padding, background colors, initial data range.
void MPlot::setDefaults() {

	// Set margin defaults:
	margins_[MPlotAxis::Left] = 15;
	margins_[MPlotAxis::Bottom] = 15;
	margins_[MPlotAxis::Right] = 10;
	margins_[MPlotAxis::Top] = 10;


	scalePadding_ = 0.05;	// 5% axis padding on scale. (ie: setting axis from -1...1 gives extra 5% on each side)

	background_->setBrush(QBrush(QColor(240, 240, 240)));
	background_->setPen(QPen(QBrush(QColor(240,240,240)), 0));

	plotArea_->setBrush(QBrush(QColor(230, 230, 230)));
	plotArea_->setPen(QPen(QBrush(QColor(230, 230, 230)),0));
	/// Data area should be invisible/transparent
	dataArea_->setBrush(QBrush());
	dataArea_->setPen(QPen(QBrush(QColor(230, 230, 230)),0));

	// Starting data ranges:
	setYDataRangeLeft(0, 10);
	setYDataRangeRight(0, 10);
	setXDataRange(0, 10);

	enableAutoScale(0);	// autoscaling disabled on all axes

}


// These implementations leave out the loop that applies the new transforms to all the items.
// If this happens to be expensive, then internally we can just do that loop once after a combination of x- and y-scaling
// (Cuts down on dual x- y- autoscale time)
void MPlot::setXDataRangeImp(double min, double max, bool autoscale, bool applyPadding) {

	// Autoscale?
	if(autoscale) {


		QRectF bounds;
		foreach(MPlotItem* itm, items_)
			bounds |= itm->dataRect();

		if(bounds.isValid()) {
			min = bounds.left();
			max = bounds.right();
		}
		else
			return;	// no item found... Autoscale does nothing.

	}

	// ensure minimum range not violated:
	if(max - min < MPLOT_MIN_AXIS_RANGE)
		max = min + MPLOT_MIN_AXIS_RANGE;

	// Before padding, remember these as our actual axis limits:
	xmin_ = min;
	xmax_ = max;


	if(applyPadding) {
		double padding = (max-min)*scalePadding_;
		min -= padding; max += padding;
	}

	// Transforms: m31 is x-translate. m32 is y-translate. m11 is x-scale; m22 is y-scale. m21=m12=0 (no shear) m33 = 1 (no affine scaling)
	double yscale, ytranslate, xscale, xtranslate;
	yscale = leftAxisTransform_.m22();
	ytranslate = leftAxisTransform_.m32();
	xscale = 1.0/(max-min);
	xtranslate = -min*xscale;

	leftAxisTransform_.setMatrix(xscale, 0, 0, 0, yscale, 0, xtranslate, ytranslate, 1);

	yscale = rightAxisTransform_.m22();
	ytranslate = rightAxisTransform_.m32();

	rightAxisTransform_.setMatrix(xscale, 0, 0, 0, yscale, 0, xtranslate, ytranslate, 1);

	axes_[MPlotAxis::Bottom]->setRange(min, max);
	axes_[MPlotAxis::Top]->setRange(min, max);
}

void MPlot::setYDataRangeLeftImp(double min, double max, bool autoscale, bool applyPadding) {
	// Autoscale?
	if(autoscale) {

		QRectF bounds;
		foreach(MPlotItem* itm, items_) {
			if(itm->yAxisTarget() == MPlotAxis::Left)
				bounds |= itm->dataRect();
		}
		if(bounds.isValid()) {
			min = bounds.top();
			max = bounds.bottom();
		}
		else
			return;	// no items found... Autoscale does nothing.
	}

	// ensure minimum range not violated:
	if(max - min < MPLOT_MIN_AXIS_RANGE)
		max = min + MPLOT_MIN_AXIS_RANGE;

	// Before padding, remember these as our actual axis limits:
	yleftmin_ = min;
	yleftmax_ = max;

	if(applyPadding) {
		double padding = (max-min)*scalePadding_;
		min -= padding; max += padding;
	}
	double xscale, xtranslate, yscale, ytranslate;
	xscale = leftAxisTransform_.m11();
	xtranslate = leftAxisTransform_.m31();
	yscale = 1.0/(max-min);
	ytranslate = -min*yscale;

	leftAxisTransform_.setMatrix(xscale, 0, 0, 0, yscale, 0, xtranslate, ytranslate, 1);

	axes_[MPlotAxis::Left]->setRange(min, max);
}

void MPlot::setYDataRangeRightImp(double min, double max, bool autoscale, bool applyPadding) {

	// Autoscale?
	if(autoscale) {

		QRectF bounds;
		foreach(MPlotItem* itm, items_) {
			if(itm->yAxisTarget() == MPlotAxis::Right)
				bounds |= itm->dataRect();
		}
		if(bounds.isValid()) {
			min = bounds.top();
			max = bounds.bottom();
		}
		else
			return;	// no items found... Autoscale does nothing.
	}

	// ensure minimum range not violated:
	if(max - min < MPLOT_MIN_AXIS_RANGE)
		max = min + MPLOT_MIN_AXIS_RANGE;

	// Before padding, remember these as our actual axis limits:
	yrightmin_ = min;
	yrightmax_ = max;

	if(applyPadding) {
		double padding = (max-min)*scalePadding_;
		min -= padding; max += padding;
	}

	double xscale, xtranslate, yscale, ytranslate;
	xscale = rightAxisTransform_.m11();
	xtranslate = rightAxisTransform_.m31();
	yscale = 1.0/(max-min);
	ytranslate = -min*yscale;

	rightAxisTransform_.setMatrix(xscale, 0, 0, 0, yscale, 0, xtranslate, ytranslate, 1);

	axes_[MPlotAxis::Right]->setRange(min, max);
}

MPlotGW::MPlotGW(QGraphicsItem* parent, Qt::WindowFlags flags) :
		QGraphicsWidget(parent, flags)
{
	plot_ = new MPlot(QRectF(0,0,100,100), this);
}

MPlotGW::~MPlotGW() {
	delete plot_;
}

MPlot* MPlotGW::plot() const { return plot_; }




void MPlotGW::resizeEvent ( QGraphicsSceneResizeEvent * event ) {
	QGraphicsWidget::resizeEvent(event);

	plot_->setRect(QRectF(QPointF(0,0), event->newSize() ));
}

#endif
