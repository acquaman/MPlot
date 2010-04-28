#ifndef __MPlot_H__
#define __MPlot_H__

#include "MPlotAxis.h"
#include "MPlotLegend.h"
#include "MPlotItem.h"
#include "MPlotSeries.h"
#include "MPlotAbstractTool.h"

#include <QList>
#include <QGraphicsObject>
#include <QGraphicsScene>
#include <QGraphicsRectItem>

#include <float.h>

/// Defines the minimum distance between min- and max- values for the range of an axis. Without this check, calling setXDataRange(3, 3) or set___DataRange(f, g=f) will cause a segfault within Qt's drawing functions... it can't handle a clipPath with a width of 0.
#define MPLOT_MIN_AXIS_RANGE DBL_EPSILON

/// This class provides plotting capabilities within a QGraphicsItem that can be added to any QGraphicsScene,
class MPlot : public QGraphicsObject {
	Q_OBJECT

public:
	MPlot(QRectF rect = QRectF(0,0,100,100), QGraphicsItem* parent = 0) :
		QGraphicsObject(parent),
		rect_(rect) {

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

	}

	/// Required paint function. (All painting is done by children)
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0) {
		Q_UNUSED(painter)
		Q_UNUSED(option)
		Q_UNUSED(widget)
	}

	virtual QRectF boundingRect() const {
		return rect_;
	}

	/// Use this to add a new data-item to a plot:
	void addItem(MPlotItem* newItem) {
		newItem->setParentItem(dataArea_);
		items_ << newItem;

		connect(newItem, SIGNAL(dataChanged(MPlotItem*)), this, SLOT(onDataChanged(MPlotItem*)));
		// Possible optimization: only connect items to this slot when continuous autoscaling is enabled.
		// That way non-autoscaling plots don't fire in a bunch of non-required signals.

		// Apply transforms as needed
		placeItem(newItem);

	}

	/// Remove a data-item from a plot. (Note: Does not delete the item...)
	bool removeItem(MPlotItem* removeMe) {
		if(items_.contains(removeMe)) {
			removeMe->setParentItem(0);
			if(scene())
				scene()->removeItem(removeMe);
			items_.removeAll(removeMe);
			disconnect(removeMe, 0, this, 0);
			return true;
		}
		else
			return false;
	}

	QList<MPlotItem*> plotItems() const { return items_; }

	/// Add a tool to the plot:
	void addTool(MPlotAbstractTool* newTool) {
		newTool->setParentItem(plotArea_);
		tools_ << newTool;

		// placeTool(newTool);
		newTool->setPlot(this);
	}

	/// Remove a tool from a plot. (Note: Does not delete the tool...)
	bool removeTool(MPlotAbstractTool* removeMe) {
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


	// access elements of the canvas:
	MPlotAxis* axisBottom() { return axes_[MPlotAxis::Bottom]; }
	MPlotAxis* axisTop() { return axes_[MPlotAxis::Top]; }
	MPlotAxis* axisLeft() { return axes_[MPlotAxis::Left]; }
	MPlotAxis* axisRight() { return axes_[MPlotAxis::Right]; }


	// Access properties of the Canvas: (TODO: add to property system)
	QGraphicsRectItem* background() { return background_; }

	/// returns the rectangle filled by this plot (in scene or parent QGraphicsItem coordinates)
	QRectF rect() const { return rect_; }

	/// Sets the rectangle to be filled by this plot (in scene or parent QGraphicsItem coordinates).
	/*! Also rescales and re-applies the margins and transform for the plotArea). Can call with setRect(rect()) to re-compute margins.)*/
	void setRect(const QRectF& rect) {

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
	double margin(MPlotAxis::AxisID margin) const { return margins_[margin]; }

	double marginLeft() const { return margins_[MPlotAxis::Left]; }
	double marginRight() const { return margins_[MPlotAxis::Right]; }
	double marginTop() const { return margins_[MPlotAxis::Top]; }
	double marginBottom() const { return margins_[MPlotAxis::Bottom]; }

	void setMargin(MPlotAxis::AxisID margin, double value) { margins_[margin] = value; setRect(rect_); }

	void setMarginLeft(double value) { setMargin(MPlotAxis::Left, value); }
	void setMarginRight(double value) { setMargin(MPlotAxis::Right, value); }
	void setMarginTop(double value) { setMargin(MPlotAxis::Top, value); }
	void setMarginBottom(double value) { setMargin(MPlotAxis::Bottom, value); }


	double xMin() { return xmin_; }
	double xMax() { return xmax_; }
	double yLeftMin() { return yleftmin_; }
	double yRightMin() { return yrightmin_; }
	double yLeftMax() { return yleftmax_; }
	double yRightMax() { return yrightmax_; }
	QTransform leftAxisTransform() { return leftAxisTransform_; }
	QTransform rightAxisTransform() { return rightAxisTransform_; }


	void enableAutoScaleBottom(bool autoScaleOn) { if(autoScaleBottomEnabled_ = autoScaleOn) setXDataRange(0, 0, true); }
	void enableAutoScaleLeft(bool autoScaleOn) { if(autoScaleLeftEnabled_ = autoScaleOn) setYDataRangeLeft(0, 0, true); }
	void enableAutoScaleRight(bool autoScaleOn) { if(autoScaleRightEnabled_ = autoScaleOn) setYDataRangeRight(0, 0, true);}
	void enableAutoScale(int axisFlags) {
		enableAutoScaleBottom(axisFlags & MPlotAxis::Bottom);
		enableAutoScaleLeft(axisFlags & MPlotAxis::Left);
		enableAutoScaleRight(axisFlags & MPlotAxis::Right);
	}



	void setScalePadding(double percent) {
		scalePadding_ = percent/100;
		// re-scale axis if needed:
		setXDataRangeImp(xmin_, xmax_);
		setYDataRangeLeftImp(yleftmin_, yleftmax_);
		setYDataRangeRightImp(yrightmin_, yrightmax_);

		foreach(MPlotItem* item, items_)
			placeItem(item);
	}

	double scalePadding() { return scalePadding_ * 100; }

	void setXDataRange(double min, double max, bool autoscale = false, bool applyPadding = true) {

		setXDataRangeImp(qMin(min, max), qMax(min, max), autoscale, applyPadding);

		// We have new transforms.  Need to apply them to all item:
		foreach(MPlotItem* item, items_) {
			placeItem(item);
		}

	}

	void setYDataRangeLeft(double min, double max, bool autoscale = false, bool applyPadding = true) {

		setYDataRangeLeftImp(qMin(min, max), qMax(min, max), autoscale, applyPadding);

		// We have new transforms.  Need to apply them:
		foreach(MPlotItem* item, items_) {
			placeItem(item);
		}
	}

	void setYDataRangeRight(double min, double max, bool autoscale = false, bool applyPadding = true) {

		setYDataRangeRightImp(qMin(min, max), qMax(min, max), autoscale, applyPadding);

		// Apply new transforms:
		foreach(MPlotItem* item, items_)
			placeItem(item);
	}

public slots:


protected slots:

	// This is called when a item updates it's data.  We may have to autoscale/rescale:
	void onDataChanged(MPlotItem* item1) {

		if(autoScaleBottomEnabled_)
			setXDataRangeImp(0, 0, true);

		if(autoScaleLeftEnabled_ && item1->yAxisTarget() == MPlotAxis::Left)
			setYDataRangeLeftImp(0, 0, true);

		if(autoScaleRightEnabled_ && item1->yAxisTarget() == MPlotAxis::Right)
			setYDataRangeRightImp(0, 0, true);

		// We have new transforms.  Need to apply them:
		if(autoScaleBottomEnabled_ | autoScaleLeftEnabled_ | autoScaleRightEnabled_) {
			foreach(MPlotItem* item, items_)
				placeItem(item);
		}

		// Possible optimizations:
		/*

		 - set_DataRange___(0, 0, true) currently loops through all items (see above).
		   If we stored the combined QRectF bounds (for all series),
		   we could just subtract this item's bounds (nope.. it's changed... don't have old) and |= on the new one.
		 */
	}



protected:
	// Members:
	QRectF rect_;

	MPlotLegend* legend_;
	MPlotAxis* axes_[9];		// We only use [1], [2], [4], and [8]...
	QList<MPlotItem*> items_;	// list of current data items displayed on plot
	QList<MPlotAbstractTool*> tools_;	// list of tools that have been installed on the plot

	double margins_[9];			// We only use [1], [2], [4], and [8]...

	QGraphicsRectItem* background_;
	QGraphicsRectItem* plotArea_, *dataArea_;

	bool autoScaleBottomEnabled_;
	bool autoScaleLeftEnabled_;
	bool autoScaleRightEnabled_;


	// Data ranges: (specify axis range __prior to padding__)
	double xmin_, xmax_, yleftmin_, yleftmax_, yrightmin_, yrightmax_;
	QTransform leftAxisTransform_, rightAxisTransform_;

	double scalePadding_;


	/// Applies the leftAxis or rightAxis transformation matrix (depending on the yAxis target)
	void placeItem(MPlotItem* theItem) {
		if(theItem->yAxisTarget() == MPlotAxis::Right) {
			theItem->setTransform(rightAxisTransform_);
		}
		else {
			theItem->setTransform(leftAxisTransform_);
		}
	}




	/// Sets the defaults for the drawing options: margins, scale padding, background colors, initial data range.
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
	void setXDataRangeImp(double min, double max, bool autoscale = false, bool applyPadding = true) {

		// Autoscale?
		if(autoscale) {

			QRectF bounds;
			foreach(MPlotItem* itm, items_) {
				bounds |= itm->dataRect();
			}
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

	void setYDataRangeLeftImp(double min, double max, bool autoscale = false, bool applyPadding = true) {
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

	void setYDataRangeRightImp(double min, double max, bool autoscale = false, bool applyPadding = true) {

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


};

#endif
