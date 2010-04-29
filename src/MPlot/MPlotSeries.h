#ifndef __MPlotSeries_H__
#define __MPlotSeries_H__

#include "MPlotMarker.h"
#include "MPlotItem.h"
#include "MPlotSeriesData.h"

#include <QPen>
#include <QBrush>
#include <QPainter>
#include <QDebug>


// When the number of points exceeds this, we simply return the bounding box instead of the exact shape of the plot.  Makes selection less precise, but faster.
#define MPLOT_EXACTSHAPE_POINT_LIMIT 10000

/// MPlotAbstractSeries is the base class for all MPlotItems that display series curves (2D curve data) on a plot.  Different drawing implementations are possible. One implementation is provided in MPlotSeriesBasic.
class MPlotAbstractSeries : public MPlotItem {

	Q_OBJECT

public:

	MPlotAbstractSeries(const MPlotAbstractSeriesData* data = 0) : MPlotItem() {

		data_ = 0;
		marker_ = 0;

		// Set style defaults:
		setDefaults();	// override in subclasses

		// Set model (will check that data != 0)
		setModel(data);

	}



	// Properties:
	virtual void setLinePen(const QPen& pen) {
		linePen_ = pen;
		linePen_.setCosmetic(true);
	}

	// Returns the current marker, which can be used to access it's pen, brush, and size.
		// If the plot has no marker (or MPlotMarkerShape::None), then this will be a null pointer. Must check before setting.
	virtual MPlotAbstractMarker* marker() const { return marker_; }

	virtual void setMarkerShape(MPlotMarkerShape::Shape shape) {
		double oldsize = 6;
		QPen oldpen;
		QBrush oldbrush;
		if(marker_) {
			oldsize = marker_->size();
			oldpen = marker_->pen();
			oldbrush = marker_->brush();
			delete marker_;
		}
		marker_ = MPlotMarker::create(shape, oldsize, oldpen, oldbrush);
		update();
	}

	// Note: must use setMarkerShape to something besides MPlotMarkerShape::None before using these, otherwise changes will be lost.
	virtual void setMarkerPen(const QPen& pen) {
		if(marker_) {
			marker_->setPen(pen);
			update();
		}
	}

	virtual void setMarkerBrush(const QBrush& brush) {
		if(marker_) {
			marker_->setBrush(brush);
			update();
		}
	}


	virtual void setMarkerSize(double size) {
		if(marker_) {
			marker_->setSize(size);
			update();
		}
	}


	// Sets this series to view the model in 'data';
	virtual void setModel(const MPlotAbstractSeriesData* data) {

		// If there was an old model, disconnect old signals:
		if(data_)
			disconnect(data_, 0, this, 0);

		// new data from here:
		data_ = data;

		// If there's a new valid model:
		if(data_) {

			// Connect model signals to slots: dataChanged(unsigned fromIndex, unsigned toIndex);
			connect(data_, SIGNAL(dataChanged(unsigned, unsigned)), this, SLOT(onDataChanged(unsigned, unsigned)));
		}

		emit dataChanged(this);

	}

	virtual const MPlotAbstractSeriesData* model() const { return data_; }


	// Required functions:
	//////////////////////////
	// Bounding rect: reported in our PlotSeries coordinates, which are just the actual data coordinates. This is used by the graphics view system to figure out how much we cover/need to redraw.  Subclasses that draw selection borders or markers need to add their size on top of this.
	virtual QRectF boundingRect() const { if(data_) return data_->boundingRect(); else return QRectF(); }

	// Data rect: also reported in our PlotSeries coordinates, which are the actual data coordinates. This is used by the auto-scaling to figure out the range of our data on an axis.
	virtual QRectF dataRect() const { if(data_) return data_->boundingRect(); else return QRectF(); }

	// Paint: must be implemented in subclass.
	virtual void paint(QPainter* painter,
					   const QStyleOptionGraphicsItem* option,
					   QWidget* widget) = 0;

	virtual QPainterPath shape() const {

		QPainterPath shape;

		// If there's under 1000 points, we can return a detailed shape with ok performance.
		// Above 1000 points, let's just return the bounding box.
		if(data_ && data_->count() > MPLOT_EXACTSHAPE_POINT_LIMIT)
			shape.addRect(boundingRect());


		else if(data_ && data_->count() > 0) {
			shape.moveTo(data_->x(0), data_->y(0));
			for(unsigned i=0; i<data_->count(); i++)
				shape.lineTo(data_->x(i), data_->y(i));

			for(int i=data_->count()-2; i>=0; i--)
				shape.lineTo(data_->x(i), data_->y(i));
			shape.lineTo(data_->x(0), data_->y(0));
		}

		return shape;
	}

protected slots:
	virtual void onDataChanged(unsigned fromIndex, unsigned toIndex) {
		Q_UNUSED(fromIndex)
		Q_UNUSED(toIndex)
		emit dataChanged(this);
	}

signals:

	// in base class: void dataChanged(MPlotAbstractSeries* series);	// listen to this if you want to auto-scale on changes.
	// in base class: void selectedChanged(bool isSelected);


protected:
	QPen linePen_, selectedPen_;
	MPlotAbstractMarker* marker_;

	const MPlotAbstractSeriesData* data_;

	virtual void setDefaults() {

		setYAxisTarget(MPlotAxis::Left);

		setLinePen(QPen(QColor(Qt::red)));	// Red solid lines on plot

		setMarkerShape(MPlotMarkerShape::Square);
		setMarkerPen(QPen(QColor(Qt::blue), 0)); // Blue outlines on markers
		setMarkerBrush(QBrush());	// default: NoBrush


		QColor selectionColor = MPLOT_SELECTION_COLOR;
		selectionColor.setAlphaF(MPLOT_SELECTION_OPACITY);
		selectedPen_ = QPen(QBrush(selectionColor), MPLOT_SELECTION_LINEWIDTH);
		selectedPen_.setCosmetic(true);
	}
};






// When drawing large datasets, we won't draw more than MPLOT_MAX_LINES_PER_PIXEL lines per x-axis pixel.
// We sub-sample by plotting only the maximum and minimum values over the x-axis increment that corresponds to 1px.
// The value that makes sense here is 1 (since you can't see any more... they would just look like vertical lines on top of each other anyway.)  When drawing anti-aliased, changing this to 2 makes smoother plots.
#define MPLOT_MAX_LINES_PER_PIXEL 2.0

// If you're going to add a lot of points to the model (without caring about updates in between), recommend this for performance reasons:
/*
	MPlotSeriesBasic series;
	 ....
	series->setModel(0);	// disconnect series from data model
	// add points to model...
	series->setModel(model);	// reconnect model to series
 */

/// MPlotSeriesBasic provides one drawing implementation for a 2D plot curve.  It is optimized to efficiently draw curves with 1,000,000+ data points along the x-axis, by only drawing as many lines as would be visible.

class MPlotSeriesBasic : public MPlotAbstractSeries {

	Q_OBJECT

public:

	MPlotSeriesBasic(const MPlotAbstractSeriesData* data = 0) : MPlotAbstractSeries(data) {

		// no unique setup for MPlotSeriesBasic?


	}

	// Sets this series to view the model in 'data';
	virtual void setModel(const MPlotAbstractSeriesData* data) {

		MPlotAbstractSeries::setModel(data);

		// All we need to do extra: schedule a draw update.  (Note: update() only schedules an painting update. The repaint isn't performed until we get back to Qt's main run loop, so there's no performance hit for calling update() multiple times.)
		update();

	}


	// Required functions:
	//////////////////////////
	// boundingRect: reported in our PlotSeries coordinates, which are just the actual data coordinates.
	// using parent implementation, but adding extra room on edges for our selection highlight and markers.
	virtual QRectF boundingRect() const {
		QRectF br = MPlotAbstractSeries::boundingRect();
		if(br.isValid()) {
			// create rectangle at least as big as our selection highlight, and if we have a marker, the marker size.
			QRectF hs = QRectF(0, 0, MPLOT_SELECTION_LINEWIDTH, MPLOT_SELECTION_LINEWIDTH);
			if(marker())
				hs |= QRectF(0,0, marker()->size(), marker()->size());
			// these sizes so far are in pixels (hopefully scene coordinates... trusting on an untransformed view.) Converting to local coordinates.
			hs = mapRectFromScene(hs);
			// really we just need 1/2 the marker size and 1/2 the selection highlight width. But extra doesn't hurt.
			br.adjust(-hs.width(),-hs.height(),hs.width(), hs.height());
		}
		return br;
	}

	// Paint:
	virtual void paint(QPainter* painter,
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

	virtual void paintLines(QPainter* painter) {
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

			xstart = data_->x(0);
			ymin = ymax = ystart = data_->y(0);

			// move through the datapoints along x. (Note that x could be jumping forward or backward here... it's not necessarily sorted)
			for(unsigned i=1; i<data_->count(); i++) {
				// if within the range around xstart: update max/min to be representative of this range
				if(fabs(data_->x(i) - xstart) < xinc) {
					if(data_->y(i) > ymax)
						ymax = data_->y(i);
					if(data_->y(i) < ymin)
						ymin = data_->y(i);
				}
				// otherwise draw the lines and move on to next range...
				// The first line represents everything within the range [xstart, xstart+xinc).  Note that these will all be plotted at same x-pixel.
				// The second line connects this range to the next.  Note that (if the x-axis point spacing is not uniform) x(i) may be many pixels from xstart, to the left or right. All we know is that it's outside of our 1px range. If it _is_ far outside the range, to get the slope of the connecting line correct, we need to connect it to the last point preceding it. The point (x_(i-1), y_(i-1)) is within the 1px range [xstart, x_(i-1)] represented by the vertical line.
				// (Brain hurt? imagine a simple example: (0,2) (0,1) (0,0), (5,0).  It should be a vertical line from (0,2) to (0,0), and then a horizontal line from (0,0) to (5,0).  The xinc range is from i=0 (xstart = x(0)) to i=2. The point outside is i=3.
				// For normal/small datasets where the x-point spacing is >> pixel spacing , what will happen is ymax = ymin = ystart (all the same point), and (x(i), y(i)) is the next point.
				else {
					if(ymin != ymax)
						painter->drawLine(QPointF(xstart, ymin), QPointF(xstart, ymax));

					painter->drawLine(QPointF(data_->x(i-1), data_->y(i-1)), QPointF(data_->x(i), data_->y(i)));
					//painter->drawLine(QPointF(xstart, ystart), QPointF(data_->x(i), data_->y(i)));

					xstart = data_->x(i);
					ymin = ymax = ystart = data_->y(i);
				}
			}
		}
	}

	virtual void paintMarkers(QPainter* painter) {
		QTransform wt = painter->deviceTransform();	// equivalent to worldTransform and combinedTransform
		QTransform wtInverse;
		wtInverse.scale(1/wt.m11(), 1/wt.m22());

		if(data_ && marker_) {
			for(unsigned i=0; i<data_->count(); i++) {
				// Paint marker:
				painter->save();
				painter->translate(data_->x(i), data_->y(i));
				painter->setTransform(wtInverse, true);
				marker_->paint(painter);
				painter->restore();
			}
		}
	}


	/// re-implemented from MPlotItem base to draw an update if we're now selected (with our selection highlight)
	virtual void setSelected(bool selected = true) {

		bool wasSelected = isSelected();
		MPlotItem::setSelected(selected);
		if(isSelected() != wasSelected)
			update();

	}


protected slots:

	virtual void onDataChanged( unsigned, unsigned ) {

		emit dataChanged(this);
		update();
	}


protected:

	// Customize this if needed for MPlotSeries. For now we use the parent class implementation
	/*
	 virtual void setDefaults() {

	 }*/

};

#endif
