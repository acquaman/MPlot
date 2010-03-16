#ifndef __MPlot_H__
#define __MPlot_H__

#include <QGraphicsScene>

#include "MPlotBackground.h"
#include "MPlotAxis.h"
#include "MPlotLegend.h"
#include "MPlotSeries.h"
#include <QList>


#define SCENESIZE 100

// This is the color of the selection highlight
#define MPLOT_SELECTION_COLOR QColor(255, 210, 129)
#define MPLOT_SELECTION_HIGHLIGHT_ZVALUE -1000


class MPlot : public QGraphicsScene {
    Q_OBJECT

public:
	MPlot(QObject* parent = 0) : QGraphicsScene(0, 0, SCENESIZE, SCENESIZE, parent) {
		
		// TODO: test performance of:
		// setItemIndexMethod(NoIndex);
		
		// initial aspect ratio:
		ar_ = 1.0;
	
		// Create background rectangle:
		background_ = new MPlotBackground(sceneRect(), 0);	// no parent.
		this->addItem(background_);	// background added first... on the bottom.
		
		// SCHEUDLE FOR DELETION: connect(background_, SIGNAL(backgroundPressed()), this, SLOT(onBackgroundPressed()));
		
		// Create plot area rectangle.  All plot items will be children of plotArea_
		plotArea_ = new MPlotBackground(QRectF(0, 0, 1, 1), 0);		// The plotArea_ has local coordinates from (0,0) to (1,1), transformed appropriately 
		plotArea_->setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);
		this->addItem(plotArea_);
		
		// Create axes:
		axes_[MPlotAxis::Left] = new MPlotAxis(MPlotAxis::Left, 0);	// axes are top-level (scene-level) items
		this->addItem(axes_[MPlotAxis::Left]);
		axes_[MPlotAxis::Right] = new MPlotAxis(MPlotAxis::Right, 0);	// axes are top-level (scene-level) items
		this->addItem(axes_[MPlotAxis::Right]);
		axes_[MPlotAxis::Bottom] = new MPlotAxis(MPlotAxis::Bottom, 0);	// axes are top-level (scene-level) items
		this->addItem(axes_[MPlotAxis::Bottom]);
		axes_[MPlotAxis::Top] = new MPlotAxis(MPlotAxis::Top, 0);	// axes are top-level (scene-level) items
		this->addItem(axes_[MPlotAxis::Top]);
		
		
		// Create Legend:
		legend_ = new MPlotLegend();
		this->addItem(legend_);

		
		// Set apperance defaults (override for custom plots)
		setDefaults();
		
		// Place 
		placeComponents();
		
		// Prepare the plot series we use to draw highlights:
		highlightSeries_ = new MPlotSeries;
		highlightSeries_->setObjectName("MPLOT_HIGHLIGHT");	// this is important... For now, using this to designate plotSeries just used to highlight other plots. (TODO: clean up)
		QPen linePen(QBrush(MPLOT_SELECTION_COLOR), 10);
		highlightSeries_->setLinePen(linePen);
		highlightSeries_->setMarkerShape(MPlotMarkerShape::None);
		highlightSeries_->setZValue(MPLOT_SELECTION_HIGHLIGHT_ZVALUE);
		highlightSeries_->setVisible(false);
		addSeries(highlightSeries_);
		
		
	}

	
	
	// Use this to add a series to a plot:
	void addSeries(MPlotSeries* newSeries) {
		newSeries->setParentItem(plotArea_);
		series_ << newSeries;
		
		connect(newSeries, SIGNAL(dataChanged(MPlotSeries*)), this, SLOT(onDataChanged(MPlotSeries*)));
		// Possible optimization: only connect series to this slot when continuous autoscaling is enabled.
		// That way non-autoscaling plots don't fire in a bunch of non-required signals.
		
		// Apply transforms as needed
		placeSeries(newSeries);

	}
	
	// Remove a series from a plot:
	bool removeSeries(MPlotSeries* removeMe) {
		if(series_.contains(removeMe)) {
			this->removeItem(removeMe);
			series_.removeAll(removeMe);
			disconnect(removeMe, 0, this, 0);
			return true;
		}
		else
			return false;
	}


	
	// access elements of the canvas:
	MPlotAxis* axisBottom() { return axes_[MPlotAxis::Bottom]; }
	MPlotAxis* axisTop() { return axes_[MPlotAxis::Top]; }
	MPlotAxis* axisLeft() { return axes_[MPlotAxis::Left]; }
	MPlotAxis* axisRight() { return axes_[MPlotAxis::Right]; }
	
	
	// Access properties of the Canvas: (TODO: add to property system)
	QGraphicsRectItem* background() { return background_; }
	
	
	
	// Margins: are set in logical coordinates (ie: as a percentage of the chart width or chart height);
	double margin(MPlotAxis::AxisID margin) const { return margins_[margin]; }
	
	double marginLeft() const { return margins_[MPlotAxis::Left]; }
	double marginRight() const { return margins_[MPlotAxis::Right]; }
	double marginTop() const { return margins_[MPlotAxis::Top]; }
	double marginBottom() const { return margins_[MPlotAxis::Bottom]; }
	
	void setMargin(MPlotAxis::AxisID margin, double value) { margins_[margin] = value; placeAxes(); }
	
	void setMarginLeft(double value) { setMargin(MPlotAxis::Left, value); }
	void setMarginRight(double value) { setMargin(MPlotAxis::Right, value); }
	void setMarginTop(double value) { setMargin(MPlotAxis::Top, value); }
	void setMarginBottom(double value) { setMargin(MPlotAxis::Bottom, value); }
	
	/*
	double xDataRangeMin() { return xmin_; }
	double xDataRangeMax() { return xmax_; }
	double yDataRangeLeftMin() { return yleftmin_; }
	double yDataRangeRightMin() { return yrightmin_; }
	double yDataRangeLeftMax() { return yleftmax_; }
	double yDataRangeRightMax() { return yrightmax_; }
	 */
	
	void enableAutoScaleBottom(bool autoScaleOn) { if(autoScaleBottomEnabled_ = autoScaleOn) setXDataRange(0, 0, true); }
	void enableAutoScaleLeft(bool autoScaleOn) { if(autoScaleLeftEnabled_ = autoScaleOn) setYDataRangeLeft(0, 0, true); }
	void enableAutoScaleRight(bool autoScaleOn) { if(autoScaleRightEnabled_ = autoScaleOn) setYDataRangeRight(0, 0, true);}
	void enableAutoScale(int axisFlags) {
		enableAutoScaleBottom(axisFlags & MPlotAxis::Bottom); 
		enableAutoScaleLeft(axisFlags & MPlotAxis::Left); 
		enableAutoScaleRight(axisFlags & MPlotAxis::Right);
	}
	
	// Call this to change the aspect ratio of the scene to match the view (ie: so that text appears normal-height when the view is rectangular
	void setAspectRatio(double ar) {	// Margins stay scaled as percentage of scene size.
		if(ar_ != ar) {					// either use x/ar or y*ar (depending on ar>1 or ar<1) so that we always grow or maintain the scene size for either axis... never shrink it below (-0.5...0.5)
			ar_ = ar;
			if(ar_<1)
				setSceneRect(0, 0, (SCENESIZE/ar_), (SCENESIZE));
			else
				setSceneRect(0, 0, (SCENESIZE), (SCENESIZE*ar_));
			
			// Recalculate positions of: background, plotArea, axes, series...
			placeComponents();
		}
	}
	
	void setScalePadding(double percent) { scalePadding_ = percent/100; placeComponents(); }
	double scalePadding() { return scalePadding_ * 100; }
	
	void setXDataRange(double min, double max, bool autoscale = false) {
		
		setXDataRangeImp(min, max, autoscale);
		
		// We have new transforms.  Need to apply them to all series:
		foreach(MPlotSeries* series, series_) {
			placeSeries(series);
		}
		
	}
	
	void setYDataRangeLeft(double min, double max, bool autoscale = false) {
		
		setYDataRangeLeftImp(min, max, autoscale);
		
		// We have new transforms.  Need to apply them:
		foreach(MPlotSeries* series, series_) {
			placeSeries(series);
		}
	}
	
	void setYDataRangeRight(double min, double max, bool autoscale = false) {
		
		setYDataRangeRightImp(min, max, autoscale);
		
		// Apply new transforms:
		foreach(MPlotSeries* series, series_) {
			placeSeries(series);
		}
	}
	
public slots:
	// These are used to draw a highlight when a series is selected:
	void onSeriesSelected(MPlotSeries* newSeries) {
		highlightSeries_->setModel(newSeries->model());
		highlightSeries_->setYAxisTarget(newSeries->yAxisTarget());
		highlightSeries_->setVisible(true);
	}
	
	void onDeselected() {
		highlightSeries_->setVisible(false);
	}	// TODO: determine what happens if someone deletes a plot series while it is highlighted...
		// todo: also determine what happens for memory management when removing plotSeries from a plot.
	
protected slots:
	
	// This is called when a series updates it's data.  We may have to autoscale/rescale:
	void onDataChanged(MPlotSeries* series) {
		
		if(autoScaleBottomEnabled_)
			setXDataRangeImp(0, 0, true);
		
		if(autoScaleLeftEnabled_ && series->yAxisTarget() == MPlotAxis::Left)
			setYDataRangeLeftImp(0, 0, true);
		
		if(autoScaleRightEnabled_ && series->yAxisTarget() == MPlotAxis::Right)
			setYDataRangeRightImp(0, 0, true);
		
		// We have new transforms.  Need to apply them:
		if(autoScaleBottomEnabled_ | autoScaleLeftEnabled_ | autoScaleRightEnabled_)
			foreach(MPlotSeries* series, series_)
				placeSeries(series);
		
		// Possible optimizations:
		/*
		 - COMPLETED: setXDataRange() and setYDataRange____() both call placeSeries(), which applies the transform to all series (expensive).
		   We could only call this once only (if we knew that both setXDataRange() and setYDataRangeLeft() were going to be called.)
		 
		 - set_DataRange___(0, 0, true) currently loops through all series (see above).
		   If we stored the combined QRectF bounds (for all series),
		   we could just subtract this series bounds (nope.. it's changed... don't have old) and |= on the new one.
		 */
	}
	

	
protected:
	// Members:
	MPlotLegend* legend_;
	MPlotAxis* axes_[9];		// We only use [1], [2], [4], and [8]...
	QList<MPlotSeries*> series_;	// list of current series displayed on plot
	MPlotSeries* highlightSeries_;	// used to draw the highlight on the selected plot.
	
	double ar_;	// Aspect ratio: scene height = width * ar_;
	
	double margins_[9];			// We only use [1], [2], [4], and [8]...
	
	MPlotBackground* background_;
	QGraphicsRectItem* plotArea_;
	
	bool autoScaleBottomEnabled_;
	bool autoScaleLeftEnabled_;
	bool autoScaleRightEnabled_;
	
	QTransform plotAreaTransform_;
	
	// Data ranges:
	double xmin_, xmax_, yleftmin_, yleftmax_, yrightmin_, yrightmax_;
	QTransform leftAxisTransform_, rightAxisTransform_;
	
	double scalePadding_;
	
	// Helper functions:
	// Returns the rectangle (topLeft corner, width, and height) of the plot area, in "scene" (aka logical) coord.
	QRectF plotAreaSceneCoord() {
		if(ar_<1)
			return QRectF(	(marginLeft())/ar_, 
					  (marginTop()), 
					  (SCENESIZE-marginLeft()-marginRight())/ar_, 
					  (SCENESIZE-marginTop()-marginBottom()) );
		else
			return QRectF(	(marginLeft()), 
						  (marginTop())*ar_, 
						  (SCENESIZE-marginLeft()-marginRight()), 
						  (SCENESIZE-marginTop()-marginBottom())*ar_ );
	}
	
	// Call this when the scene coordinates have changed, to re-place everything correctly: 1TODO -- needed if we stabilized the plot area coord?
	void placeComponents() {
		
		QRectF plotAreaSCoords = plotAreaSceneCoord();
		
		background_->setRect(sceneRect());
		
		// This transform is applied to the plotArea_ to it occupy the correct amount of the scene.
		// It now believes to be drawing itself in a cartesian (right-handed) 0,0 -> 1,1 box.
		plotAreaTransform_ = QTransform::fromTranslate(plotAreaSCoords.left(), plotAreaSCoords.bottom()).scale(plotAreaSCoords.width(), -plotAreaSCoords.height());
		plotArea_->setTransform(plotAreaTransform_);
		
		// Rather than transforming the axes, we draw them in the scene coord. to keep the ticks and text in our undistorted 0->100% "logical" coord. system
		placeAxes();

	}
	
	
	// Adjust the positioning of the axes, in scene coord: (ex: use when the margins are changed)
	void placeAxes() {
		
		axes_[MPlotAxis::Bottom]->setPos( plotAreaSceneCoord().bottomLeft() );
		axes_[MPlotAxis::Bottom]->setExtent(plotAreaSceneCoord().width());
		
		axes_[MPlotAxis::Top]->setPos( plotAreaSceneCoord().topLeft());
		axes_[MPlotAxis::Top]->setExtent(plotAreaSceneCoord().width());
		
		axes_[MPlotAxis::Left]->setPos( plotAreaSceneCoord().bottomLeft() );
		axes_[MPlotAxis::Left]->setExtent(plotAreaSceneCoord().height());
		
		axes_[MPlotAxis::Right]->setPos( plotAreaSceneCoord().bottomRight() );
		axes_[MPlotAxis::Right]->setExtent(plotAreaSceneCoord().height());
		
	}
	
	
	// Applies the leftAxis or rightAxis transformation matrix (depending on the 
	void placeSeries(MPlotSeries* series) {
		if(series->yAxisTarget() == MPlotAxis::Right) {
			series->setTransform(rightAxisTransform_);
		}
		else {
			series->setTransform(leftAxisTransform_);
		}
	}
	
		
	/// Drawing defaults:
	
	void setDefaults() {
		
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
		
		// Starting data ranges:
		setYDataRangeLeft(0, 10);
		setYDataRangeRight(0, 10);
		setXDataRange(0, 10);
		
		enableAutoScale(0);	// autoscaling disabled on all axes
		
	}
	
	
	// These implementations leave out the loop that applies the new transforms to all the series.
	// If this happens to be expensive, then internally we can just do that loop once after a combination of x- and y-scaling
	// (Cuts down on dual x- y- autoscale time)
	void setXDataRangeImp(double min, double max, bool autoscale = false) {
		
		// Autoscale?
		if(autoscale) {
			
			QRectF bounds; 
			foreach(MPlotSeries* series, series_) {
				bounds |= series->boundingRect();
			}
			if(bounds.isValid()) {
				min = bounds.left();
				max = bounds.right();
			}
			else
				return;	// no series found... Autoscale does nothing.
			
		}
		
		xmin_ = min; xmax_ = max;
		
		double padding = (max-min)*scalePadding_;
		min -= padding; max += padding;
		
		// Transforms: m31 is x-translate. m32 is y-translate. m11 is x-scale; m22 is y-scale. m21=m12=0 (no shear) m33 = 1 (no affine scaling)
		double yscale, ytranslate, xscale, xtranslate;
		yscale = leftAxisTransform_.m22();
		ytranslate = leftAxisTransform_.m32();
		xscale = 1.0/(max-min);
		xtranslate = -min*xscale;	// TODO: Why do I need to divide by xscale here... but for setYDataRange I don't need to divide by yscale?
		
		leftAxisTransform_.setMatrix(xscale, 0, 0, 0, yscale, 0, xtranslate, ytranslate, 1);
		
		yscale = rightAxisTransform_.m22();
		ytranslate = rightAxisTransform_.m32();
		
		rightAxisTransform_.setMatrix(xscale, 0, 0, 0, yscale, 0, xtranslate, ytranslate, 1);
		
		axes_[MPlotAxis::Bottom]->setRange(min, max);
		axes_[MPlotAxis::Top]->setRange(min, max);
	}
	
	void setYDataRangeLeftImp(double min, double max, bool autoscale = false) {
		// Autoscale?
		if(autoscale) {
			
			QRectF bounds; 
			foreach(MPlotSeries* series, series_) {
				if(series->yAxisTarget() == MPlotAxis::Left)
					bounds |= series->boundingRect();
			}
			if(bounds.isValid()) {
				min = bounds.top();
				max = bounds.bottom();
			}
			else
				return;	// no series found... Autoscale does nothing.
		}
		
		yleftmin_ = min; yleftmax_ = max;
		
		double padding = (max-min)*scalePadding_;
		min -= padding; max += padding;
		
		double xscale, xtranslate, yscale, ytranslate;
		xscale = leftAxisTransform_.m11();
		xtranslate = leftAxisTransform_.m31();
		yscale = 1.0/(max-min);
		ytranslate = -min*yscale;
		
		leftAxisTransform_.setMatrix(xscale, 0, 0, 0, yscale, 0, xtranslate, ytranslate, 1);
		
		axes_[MPlotAxis::Left]->setRange(min, max);
	}
	
	void setYDataRangeRightImp(double min, double max, bool autoscale = false) {
		
		// Autoscale?
		if(autoscale) {
			
			QRectF bounds; 
			foreach(MPlotSeries* series, series_) {
				if(series->yAxisTarget() == MPlotAxis::Right)
					bounds |= series->boundingRect();
			}
			if(bounds.isValid()) {
				min = bounds.top();
				max = bounds.bottom();
			}
			else
				return;	// no series found... Autoscale does nothing.
		}
		
		yrightmin_ = min; yrightmax_ = max;
		
		double padding = (max-min)*scalePadding_;
		min -= padding; max += padding;
		
		double xscale, xtranslate, yscale, ytranslate;
		xscale = rightAxisTransform_.m11();
		xtranslate = rightAxisTransform_.m31();
		yscale = 1.0/(max-min);
		ytranslate = -min*yscale;
		
		rightAxisTransform_.setMatrix(xscale, 0, 0, 0, yscale, 0, xtranslate, ytranslate, 1);
		
		axes_[MPlotAxis::Right]->setRange(min, max);
	}
	

};

#endif
