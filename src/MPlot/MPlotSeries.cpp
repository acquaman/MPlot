#ifndef __MPlotSeries_CPP__
#define __MPlotSeries_CPP__

#include "MPlotSeries.h"

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
	dx_ = dy_ = 1.0;
	offset_ = QPointF(0.0,0.0);

	/// Indicates whether normalization is on:
	yAxisNormalizationOn_ = xAxisNormalizationOn_ = false;

	/// Normalization ranges:
	normYMin_ = normXMin_ = 0.0;
	normYMax_ = normXMax_ = 1.0;

	signalHandler_ = new MPlotSeriesSignalHandler(this);

	// Set style defaults:
	setDefaults();	// override in subclasses

	// Set model (will check that data != 0)
	// No! We need a fully instantiated subclass first. [setModel(data);]

}

MPlotAbstractSeries::~MPlotAbstractSeries() {
	// If we have a model, need to disconnect from its updates before we get deleted.
	if(data_)
		QObject::disconnect(data_->signalSource(), 0, signalHandler_, 0);
	delete signalHandler_;
	signalHandler_ = 0;
}



// Properties:
void MPlotAbstractSeries::setLinePen(const QPen& pen) {
	linePen_ = pen;
	linePen_.setCosmetic(true);
	emitLegendContentChanged(); // this changes the legendColor();
}

// Returns the current marker, which can be used to access it's pen, brush, and size.
// If the plot has no marker (or MPlotMarkerShape::None), then this will be a null pointer. Must check before setting.
MPlotAbstractMarker* MPlotAbstractSeries::marker() const {
	return marker_;
}

void MPlotAbstractSeries::setMarker(MPlotMarkerShape::Shape shape, double size, const QPen& pen, const QBrush& brush) {
	if(marker_)
		delete marker_;

	QPen realPen = pen;
	realPen.setCosmetic(true);

	marker_ = MPlotMarker::create(shape, size, realPen, brush);
	update();
}


// Sets this series to view the model in 'data';
void MPlotAbstractSeries::setModel(const MPlotAbstractSeriesData* data) {

	// If there was an old model, disconnect old signals:
	if(data_)
		QObject::disconnect(data_->signalSource(), 0, signalHandler_, 0);

	// new data from here:
	data_ = data;

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


// Required functions:
//////////////////////////
// Bounding rect: reported in our PlotSeries coordinates, which are just the actual data coordinates. This is used by the graphics view system to figure out how much we cover/need to redraw.  Subclasses that draw selection borders or markers need to add their size on top of this.
QRectF MPlotAbstractSeries::boundingRect() const {

	return dataRect();

}

// Data rect: also reported in our PlotSeries coordinates, which are the actual data coordinates. This is used by the auto-scaling to figure out the range of our data on an axis.
QRectF MPlotAbstractSeries::dataRect() const {
	// this indicates that the raw data's bounding rectangle has changed, and we might need to calculate new normalizations.
	if(dataChangedUpdateNeeded_) {
		if(data_) {
			cachedDataRect_ = data_->boundingRect();

			if(yAxisNormalizationOn_) {
				sy_ = (normYMax_ - normYMin_)/(qMax(MPLOT_MIN_NORMALIZATION_RANGE, cachedDataRect_.height()));
				dy_ = normYMin_ - cachedDataRect_.top()*sy_;
			}
			if(xAxisNormalizationOn_) {
				sx_ = (normXMax_ - normXMin_)/(qMax(MPLOT_MIN_NORMALIZATION_RANGE, cachedDataRect_.width()));
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

/*
double MPlotAbstractSeries::yy(unsigned i) const {
	double rv = data_->y(i)*sy_+dy_+offset_.y();
	if(rv < dataRect().top() || rv > dataRect().bottom()) {
		qDebug() << description() << "y val:" << rv << "is outside y data range: bottom:" << dataRect().bottom() << "top:" << dataRect().top();
	}
	return rv;
}*/

QPainterPath MPlotAbstractSeries::shape() const {

	QPainterPath shape;

	// If there's under 1000 points, we can return a detailed shape with ok performance.
	// Above 1000 points, let's just return the bounding box.
	if(data_ && data_->count() > MPLOT_EXACTSHAPE_POINT_LIMIT)
		shape.addRect(boundingRect());


	else if(data_ && data_->count() > 0) {
		shape.moveTo(xx(0), yy(0));
		for(int i=0; i<data_->count(); i++)
			shape.lineTo(xx(i), yy(i));

		for(int i=data_->count()-2; i>=0; i--)
			shape.lineTo(xx(i), yy(i));
		shape.lineTo(xx(0), yy(0));
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

	setYAxisTarget(MPlotAxis::Left);

	setLinePen(QPen(QColor(Qt::red)));	// Red solid lines on plot

	setMarker(MPlotMarkerShape::Square, 6, QPen(QColor(Qt::blue), 0), QBrush()); // Blue outlines on markers, No Brush


	QColor selectionColor = MPLOT_SELECTION_COLOR;
	selectionColor.setAlphaF(MPLOT_SELECTION_OPACITY);
	selectedPen_ = QPen(QBrush(selectionColor), MPLOT_SELECTION_LINEWIDTH);
	selectedPen_.setCosmetic(true);
}


/// Use this function to apply a constant transformation to the series, on top of the underlying data. All data points are first scaled by (\c sx, \c sy) and then shifted by (\c dx, \c dy).
/*! Calling this function will only have an effect on axes which do not have normalization enabled (using enableYAxisNormalization() or enableXAxisNormalization()). If you want your changes to stick, be sure to disable normalization first.*/
void MPlotAbstractSeries::applyTransform(double sx, double sy, double dx, double dy) {
	sx_ = sx;
	sy_ = sy;
	dx_ = dx;
	dy_ = dy;

	onDataChangedPrivate();	// will require an update to the bounding rect, geometry, and plot
}

/// Call this function to keep the data normalized within a specified range.  When normalization is enabled, regardless of how the data source changes, the minimum value will always appear at \c min and the maximum value will always appear at \c max.  This effectively disables applyTransform() in the y-axis.
void MPlotAbstractSeries::enableYAxisNormalization(bool on, double min, double max) {
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

/// Call this function to keep the data normalized within a specified range.  When normalization is enabled, regardless of how the data source changes, the minimum value will always appear at \c min and the maximum value will always appear at \c max.  This effectively disables applyTransform() in the x-axis.
void MPlotAbstractSeries::enableXAxisNormalization(bool on, double min, double max) {
	xAxisNormalizationOn_ = on;
	if(on) {
		normXMin_ = min;
		normXMax_ = max;
	}
	else {
		sx_ = 1.0;
		sy_ = 0.0;
	}

	onDataChangedPrivate();
}


void MPlotAbstractSeries::setOffset(double dx, double dy) {
	offset_ = QPointF(dx, dy);

	onDataChangedPrivate();
}







/////////////////////////////
// MPlotSeriesBasic
////////////////////////////

MPlotSeriesBasic::MPlotSeriesBasic(const MPlotAbstractSeriesData* data)
	: MPlotAbstractSeries() {

	setModel(data);
}



MPlotSeriesBasic::~MPlotSeriesBasic() {

}


// Required functions:
//////////////////////////
// boundingRect: reported in our PlotSeries coordinates, which are just the actual data coordinates.
// using parent implementation, but adding extra room on edges for our selection highlight and markers.
QRectF MPlotSeriesBasic::boundingRect() const {
	QRectF br = MPlotAbstractSeries::boundingRect();
	if(br.isValid()) {
		// create rectangle at least as big as our selection highlight, and if we have a marker, the marker size.
		QRectF hs = QRectF(0, 0, MPLOT_SELECTION_LINEWIDTH, MPLOT_SELECTION_LINEWIDTH);
		// expand by marker size (if expressed in pixels)
		if(marker())
			hs |= QRectF(0,0, marker()->size(), marker()->size());


		// these sizes so far are in pixels (hopefully scene coordinates... trusting on an untransformed view.) Converting to local (data) coordinates.
		hs = mapRectFromScene(hs);

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
	if(selected()) {
		painter->setPen(selectedPen_);
		paintLines(painter);
	}
	painter->setPen(linePen_);
	paintLines(painter);

}

void MPlotSeriesBasic::paintLines(QPainter* painter) {
	// todo: if it's a small dataset (< 500 lines), just draw it asap

	// xinc is the delta-x that corresponds to 1 pixel-width on the screen.  It makes no sense to plot many points inside one delta-x.
	QTransform wt = painter->deviceTransform();	// equivalent to worldTransform and combinedTransform

	/*
		qDebug() << "worldT m11" << painter->worldTransform().m11();
		qDebug() << "deviceT m11" << painter->deviceTransform().m11();
		qDebug() << "combinedT m11" << painter->combinedTransform().m11();
		 */

	double xinc = 1.0 / wt.m11() / MPLOT_MAX_LINES_PER_PIXEL;

	// Instead, we'll just plot the max and min value within every xinc range.  This ensures that if there is noise/jumps within a subsample (xinc) range, we'll still see it on the plot.
	if(data_ && data_->count() > 0) {
		double xstart;
		double ystart, ymin, ymax;

		xstart = xx(0);
		ymin = ymax = ystart = yy(0);

		// move through the datapoints along x. (Note that x could be jumping forward or backward here... it's not necessarily sorted)
		for(int i=1; i<data_->count(); i++) {
			// if within the range around xstart: update max/min to be representative of this range
			if(fabs(xx(i) - xstart) < xinc) {
				if(yy(i) > ymax)
					ymax = yy(i);
				if(yy(i) < ymin)
					ymin = yy(i);
			}
			// otherwise draw the lines and move on to next range...
			// The first line represents everything within the range [xstart, xstart+xinc).  Note that these will all be plotted at same x-pixel.
			// The second line connects this range to the next.  Note that (if the x-axis point spacing is not uniform) x(i) may be many pixels from xstart, to the left or right. All we know is that it's outside of our 1px range. If it _is_ far outside the range, to get the slope of the connecting line correct, we need to connect it to the last point preceding it. The point (x_(i-1), y_(i-1)) is within the 1px range [xstart, x_(i-1)] represented by the vertical line.
			// (Brain hurt? imagine a simple example: (0,2) (0,1) (0,0), (5,0).  It should be a vertical line from (0,2) to (0,0), and then a horizontal line from (0,0) to (5,0).  The xinc range is from i=0 (xstart = x(0)) to i=2. The point outside is i=3.
			// For normal/small datasets where the x-point spacing is >> pixel spacing , what will happen is ymax = ymin = ystart (all the same point), and (x(i), y(i)) is the next point.
			else {
				if(ymin != ymax)
					painter->drawLine(QPointF(xstart, ymin), QPointF(xstart, ymax));

				painter->drawLine(QPointF(xx(i-1), yy(i-1)), QPointF(xx(i), yy(i)));
				//painter->drawLine(QPointF(xstart, ystart), QPointF(xx(i), yy(i)));

				xstart = xx(i);
				ymin = ymax = ystart = yy(i);
			}
		}
	}
}

void MPlotSeriesBasic::paintMarkers(QPainter* painter) {

	QTransform wt = painter->deviceTransform();	// equivalent to worldTransform and combinedTransform
	QTransform wtInverse;
	wtInverse.scale(1/wt.m11(), 1/wt.m22());

	if(data_ && marker_) {

		for(int i=0; i<data_->count(); i++) {
			// Paint marker:
			painter->save();
			painter->translate(xx(i), yy(i));
			painter->setTransform(wtInverse, true);
			marker_->paint(painter);
			painter->restore();
		}
	}
}


/// re-implemented from MPlotItem base to draw an update if we're now selected (with our selection highlight)
void MPlotSeriesBasic::setSelected(bool selected) {

	bool wasSelected = isSelected();
	MPlotItem::setSelected(selected);
	if(isSelected() != wasSelected)
		update();

}

/// All the specific re-drawing we need to do when the data changes (or a new model is set) is contained in update().
void MPlotSeriesBasic::onDataChanged() {
	update();
}


#endif

