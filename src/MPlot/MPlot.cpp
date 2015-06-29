
#ifndef __MPlot_CPP__
#define __MPlot_CPP__

#include "MPlot/MPlot.h"
#include "MPlot/MPlotAxisScale.h"
#include "MPlot/MPlotSeries.h"
#include "MPlot/MPlotImage.h"
#include "MPlot/MPlotAbstractTool.h"

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

// This class provides plotting capabilities within a QGraphicsItem that can be added to any QGraphicsScene,
MPlot::MPlot(const QRectF& rect, QGraphicsItem* parent) :
	QGraphicsItem(parent), rect_(rect)
{
	signalHandler_ = new MPlotSignalHandler(this);

	// No auto-scale scheduled right now.
	autoScaleScheduled_ = false;

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
	axisScales_ << new MPlotAxisScale(Qt::Vertical, QSizeF(100,100), MPlotAxisRange(0,1), 0);	// verticalRelative (fixed between 0 and 1)
	axisScales_ << new MPlotAxisScale(Qt::Horizontal, QSizeF(100,100), MPlotAxisRange(0,1), 0);// horizontalRelative (fixed between 0 and 1)

	axisScaleLogScaleOn_ << false << false << false << false << false << false;
	axisScaleNormalizationOn_ << false << false << false << false << false << false;
	axisScaleWaterfallAmount_ << 0 << 0 << 0 << 0 << 0 << 0;
	axisScaleNormalizationRange_ << MPlotAxisRange(0,1) << MPlotAxisRange(0,1) << MPlotAxisRange(0,1) << MPlotAxisRange(0,1) << MPlotAxisRange(0,1) << MPlotAxisRange(0,1);

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

	colorLegend_ = new MPlotColorLegend(this, this);
	colorLegend_->setZValue(1e12);
	colorLegend_->setBoxNumber(20);
	colorLegend_->setVisible(false);

	// Set apperance defaults (override for custom plots)
	setDefaults();

	// Place and scale everything as required...
	setRect(rect_);

	gettingDeleted_ = false;

}

MPlot::~MPlot() {

	gettingDeleted_ = true;
	delete signalHandler_;
	signalHandler_ = 0;

	foreach(MPlotAxis* axis, axes_)
		delete axis;
	foreach(MPlotAxisScale* as, axisScales_)
		delete as;
}

// Required paint function. (All painting is done by children)
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

	newItem->setYAxisTarget(axisScale(yAxisTargetIndex));
	newItem->setXAxisTarget(axisScale(xAxisTargetIndex));

	newItem->setParentItem(dataArea_);
	items_.insert(index, newItem);
	newItem->setPlot(this);

	// hook up "signals"
	QObject::connect(newItem->signalSource(), SIGNAL(boundsChanged()), signalHandler_, SLOT(onBoundsChanged()));
	QObject::connect(newItem->signalSource(), SIGNAL(selectedChanged(bool)), signalHandler_, SLOT(onSelectedChanged(bool)));
	QObject::connect(newItem->signalSource(), SIGNAL(legendContentChanged()), signalHandler_, SLOT(onPlotItemLegendContentChanged()));

	// If axis normalization is on already, apply to this series too... (Obviously, only if this item is a series))
	MPlotAbstractSeries* series = qgraphicsitem_cast<MPlotAbstractSeries*>(newItem);
	if(series) {
		series->enableYAxisNormalization(axisScaleNormalizationOn_.at(yAxisTargetIndex), axisScaleNormalizationRange_.at(yAxisTargetIndex));
		series->enableXAxisNormalization(axisScaleNormalizationOn_.at(xAxisTargetIndex), axisScaleNormalizationRange_.at(xAxisTargetIndex));

		if(axisScaleWaterfallAmount_.at(yAxisTargetIndex) != 0)
			setAxisScaleWaterfall(yAxisTargetIndex, axisScaleWaterfallAmount_.at(yAxisTargetIndex));	// apply to whole plot, in case we are inserting not at the end, and other items have to higher in offset to make room for this one.
		if(axisScaleWaterfallAmount_.at(xAxisTargetIndex) != 0)
			setAxisScaleWaterfall(xAxisTargetIndex, axisScaleWaterfallAmount_.at(xAxisTargetIndex));
	}

	// if autoscaling is active already, could need to rescale already
	onBoundsChanged(newItem);

	// make sure the new item has an entry added to the legend
	legend()->onLegendContentChanged(newItem);
}

// Remove a data-item from a plot. (Note: Does not delete the item...)
bool MPlot::removeItem(MPlotItem* removeMe) {
	// optimization: speeds up the ~MPlot() destructor, which will eventually call delete on all child plot items, which will call removeItem() on their plot (ie: us!) Don't bother with this whole process.
	if(gettingDeleted_)
		return true;

	if(items_.contains(removeMe)) {

		MPlotAxisScale* oldYAxisTarget = removeMe->yAxisTarget();
		MPlotAxisScale* oldXAxisTarget = removeMe->xAxisTarget();

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

		// Might need to re-apply the waterfall... If the waterfall on this item's axes was not 0, we might need to move other items' waterfall position down.
		MPlotAbstractSeries* series = qgraphicsitem_cast<MPlotAbstractSeries*>(removeMe);
		if(series) {
			int yAxisTargetIndex = indexOfAxisScale(oldYAxisTarget);
			if(axisScaleWaterfallAmount_.at(yAxisTargetIndex) != 0)
				setAxisScaleWaterfall(yAxisTargetIndex, axisScaleWaterfallAmount_.at(yAxisTargetIndex));
			int xAxisTargetIndex = indexOfAxisScale(oldXAxisTarget);
			if(axisScaleWaterfallAmount_.at(xAxisTargetIndex) != 0)
				setAxisScaleWaterfall(xAxisTargetIndex, axisScaleWaterfallAmount_.at(xAxisTargetIndex));
		}


		legend()->onLegendContentChanged();

		return true;
	}
	else
		return false;
}

// Add a tool to the plot:
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

// Remove a tool from a plot. (Note: Does not delete the tool...)
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

void MPlot::removeTools()
{
	foreach (MPlotAbstractTool *tool, tools_) {
		removeTool(tool);
	}
}

// Sets the rectangle to be filled by this plot (in scene or parent QGraphicsItem coordinates).
/* Also rescales and re-applies the margins and transform for the plotArea). Can call with setRect(rect()) to re-compute margins.)*/
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

	colorLegend_->setHorizontalOffset(w+(marginRight()/100*rect_.width()));
	colorLegend_->setVerticalOffset(marginTop());
}

// called when the autoscaling of an axis scale changes
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

		axis->setDataRange(range);
		axis->setAutoScaleScheduled(false);	// we just completed that.
	}

	// Clear this flag... we're completing this scheduled autoscale right now
	autoScaleScheduled_ = false;
}

// Sets the defaults for the drawing options: margins, scale padding, background colors, initial data range.
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

void MPlot::addAxisScale(MPlotAxisScale *newScale)
{
	axisScales_ << newScale;
	axisScaleLogScaleOn_ << false;
	axisScaleNormalizationOn_ << false;
	axisScaleNormalizationRange_ << MPlotAxisRange(0,1);
	axisScaleWaterfallAmount_ << 0;
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

void MPlot::enableLogScale(int axisScaleIndex, bool logScaleOn)
{
	axisScaleLogScaleOn_[axisScaleIndex] = logScaleOn;

	axisScale(axisScaleIndex)->setLogScaleEnabled(logScaleOn);
}

void MPlot::enableAxisNormalization(int axisScaleIndex, bool normalizationOn, const MPlotAxisRange &normalizationRange)
{
	MPlotAxisScale* axis = axisScale(axisScaleIndex);

	if( (axisScaleNormalizationOn_[axisScaleIndex] = normalizationOn) ) {
		axisScaleNormalizationRange_[axisScaleIndex] = normalizationRange;
	}

	foreach(MPlotItem* item, items_) {
		MPlotAbstractSeries* series = qgraphicsitem_cast<MPlotAbstractSeries*>(item);
		if(axis->orientation() == Qt::Vertical && series && series->yAxisTarget() == axis)
			series->enableYAxisNormalization(normalizationOn, normalizationRange.min(), normalizationRange.max());
		else if(axis->orientation() == Qt::Horizontal && series && series->xAxisTarget() == axis)
			series->enableXAxisNormalization(normalizationOn, normalizationRange.min(), normalizationRange.max());
	}
}

void MPlot::setAxisScaleWaterfall(int axisScaleIndex, qreal amount)
{
	MPlotAxisScale* axis = axisScale(axisScaleIndex);

	axisScaleWaterfallAmount_[axisScaleIndex] = amount;

	int seriesCounter = 0;
	foreach(MPlotItem* item, items_) {
		MPlotAbstractSeries* series = qgraphicsitem_cast<MPlotAbstractSeries*>(item);
		if(axis->orientation() == Qt::Vertical && series && series->yAxisTarget() == axis)
			series->setOffset(0, amount*seriesCounter++);
		else if(axis->orientation() == Qt::Horizontal && series && series->xAxisTarget() == axis)
			series->setOffset(amount*seriesCounter++, 0);
	}
}

double MPlot::minimumXSeriesValue()
{
	double min = MPLOT_POS_INFINITY;

	foreach(MPlotItem* item, items_) {
		MPlotAbstractSeries* series = qgraphicsitem_cast<MPlotAbstractSeries*>(item);
		if (series){

			QRectF rect(series->dataRect());

			if (rect.isValid() && rect.left() < min)
				min = rect.left();
		}
	}

	return min == MPLOT_POS_INFINITY ? MPLOT_NEG_INFINITY : min;
}

double MPlot::maximumXSeriesValue()
{
	double max = MPLOT_NEG_INFINITY;

	foreach(MPlotItem* item, items_) {
		MPlotAbstractSeries* series = qgraphicsitem_cast<MPlotAbstractSeries*>(item);
		if (series) {

			QRectF rect(series->dataRect());

			if (rect.isValid() && rect.right() > max)
				max = rect.right();
		}
	}

	return max == MPLOT_NEG_INFINITY ? MPLOT_POS_INFINITY : max;
}

double MPlot::minimumYSeriesValue()
{
	double min = MPLOT_POS_INFINITY;

	foreach(MPlotItem* item, items_) {
		MPlotAbstractSeries* series = qgraphicsitem_cast<MPlotAbstractSeries*>(item);
		if (series){

			QRectF rect(series->dataRect());

			if (rect.isValid() && rect.top() < min)
				min = rect.top();
		}
	}

	return min == MPLOT_POS_INFINITY ? MPLOT_NEG_INFINITY : min;
}

double MPlot::maximumYSeriesValue()
{
	double max = MPLOT_NEG_INFINITY;

	foreach(MPlotItem* item, items_) {
		MPlotAbstractSeries* series = qgraphicsitem_cast<MPlotAbstractSeries*>(item);
		if (series) {

			QRectF rect(series->dataRect());

			if (rect.isValid() && rect.bottom() > max)
				max = rect.bottom();
		}
	}

	return max == MPLOT_NEG_INFINITY ? MPLOT_POS_INFINITY : max;
}

int MPlot::seriesItemsCount() const
{
	int rv = 0;
	foreach(MPlotItem* item, items_) {
		MPlotAbstractSeries* series = qgraphicsitem_cast<MPlotAbstractSeries*>(item);
		if (series)
			rv++;
	}
	return rv;
}

int MPlot::imageItemsCount() const
{
	int rv = 0;
	foreach(MPlotItem* item, items_) {
		MPlotAbstractImage* image = qgraphicsitem_cast<MPlotAbstractImage*>(item);
		if (image)
			rv++;
	}
	return rv;
}


//////////////////////
// MPlotGW
////////////////////////////

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
