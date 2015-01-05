#ifndef __MPlotSeries_CPP__
#define __MPlotSeries_CPP__

#include "MPlot/MPlotSeries.h"
#include <QPainter>
#include <QDebug>

MPlotSeriesSignalHandler::MPlotSeriesSignalHandler(MPlotAbstractSeries *parent)
	: QObject(0) {
	series_ = parent;
}

void MPlotSeriesSignalHandler::onDataChanged() {
	series_->onDataChangedPrivate();
}

MPlotAbstractSeries::MPlotAbstractSeries() :
	MPlotItem()
{
	data_ = 0;
	marker_ = 0;
	dataChangedUpdateNeeded_ = true;

	/// Scale and shift factors
	sx_ = sy_ = 1.0;
	dx_ = dy_ = 0.0;
	offset_ = QPointF(0.0,0.0);

	/// Indicates whether normalization is on:
	yAxisNormalizationOn_ = xAxisNormalizationOn_ = false;

	/// Normalization ranges:
	normYMin_ = normXMin_ = 0.0;
	normYMax_ = normXMax_ = 1.0;

	signalHandler_ = new MPlotSeriesSignalHandler(this);

}

MPlotAbstractSeries::~MPlotAbstractSeries() {
	// If we have a model, need to disconnect from its updates before we get deleted.
	if(data_) {
		QObject::disconnect(data_->signalSource(), 0, signalHandler_, 0);
		if(ownsModel_) {
			delete data_;
			data_ = 0;
		}
	}
	delete signalHandler_;
	signalHandler_ = 0;
}

// Properties:
void MPlotAbstractSeries::setLinePen(const QPen& pen) {
	linePen_ = pen;
	emitLegendContentChanged(); // this changes the legendColor();
	update();
}

// Returns the current marker, which can be used to access it's pen, brush, and size.
// If the plot has no marker (or MPlotMarkerShape::None), then this will be a null pointer. Must check before setting.
MPlotAbstractMarker* MPlotAbstractSeries::marker() const {
	return marker_;
}

void MPlotAbstractSeries::setMarker(MPlotMarkerShape::Shape shape, qreal size, const QPen& pen, const QBrush& brush) {
	if(marker_)
		delete marker_;

	marker_ = MPlotMarker::create(shape, size, pen, brush);
	update();
}

// Sets this series to view the model in 'data';
void MPlotAbstractSeries::setModel(const MPlotAbstractSeriesData* data, bool ownsModel) {

	// efficiency check: if new model is the same one as old model, don't change anything.
	if(data == data_) {
		ownsModel_ = ownsModel;
		return;
	}

	// Changing models.

	// If there was an old model, disconnect old signals.  Delete the old model if ownsModel_ was set.
	if(data_) {
		QObject::disconnect(data_->signalSource(), 0, signalHandler_, 0);
		if(ownsModel_)
			delete data_;
	}

	// new data from here:
	data_ = data;
	ownsModel_ = ownsModel;

	dataChangedUpdateNeeded_ = true;
	prepareGeometryChange();

	// If there's a new valid model:
	if(data_) {
		QObject::connect(data_->signalSource(), SIGNAL(dataChanged()), signalHandler_, SLOT(onDataChanged()));
	}

	emitBoundsChanged();
	onDataChanged();

}

const MPlotAbstractSeriesData* MPlotAbstractSeries::model() const { return data_; }

void MPlotAbstractSeries::xxValues(unsigned start, unsigned end, qreal *outputValues) const
{
	qreal offset = offset_.x();
	int size = end-start+1;
	QVector<qreal> x = QVector<qreal>(size);
	data_->xValues(start, end, x.data());

	for (int i = 0; i < size; i++)
		outputValues[i] = x.at(i)*sx_ + dx_ + offset;
}

void MPlotAbstractSeries::yyValues(unsigned start, unsigned end, qreal *outputValues) const
{
	qreal offset = offset_.y();
	int size = end-start+1;
	QVector<qreal> y = QVector<qreal>(size);
	data_->yValues(start, end, y.data());

	for (int i = 0; i < size; i++)
		outputValues[i] = y.at(i)*sy_ + dy_ + offset;
}

// Required functions:
//////////////////////////

// Data rect: also reported in our PlotSeries coordinates, which are the actual data coordinates. This is used by the auto-scaling to figure out the range of our data on an axis.
QRectF MPlotAbstractSeries::dataRect() const {
	// this indicates that the raw data's bounding rectangle has changed, and we might need to calculate new normalizations.
	if(dataChangedUpdateNeeded_) {
		if(data_) {
			cachedDataRect_ = data_->boundingRect();

			if(yAxisNormalizationOn_) {
//				sy_ = (normYMax_ - normYMin_)/(qMax(MPLOT_MIN_NORMALIZATION_RANGE, cachedDataRect_.height()));
                sy_ = (normYMax_ - normYMin_)/(qMax(MPLOT_MIN_NORMALIZATION_RANGE, cachedDataRect_.height()) == MPLOT_MIN_NORMALIZATION_RANGE ? 1.0 : cachedDataRect_.height());

                if (sy_ == 0)
                    sy_ = 1.0; // Because there is no practical reason to have a scale of 0.

                dy_ = normYMin_ - cachedDataRect_.top()*sy_;
			}

            if(xAxisNormalizationOn_) {
//				sx_ = (normXMax_ - normXMin_)/(qMax(MPLOT_MIN_NORMALIZATION_RANGE, cachedDataRect_.width()));
				sx_ = (normXMax_ - normXMin_)/(qMax(MPLOT_MIN_NORMALIZATION_RANGE, cachedDataRect_.width()) == MPLOT_MIN_NORMALIZATION_RANGE ? 1.0 : cachedDataRect_.width());

                if (sx_ == 0)
                    sx_ = 1.0; // Because there is no practical reason to have a scale of 0.

                dx_ = normXMin_ - cachedDataRect_.left()*sx_;
			}
			cachedDataRect_ = completeTransform().mapRect(cachedDataRect_);

		}
		else {
			cachedDataRect_ = QRectF();
		}

		dataChangedUpdateNeeded_ = false;
	}

	// in all other cases, simply return this cached bounding rectangle for performance
	return cachedDataRect_;
}

QPainterPath MPlotAbstractSeries::shape() const {

	QPainterPath shape;

	// If there's under 1000 points, we can return a detailed shape with ok performance.
	// Above 1000 points, let's just return the bounding box.
	if(data_ && data_->count() > MPLOT_EXACTSHAPE_POINT_LIMIT)
		shape.addRect(boundingRect());


	else if (data_ && data_->count() > 0){

		int dataCount = data_->count();
		QVector<qreal> x = QVector<qreal>(dataCount);
		QVector<qreal> y = QVector<qreal>(dataCount);

		xxValues(0, dataCount-1, x.data());
		yyValues(0, dataCount-1, y.data());

		QVector<qreal> mappedX = QVector<qreal>(dataCount);
		QVector<qreal> mappedY = QVector<qreal>(dataCount);

		mapXValues(mappedX.size(), x.constData(), mappedX.data());
		mapYValues(mappedY.size(), y.constData(), mappedY.data());

		shape.moveTo(mappedX.at(0), mappedY.at(0));

		for (int i = 0, count = data_->count(); i < count; i++)
			shape.lineTo(mappedX.at(i), mappedY.at(i));

		for (int i = data_->count()-2; i >= 0; i--)
			shape.lineTo(mappedX.at(i), mappedY.at(i));

		shape.moveTo(mappedX.at(0), mappedY.at(0));
	}

	return shape;
}

void	MPlotAbstractSeries::onDataChangedPrivate() {
	// flag cached bounding rect as dirty:
	dataChangedUpdateNeeded_ = true;
	// warn that bounding rect is going to change:
	prepareGeometryChange();
	// Our shape has probably changed, so the plot might need a re-autoscale
	emitBoundsChanged();

	// call any base-class specific re-drawing
	onDataChanged();
}

void MPlotAbstractSeries::setDefaults() {

	setLinePen(QPen(QColor(Qt::red)));	// Red solid lines on plot

	setMarker(MPlotMarkerShape::Square, 6, QPen(QColor(Qt::blue), 0), QBrush()); // Blue outlines on markers, No Brush


	QColor selectionColor = MPLOT_SELECTION_COLOR;
	selectionColor.setAlphaF(MPLOT_SELECTION_OPACITY);
	selectedPen_ = QPen(QBrush(selectionColor), MPLOT_SELECTION_LINEWIDTH);
}


// Use this function to apply a constant transformation to the series, on top of the underlying data. All data points are first scaled by (\c sx, \c sy) and then shifted by (\c dx, \c dy).
/* Calling this function will only have an effect on axes which do not have normalization enabled (using enableYAxisNormalization() or enableXAxisNormalization()). If you want your changes to stick, be sure to disable normalization first.*/
void MPlotAbstractSeries::applyTransform(qreal sx, qreal sy, qreal dx, qreal dy) {
	sx_ = sx;
	sy_ = sy;
	dx_ = dx;
	dy_ = dy;

	onDataChangedPrivate();	// will require an update to the bounding rect, geometry, and plot
}

// Call this function to keep the data normalized within a specified range.  When normalization is enabled, regardless of how the data source changes, the minimum value will always appear at \c min and the maximum value will always appear at \c max.  This effectively disables applyTransform() in the y-axis.
void MPlotAbstractSeries::enableYAxisNormalization(bool on, qreal min, qreal max) {
	yAxisNormalizationOn_ = on;
	if(on) {
		normYMin_ = min;
		normYMax_ = max;
	}
	else {
		sy_ = 1.0;
		dy_ = 0.0;
	}

	onDataChangedPrivate();
}

// Call this function to keep the data normalized within a specified range.  When normalization is enabled, regardless of how the data source changes, the minimum value will always appear at \c min and the maximum value will always appear at \c max.  This effectively disables applyTransform() in the x-axis.
void MPlotAbstractSeries::enableXAxisNormalization(bool on, qreal min, qreal max) {
	xAxisNormalizationOn_ = on;
	if(on) {
		normXMin_ = min;
		normXMax_ = max;
	}
	else {
		sx_ = 1.0;
		dx_ = 0.0;
	}

	onDataChangedPrivate();
}

void MPlotAbstractSeries::setOffset(qreal dx, qreal dy) {
	offset_ = QPointF(dx, dy);

	onDataChangedPrivate();
}

bool MPlotAbstractSeries::drawLines() const
{
	return drawLines_;
}

void MPlotAbstractSeries::setDrawLines(bool newState)
{
	drawLines_ = newState;
}

/////////////////////////////
// MPlotSeriesBasic
////////////////////////////

MPlotSeriesBasic::MPlotSeriesBasic(const MPlotAbstractSeriesData* data)
	: MPlotAbstractSeries() {

	// Set style defaults:
	setDefaults();
	drawLines_ = true;
	setModel(data);
}

MPlotSeriesBasic::~MPlotSeriesBasic() {

}

// Required functions:
//////////////////////////

QRectF MPlotSeriesBasic::boundingRect() const {
	QRectF br = MPlotAbstractSeries::boundingRect();

	if(br.isValid()) {
		// create rectangle at least as big as our selection highlight, and if we have a marker, the marker size.
		QRectF hs = QRectF(0, 0, MPLOT_SELECTION_LINEWIDTH, MPLOT_SELECTION_LINEWIDTH);
		// expand by marker size (expressed in pixels)
		if(marker())
			hs |= QRectF(0,0, marker()->size(), marker()->size());


		// these sizes so far are in pixels (hopefully scene coordinates... trusting on an untransformed view.) Converting to local (data) coordinates.
		// no longer necessary in new drawing coordinate system: hs = mapRectFromScene(hs);

		// really we just need 1/2 the marker size and 1/2 the selection highlight width. But extra doesn't hurt.
		br.adjust(-hs.width(),-hs.height(),hs.width(), hs.height());
	}

	return br;
}

// Paint:
void MPlotSeriesBasic::paint(QPainter* painter,
							 const QStyleOptionGraphicsItem* option,
							 QWidget* widget) {

	Q_UNUSED(option);
	Q_UNUSED(widget);

	if(!yAxisTarget() || !xAxisTarget()) {
		qWarning() << "MPlotSeriesBasic: No axis scale set. Abandoning painting because we don't know what scale to use.";
		return;
	}
	// Plot the markers. Here what makes sense is one marker per data point.  This will be slow for large datasets.
	// use plot->setMarkerShape(MPlotMarkerShape::None) for large sets.
	/////////////////////////////////////////

    if(marker_) {
		painter->setPen(marker_->pen());
		painter->setBrush(marker_->brush());
		paintMarkers(painter);
	}

	// Render lines (this runs fairly fast, even for large datasets, due to subsampling)
	////////////////////////////////////////////
	if(drawLines_)
	{
		if(selected()) {
			painter->setPen(selectedPen_);
			paintLines(painter);
		}
		painter->setPen(linePen_);
		paintLines(painter);
	}
}

void MPlotSeriesBasic::paintLines(QPainter* painter) {

	if(data_ && data_->count() > 0) {

		// xinc is the delta-x (in drawing coordinates, not plot coordinates) that corresponds to 1/MPLOT_MAX_LINES_PER_PIXEL pixel-widths on the device(screen).  It makes no sense to plot many points inside one delta-x. If the x-resolution of the data is so fine as to give us many points within one pixel-width, we optimize to draw no more than two carefully-placed lines within this vertical space... One covering the entire vertical extent of the data at this x-pixel, and one connecting the previous x-pixel to the next with the appropriate slope.

		QTransform wt = painter->deviceTransform();	// equivalent to worldTransform and combinedTransform
		qreal xinc = 1.0 / wt.m11() / MPLOT_MAX_LINES_PER_PIXEL;	// will just be 1/MPLOT_MAX_LINES_PER_PIXEL = 0.5 as long as not using a scaled/transformed painter.

		int dataCount = data_->count();
		QVector<qreal> x = QVector<qreal>(dataCount);
		QVector<qreal> y = QVector<qreal>(dataCount);

		xxValues(0, dataCount-1, x.data());
		yyValues(0, dataCount-1, y.data());

		QVector<qreal> mappedX = QVector<qreal>(dataCount);
		QVector<qreal> mappedY = QVector<qreal>(dataCount);

		mapXValues(mappedX.size(), x.constData(), mappedX.data());
		mapYValues(mappedY.size(), y.constData(), mappedY.data());

		// should we just draw normally and quickly? Do that if the number of data points is less than the number of x-pixels in the drawing space (or half-pixels, in the conservative case where MPLOT_MAX_LINES_PER_PIXEL = 2).
		if(data_->count() < xAxisTarget()->drawingSize().width()/xinc) {

			for (int i = 1, count = data_->count(); i < count; i++)
				painter->drawLine(QPointF(mappedX.at(i-1), mappedY.at(i-1)), QPointF(mappedX.at(i), mappedY.at(i)));
		}

		else {	// do sub-pixel simplification.
			// Instead of drawing lines between all these data points, we'll just plot the max and min value within every xinc range.  This ensures that if there is noise/jumps within a subsample (xinc) range, we'll still see it on the plot.

			qreal xstart;
			qreal ystart, ymin, ymax;

			xstart = mappedX.at(0);
			ymin = ymax = ystart = mappedY.at(0);

			// move through the datapoints along x. (Note that x could be jumping forward or backward here... it's not necessarily sorted)
			for(int i=1, count = data_->count(); i < count; i++) {

				// if within the range around xstart: update max/min to be representative of this range
				if(fabs(mappedX.at(i) - xstart) < xinc) {
					qreal mappedYYI = mappedY.at(i);

					if(mappedYYI > ymax)
						ymax = mappedYYI;
					if(mappedYYI < ymin)
						ymin = mappedYYI;
				}
				// otherwise draw the lines and move on to next range...
				// The first line represents everything within the range [xstart, xstart+xinc).  Note that these will all be plotted at same x-pixel.
				// The second line connects this range to the next.  Note that (if the x-axis point spacing is not uniform) x(i) may be many pixels from xstart, to the left or right. All we know is that it's outside of our 1px range. If it _is_ far outside the range, to get the slope of the connecting line correct, we need to connect it to the last point preceding it. The point (x_(i-1), y_(i-1)) is within the 1px range [xstart, x_(i-1)] represented by the vertical line.
				// (Brain hurt? imagine a simple example: (0,2) (0,1) (0,0), (5,0).  It should be a vertical line from (0,2) to (0,0), and then a horizontal line from (0,0) to (5,0).  The xinc range is from i=0 (xstart = x(0)) to i=2. The point outside is i=3.
				// For normal/small datasets where the x-point spacing is >> pixel spacing , what will happen is ymax = ymin = ystart (all the same point), and (x(i), y(i)) is the next point.
				else {
					if(ymin != ymax)
						painter->drawLine(QPointF(xstart, ymin), QPointF(xstart, ymax));

					painter->drawLine(QPointF(mappedX.at(i-1), mappedY.at(i-1)), QPointF(mappedX.at(i), mappedY.at(i)));
					//NOT: painter->drawLine(QPointF(xstart, ystart), QPointF(mapX(xx(i)), mapY(yy(i))));

					xstart = mappedX.at(i);
					ymin = ymax = ystart = mappedY.at(i);
				}
			}
		}
	}
}

void MPlotSeriesBasic::paintMarkers(QPainter* painter) {

	if(data_ && marker_) {

		int dataCount = data_->count();
		QVector<qreal> x = QVector<qreal>(dataCount);
		QVector<qreal> y = QVector<qreal>(dataCount);

		xxValues(0, dataCount-1, x.data());
		yyValues(0, dataCount-1, y.data());

		QVector<qreal> mappedX = QVector<qreal>(dataCount);
		QVector<qreal> mappedY = QVector<qreal>(dataCount);

		mapXValues(mappedX.size(), x.constData(), mappedX.data());
		mapYValues(mappedY.size(), y.constData(), mappedY.data());

		for (int i = data_->count()-1; i >= 0; i--){

			painter->translate(mappedX.at(i), mappedY.at(i));
			marker_->paint(painter);
			painter->translate(-mappedX.at(i), -mappedY.at(i));
		}
	}
}


// re-implemented from MPlotItem base to draw an update if we're now selected (with our selection highlight)
void MPlotSeriesBasic::setSelected(bool selected) {

	bool wasSelected = isSelected();
	MPlotItem::setSelected(selected);
	if(isSelected() != wasSelected)
		update();

}

// All the specific re-drawing we need to do when the data changes (or a new model is set) is contained in update().
void MPlotSeriesBasic::onDataChanged() {
	update();
}

#endif
