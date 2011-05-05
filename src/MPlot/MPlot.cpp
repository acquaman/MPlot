
#ifndef __MPlot_CPP__
#define __MPlot_CPP__

#include "MPlot.h"
#include "MPlotAxisScale.h"

#include <QDebug>



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

void MPlotSignalHandler::onPlotItemLegendContentChanged() {
	MPlotItemSignalSource* source = qobject_cast<MPlotItemSignalSource*>(sender());
	if(source)
		plot_->onPlotItemLegendContentChanged(source->plotItem());
	else
		plot_->onPlotItemLegendContentChanged(0);
}




/// This class provides plotting capabilities within a QGraphicsItem that can be added to any QGraphicsScene,
MPlot::MPlot(QRectF rect, QGraphicsItem* parent) :
	QGraphicsItem(parent), rect_(rect)
{
	signalHandler_ = new MPlotSignalHandler(this);

	setFlags(QGraphicsItem::ItemHasNoContents);	// drawing optimization; all drawing done by children

	// Create background rectangle of the given size, as a child of this QGraphicsObject.
	// The background coordinate system is in scene coordinates.
	background_ = new QGraphicsRectItem(rect_, this);

	// Create the plot area rectangle.  All plot items, and axes, will be children of plotArea_. When the size of these areas changes, the axisScales_ will be notified of the new drawing sizes.  This allows us to use an untransformed painter, which has slightly higher drawing performance.
	plotArea_ = new QGraphicsRectItem(QRectF(0, 0, 100, 100), background_);	// we'll adjust these sizes in setRect shortly.
	dataArea_ = new QGraphicsRectItem(QRectF(0,0,100,100), plotArea_);// The dataArea_ has the same extent as plotArea_, but it clips its children to keep plots within the proper borders.
	dataArea_->setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);

	axisScales_ << new MPlotAxisScale(Qt::Vertical);	// left
	axisScales_ << new MPlotAxisScale(Qt::Horizontal);	// bottom
	axisScales_ << new MPlotAxisScale(Qt::Vertical);	// right
	axisScales_ << new MPlotAxisScale(Qt::Horizontal);	// top
	axisScales_ << new MPlotAxisScale(Qt::Vertical, QSizeF(100,100), MPlotAxisRange(0,1));	// verticalRelative (fixed between 0 and 1)
	axisScales_ << new MPlotAxisScale(Qt::Horizontal, QSizeF(100,100), MPlotAxisRange(0,1));// horizontalRelative (fixed between 0 and 1)

	foreach(MPlotAxisScale* axisScale, axisScales_) {
		QObject::connect(axisScale, SIGNAL(autoScaleEnabledChanged(bool)), signalHandler_, SLOT(onAxisScaleAutoScaleEnabledChanged(bool)));
	}

	// Create axes (Axes are children of plotArea)
	axes_ << new MPlotAxis(axisScaleLeft(), MPlotAxis::OnLeft, "y", plotArea_);
	axes_ << new MPlotAxis(axisScaleBottom(), MPlotAxis::OnBottom, "x", plotArea_);
	axes_ << new MPlotAxis(axisScaleLeft(), MPlotAxis::OnRight, "", plotArea_);
	axes_ << new MPlotAxis(axisScaleBottom(), MPlotAxis::OnTop, "", plotArea_);	// note: this is intentional. By default the top (visible) axis displays the Bottom axis scale, and the right (visible) axis displays the Left axis scale. That's because most people don't use a separate top and right axis, so we line up the tick marks with the conventional axis on the opposite side.  If you want to display the top axis on the unique top axis scale, call axis(MPlot::Right)->setAxisScale(axisScale(MPlot::Right));


	// Create Legend:
	legend_ = new MPlotLegend(this, this);
	legend_->setZValue(1e12);	// legends should display above everything else...


	/// \todo Fix normalization and waterfall.
	//	normBottomEnabled_ = normLeftEnabled_ = normRightEnabled_ = false;
	//	waterfallLeftAmount_ = waterfallRightAmount_ = 0;

	// Set apperance defaults (override for custom plots)
	setDefaults();

	// Place and scale everything as required...
	setRect(rect_);

	// No auto-scale scheduled right now.
	autoScaleScheduled_ = false;

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

void MPlot::insertItem(MPlotItem* newItem, int index, int yAxisTargetIndex, int xAxisTargetIndex) {
	if(index < 0 || index > numItems())
		index = numItems();

	newItem->setParentItem(dataArea_);
	items_.insert(index, newItem);
	newItem->setPlot(this);

	newItem->setYAxisTarget(axisScale(yAxisTargetIndex));
	newItem->setXAxisTarget(axisScale(xAxisTargetIndex));

	// hook up "signals"
	QObject::connect(newItem->signalSource(), SIGNAL(boundsChanged()), signalHandler_, SLOT(onBoundsChanged()));
	QObject::connect(newItem->signalSource(), SIGNAL(selectedChanged(bool)), signalHandler_, SLOT(onSelectedChanged(bool)));
	QObject::connect(newItem->signalSource(), SIGNAL(legendContentChanged()), signalHandler_, SLOT(onPlotItemLegendContentChanged()));

	/// \todo Fix axis normalization and waterfall.  (if axis normalization is on already, apply to this series too... (Obviously, only if this item is a series))
	//	MPlotAbstractSeries* s = qgraphicsitem_cast<MPlotAbstractSeries*>(newItem);
	//	if(s) {
	//		s->enableXAxisNormalization(normBottomEnabled_, normBottomRange_.first, normBottomRange_.second);
	//		if(s->yAxisTarget() == MPlotAxis::Left)
	//			s->enableYAxisNormalization(normLeftEnabled_, normLeftRange_.first, normLeftRange_.second);
	//		if(s->yAxisTarget() == MPlotAxis::Right)
	//			s->enableYAxisNormalization(normRightEnabled_, normRightRange_.first, normRightRange_.second);

	//		if(s->yAxisTarget() == MPlotAxis::Left)
	//			s->setOffset(0, waterfallLeftAmount_*seriesCounterLeft_++);
	//		if(s->yAxisTarget() == MPlotAxis::Right)
	//			s->setOffset(0, waterfallRightAmount_*seriesCounterRight_++);
	//	}

	// if autoscaling is active already, could need to rescale already
	onBoundsChanged(newItem);

	legend()->onLegendContentChanged(newItem);
}

/// Remove a data-item from a plot. (Note: Does not delete the item...)
bool MPlot::removeItem(MPlotItem* removeMe) {
	// optimization: speeds up the ~MPlot() destructor, which will eventually call delete on all child plot items, which will call removeItem() on their plot (ie: us!) Don't bother with this whole process.
	if(gettingDeleted_)
		return true;

	if(items_.contains(removeMe)) {

		// this also might need to trigger a re-scale... for ex: if removeMe had the largest/smallest bounds of all plots associated with an auto-scaling axis.
		onBoundsChanged(removeMe);

		removeMe->setYAxisTarget(0);
		removeMe->setXAxisTarget(0);
		removeMe->setPlot(0);

		if(scene())
			scene()->removeItem(removeMe);
		else
			removeMe->setParentItem(0);

		items_.removeAll(removeMe);

		legend()->onLegendContentChanged(removeMe);

		// remove signals
		QObject::disconnect(removeMe->signalSource(), 0, signalHandler_, 0);

		/// \todo Fix waterfall. (this might need to re-apply the waterfall...)
		//		MPlotAbstractSeries* series = qgraphicsitem_cast<MPlotAbstractSeries*>(removeMe);
		//		if(series && series->yAxisTarget() == MPlotAxis::Left && waterfallLeftAmount_ != 0.0)
		//			setWaterfallLeft(waterfallLeftAmount_);
		//		if(series && series->yAxisTarget() == MPlotAxis::Right && waterfallRightAmount_ != 0.0)
		//			setWaterfallRight(waterfallRightAmount_);


		legend()->onLegendContentChanged();

		return true;
	}
	else
		return false;
}



/// Add a tool to the plot:
void MPlot::addTool(MPlotAbstractTool* newTool) {
	newTool->setParentItem(plotArea_);
	newTool->setRect(QRectF(QPointF(0,0), plotAreaRect_.size()));
	tools_ << newTool;

	newTool->setPlot(this);

	QList<MPlotAxisScale*> axisScales;
	for(int i=0; i<axisScales_.count(); i++) {
		if(i!=MPlot::HorizontalRelative && i!=MPlot::VerticalRelative)
			axisScales << axisScales_.at(i);
	}
	newTool->setTargetAxes(axisScales);	// by default, plot tools get attached to all the existing axis scales... EXCEPT FOR THE PLOT-RELATIVE axes... because we don't want these modified.  If they want to choose a different set, users must call setTargetAxes() after adding the tool to the plot.
}

/// Remove a tool from a plot. (Note: Does not delete the tool...)
bool MPlot::removeTool(MPlotAbstractTool* removeMe) {

	// optimization: speeds up the ~MPlot() destructor, which will eventually call delete on all child plot items, which will call removeItem() on their plot (ie: us!) Don't bother with this whole process.
	if(gettingDeleted_)
		return true;

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




/// Sets the rectangle to be filled by this plot (in scene or parent QGraphicsItem coordinates).
/*! Also rescales and re-applies the margins and transform for the plotArea). Can call with setRect(rect()) to re-compute margins.)*/
void MPlot::setRect(const QRectF& rect) {

	rect_ = rect;

	// margins and dimensions of the plotArea in scene coordinates:
	qreal left, top, bottom, w, h;
	left = marginLeft()/100*rect_.width();
	top = marginTop()/100*rect_.height();
	bottom = (1-marginBottom()/100)*rect_.height();
	w = rect_.width()*(1 - marginLeft()/100 - marginRight()/100);
	h = rect_.height()*(1 - marginBottom()/100 - marginTop()/100);
	plotAreaRect_ = QRectF(left, top, w, h);

	// scale the background to correct size:
	background_->setRect(rect_);

	plotArea_->setPos(left, top);
	plotArea_->setRect(QRectF(QPointF(0,0), plotAreaRect_.size()));	// this will displace the plotArea to the right place, and size it to the size we want.
	dataArea_->setRect(QRectF(QPointF(0,0), plotAreaRect_.size()));	// The dataArea_ has the same size, but because it's a child of plotArea_, it doesn't need a position offset.

	// OLD: This transform is applied to the plotArea_ to it occupy the correct amount of the scene.
	// It now believes to be drawing itself in a cartesian (right-handed) 0,0 -> 1,1 box.
	//plotArea_->setTransform(QTransform::fromTranslate(left, bottom).scale(w,-h));

	// tell the axisScales of their new size
	foreach(MPlotAxisScale* axis, axisScales_) {
		axis->setDrawingSize(plotAreaRect_.size());
	}

	// tell the tools to update their size
	foreach(MPlotAbstractTool* tool, tools_) {
		tool->setRect(QRectF(QPointF(0,0), plotAreaRect_.size()));
	}

	legend_->setPos(left, top);
	legend_->setWidth(w);

}







/// called when the autoscaling of an axis scale changes
void MPlot::onAxisScaleAutoScaleEnabledChanged(bool autoScaleEnabled) {
	// when it turns on, need to schedule the autoscaling routine. (If it has been scheduled already, this does nothing, so it's okay to do repeatedly.)
	if(autoScaleEnabled)
		scheduleDelayedAutoScale();
}




#include <QTimer>
void MPlot::onBoundsChanged(MPlotItem *source) {

	if(source->ignoreWhenAutoScaling())
		return;

	MPlotAxisScale* xAxis = source->xAxisTarget();

	if(xAxis->autoScaleEnabled()) {
		xAxis->setAutoScaleScheduled();
		scheduleDelayedAutoScale();
	}

	MPlotAxisScale* yAxis = source->yAxisTarget();

	if(yAxis->autoScaleEnabled()) {
		yAxis->setAutoScaleScheduled();
		scheduleDelayedAutoScale();
	}
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
	if(!autoScaleScheduled_)
		return;

	for(int i=axisScales_.count()-1; i>= 0; i--) {
		MPlotAxisScale* axis = axisScales_.at(i);

		if(!(axis->autoScaleEnabled() && axis->autoScaleScheduled()))
			continue;

		MPlotAxisRange range;
		foreach(MPlotItem* item, items_) {
			if(!item->ignoreWhenAutoScaling()) {

				if(axis->orientation() == Qt::Vertical && item->yAxisTarget() == axis)
					range |= MPlotAxisRange(item->dataRect(), Qt::Vertical);

				else if(axis->orientation() == Qt::Horizontal && item->xAxisTarget() == axis)
					range |= MPlotAxisRange(item->dataRect(), Qt::Horizontal);
			}
		}

		if(!range.isValid())
			continue;	// there are no items to autoscale on this axis... Don't do anything for it.

		if(range.length() < MPLOT_MIN_AXIS_RANGE)
			range.setMax(range.min() + MPLOT_MIN_AXIS_RANGE);	// ensure that the axis has at least a non-zero length.  (Otherwise we have division by zero and Qt drawing bug problems.)

		axis->setDataRange(range);
		axis->setAutoScaleEnabled();	// setDataRange() will have turned off MPlotAxisScale::autoScaleEnabled()... We need to leave it on for next time.

		axis->setAutoScaleScheduled(false);	// we just completed that.
	}

	// Clear this flag... we're completing this scheduled autoscale right now
	autoScaleScheduled_ = false;
}







/// Sets the defaults for the drawing options: margins, scale padding, background colors, initial data range.
void MPlot::setDefaults() {

	// Set margin defaults:
	margins_[MPlot::Left] = 15;
	margins_[MPlot::Bottom] = 15;
	margins_[MPlot::Right] = 10;
	margins_[MPlot::Top] = 10;


	background_->setBrush(QBrush(QColor(240, 240, 240)));
	background_->setPen(QPen(QBrush(QColor(240,240,240)), 0));

	plotArea_->setBrush(QBrush(QColor(230, 230, 230)));
	plotArea_->setPen(QPen(QBrush(QColor(230, 230, 230)),0));
	/// Data area should be invisible/transparent
	dataArea_->setBrush(QBrush());
	dataArea_->setPen(QPen(QBrush(QColor(230, 230, 230)),0));

	// autoscaling is turned on by default.
	axisScaleLeft()->setAutoScaleEnabled();
	axisScaleBottom()->setAutoScaleEnabled();

}

void MPlot::onPlotItemLegendContentChanged(MPlotItem* changedItem) {
	legend()->onLegendContentChanged(changedItem);
}



//// These implementations leave out the loop that applies the new transforms to all the items.
//// If this happens to be expensive, then internally we can just do that loop once after a combination of x- and y-scaling
//// (Cuts down on dual x- y- autoscale time)
//void MPlot::setXDataRangeImp(qreal min, qreal max, bool autoscale, bool applyPadding) {

//	// Autoscale?
//	if(autoscale) {


//		QRectF bounds;
//		foreach(MPlotItem* itm, items_)
//			bounds |= itm->dataRect();

//		if(bounds.isValid()) {
//			min = bounds.left();
//			max = bounds.right();
//		}
//		else
//			return;	// no item found... Autoscale does nothing.

//	}

//	// ensure minimum range not violated:
//	if(max - min < MPLOT_MIN_AXIS_RANGE)
//		max = min + MPLOT_MIN_AXIS_RANGE;

//	// Before padding, remember these as our actual axis limits:
//	xmin_ = min;
//	xmax_ = max;


//	if(applyPadding) {
//		qreal padding = (max-min)*scalePadding_;
//		min -= padding; max += padding;
//	}

//	// Transforms: m31 is x-translate. m32 is y-translate. m11 is x-scale; m22 is y-scale. m21=m12=0 (no shear) m33 = 1 (no affine scaling)
//	qreal yscale, ytranslate, xscale, xtranslate;
//	yscale = leftAxisTransform_.m22();
//	ytranslate = leftAxisTransform_.m32();
//	xscale = 1.0/(max-min);
//	xtranslate = -min*xscale;

//	leftAxisTransform_.setMatrix(xscale, 0, 0, 0, yscale, 0, xtranslate, ytranslate, 1);

//	yscale = rightAxisTransform_.m22();
//	ytranslate = rightAxisTransform_.m32();

//	rightAxisTransform_.setMatrix(xscale, 0, 0, 0, yscale, 0, xtranslate, ytranslate, 1);

//	axes_[MPlotAxis::Bottom]->setRange(min, max);
//	axes_[MPlotAxis::Top]->setRange(min, max);
//}

//void MPlot::setYDataRangeLeftImp(qreal min, qreal max, bool autoscale, bool applyPadding) {
//	// Autoscale?
//	if(autoscale) {

//		QRectF bounds;
//		foreach(MPlotItem* itm, items_) {
//			if(itm->yAxisTarget() == MPlotAxis::Left)
//				bounds |= itm->dataRect();
//		}
//		if(bounds.isValid()) {
//			min = bounds.top();
//			max = bounds.bottom();
//		}
//		else
//			return;	// no items found... Autoscale does nothing.
//	}

//	// ensure minimum range not violated:
//	if(max - min < MPLOT_MIN_AXIS_RANGE)
//		max = min + MPLOT_MIN_AXIS_RANGE;

//	// Before padding, remember these as our actual axis limits:
//	yleftmin_ = min;
//	yleftmax_ = max;

//	if(applyPadding) {
//		qreal padding = (max-min)*scalePadding_;
//		min -= padding; max += padding;
//	}
//	qreal xscale, xtranslate, yscale, ytranslate;
//	xscale = leftAxisTransform_.m11();
//	xtranslate = leftAxisTransform_.m31();
//	yscale = 1.0/(max-min);
//	ytranslate = -min*yscale;

//	leftAxisTransform_.setMatrix(xscale, 0, 0, 0, yscale, 0, xtranslate, ytranslate, 1);

//	axes_[MPlotAxis::Left]->setRange(min, max);
//}

//void MPlot::setYDataRangeRightImp(qreal min, qreal max, bool autoscale, bool applyPadding) {

//	// Autoscale?
//	if(autoscale) {

//		QRectF bounds;
//		foreach(MPlotItem* itm, items_) {
//			if(itm->yAxisTarget() == MPlotAxis::Right)
//				bounds |= itm->dataRect();
//		}
//		if(bounds.isValid()) {
//			min = bounds.top();
//			max = bounds.bottom();
//		}
//		else
//			return;	// no items found... Autoscale does nothing.
//	}

//	// ensure minimum range not violated:
//	if(max - min < MPLOT_MIN_AXIS_RANGE)
//		max = min + MPLOT_MIN_AXIS_RANGE;

//	// Before padding, remember these as our actual axis limits:
//	yrightmin_ = min;
//	yrightmax_ = max;

//	if(applyPadding) {
//		qreal padding = (max-min)*scalePadding_;
//		min -= padding; max += padding;
//	}

//	qreal xscale, xtranslate, yscale, ytranslate;
//	xscale = rightAxisTransform_.m11();
//	xtranslate = rightAxisTransform_.m31();
//	yscale = 1.0/(max-min);
//	ytranslate = -min*yscale;

//	rightAxisTransform_.setMatrix(xscale, 0, 0, 0, yscale, 0, xtranslate, ytranslate, 1);

//	axes_[MPlotAxis::Right]->setRange(min, max);
//}






//void MPlot::enableAxisNormalizationBottom(bool normalizationOn, qreal min, qreal max) {
//	if( (normBottomEnabled_ = normalizationOn) ) {
//		normBottomRange_.first = min;
//		normBottomRange_.second = max;
//	}

//	for(int i=0; i<items_.count(); i++) {
//		MPlotAbstractSeries* series = qgraphicsitem_cast<MPlotAbstractSeries*>(items_.at(i));
//		if(series) {
//			series->enableXAxisNormalization(normalizationOn, min, max);
//		}
//	}

//}

//void MPlot::enableAxisNormalizationLeft(bool normalizationOn, qreal min, qreal max) {
//	if( (normLeftEnabled_ = normalizationOn) ) {
//		normLeftRange_.first = min;
//		normLeftRange_.second = max;
//	}

//	for(int i=0; i<items_.count(); i++) {
//		MPlotAbstractSeries* series = qgraphicsitem_cast<MPlotAbstractSeries*>(items_.at(i));
//		if(series && series->yAxisTarget() == MPlotAxis::Left) {
//			series->enableYAxisNormalization(normalizationOn, min, max);
//		}
//	}
//}


//void MPlot::enableAxisNormalizationRight(bool normalizationOn, qreal min, qreal max) {
//	if( (normRightEnabled_ = normalizationOn) ) {
//		normRightRange_.first = min;
//		normRightRange_.second = max;
//	}

//	for(int i=0; i<items_.count(); i++) {
//		MPlotAbstractSeries* series = qgraphicsitem_cast<MPlotAbstractSeries*>(items_.at(i));
//		if(series && series->yAxisTarget() == MPlotAxis::Right) {
//			series->enableYAxisNormalization(normalizationOn, min, max);
//		}
//	}
//}

//void MPlot::enableAxisNormalization(int axisFlags) {
//	enableAxisNormalizationBottom(axisFlags & MPlotAxis::Bottom);
//	enableAxisNormalizationLeft(axisFlags & MPlotAxis::Left);
//	enableAxisNormalizationRight(axisFlags & MPlotAxis::Right);
//}

//void MPlot::setWaterfallLeft(qreal amount) {
//	waterfallLeftAmount_ = amount;
//	seriesCounterLeft_ = 0;
//	for(int i=0; i<items_.count(); i++) {
//		MPlotAbstractSeries* series = qgraphicsitem_cast<MPlotAbstractSeries*>(items_.at(i));
//		if(series && series->yAxisTarget() == MPlotAxis::Left)
//			series->setOffset(0, amount*seriesCounterLeft_++);
//	}
//}

//void MPlot::setWaterfallRight(qreal amount) {
//	waterfallRightAmount_ = amount;
//	seriesCounterRight_ = 0;
//	for(int i=0; i<items_.count(); i++) {
//		MPlotAbstractSeries* series = qgraphicsitem_cast<MPlotAbstractSeries*>(items_.at(i));
//		if(series && series->yAxisTarget() == MPlotAxis::Right)
//			series->setOffset(0, amount*seriesCounterRight_++);
//	}
//}


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

void MPlot::addAxisScale(MPlotAxisScale *newScale)
{
	axisScales_ << newScale;
	QObject::connect(newScale, SIGNAL(autoScaleEnabledChanged(bool)), signalHandler_, SLOT(onAxisScaleAutoScaleEnabledChanged(bool)));
}

void MPlotSignalHandler::doDelayedAutoscale()
{
	plot_->doDelayedAutoScale();
}

void MPlotSignalHandler::onAxisScaleAutoScaleEnabledChanged(bool enabled)
{
	plot_->onAxisScaleAutoScaleEnabledChanged(enabled);
}




#endif
