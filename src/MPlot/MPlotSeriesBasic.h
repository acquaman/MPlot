#ifndef __MPlotSeriesBasic_H__
#define __MPlotSeriesBasic_H__

#include "MPlotAbstractSeries.h"
#include <QGraphicsLineItem>
#include <QQueue>

#include <QPainterPath>
#include <QPainter>


// When drawing large datasets, we won't draw more than MPLOT_MAX_LINES_PER_PIXEL lines per x-axis pixel.
// We sub-sample by plotting only the maximum and minimum values over the corresponding increment. 
// The value that makes sense here is 1 (since you can't see any more... they'll just look like vertical lines on top of each other.)
#define MPLOT_MAX_LINES_PER_PIXEL 1.0

/// \bug Using test plot in main.cpp with 100,000 pts... resize plot in x and note how slope of connecting line between end of sinusoid and the other points changes as you slide across. should not.


// If you're going to add a lot of points to the model (without caring about updates in between), recommend this for performance reasons:
/*
	MPlotSeriesBasic series;
	 ....
	series->setModel(0);	// disconnect series from data model
	// add points to model...
	series->setModel(model);	// reconnect model to series
 */

class MPlotSeriesBasic : public MPlotAbstractSeries {
	
	Q_OBJECT
	
public:
	
	MPlotSeriesBasic(const MPlotAbstractSeriesData* data = 0, QGraphicsItem* parent = 0) : MPlotAbstractSeries(data, parent) {
		
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
	// using parent implementation
	
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
		if(isSelected_) {
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
