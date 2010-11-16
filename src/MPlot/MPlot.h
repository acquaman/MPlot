#ifndef __MPlot_H__
#define __MPlot_H__

#include "MPlotAxis.h"
#include "MPlotLegend.h"
#include "MPlotItem.h"
#include "MPlotSeries.h"
#include "MPlotAbstractTool.h"


#include <QList>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsRectItem>

#include <float.h>

#include <QDebug>

/// Defines the minimum distance between min- and max- values for the range of an axis. Without this check, calling setXDataRange(3, 3) or set___DataRange(f, g=f) will cause a segfault within Qt's drawing functions... it can't handle a clipPath with a width of 0.
#define MPLOT_MIN_AXIS_RANGE 1e-60


class MPlot;

/// This class handles signals as a proxy for MPlot.  You should never need to use this class directly.
/*! To avoid restrictions on multipler inheritance, MPlot does not inherit QObject.  Still, it needs a way to respond to events from MPlotItems (such as re-scale and selected events).  This QObject receives signals from MPlotItem and calls the appropriate functions within MPlot.
  */
class MPlotSignalHandler : public QObject {
	Q_OBJECT
protected:
	MPlotSignalHandler(MPlot* parent);
	friend class MPlot;

protected slots:
	void onBoundsChanged();
	void onSelectedChanged(bool);

	void doDelayedAutoscale();

	void onPlotItemLegendContentChanged();

protected:
	MPlot* plot_;
};

/// This class provides plotting capabilities within a QGraphicsItem that can be added to any QGraphicsScene.  It can plot various types of geometric items, including 1D (x-y) series (MPlotAbstractSeries) and 2D images (MPlotAbstractImage).
/*!

  To add an item to a plot, simply create the plot, create the item, and then call MPlot::addItem().  Once added, items become children of the plot, and will be deleted when the plot is deleted.  Deleting an item automatically removes it from the plot.  To remove an item from a plot <i>without</i> deleting it, you can call MPlot::removeItem().

<b> Axes and Axes Ranges</b>
MPlot supports two independent (left and right) y-axes.  Whether an item is plotted on the right or on the left y-axis depends on its MPlotItem::yAxisTarget().

\todo (When changing the axis target on a plot item, this should probably trigger a re-autoscale and a re-application of the waterfall offsets. It currently doesn't, because the change isn't signalled to MPlot.)

The axis ranges for all three axes can be set manually using setXDataRange(), setYDataRangeLeft(), and setYDataRangeRight().  You can also specify an 'autoscale' flag for these functions, which will conduct a one-time fit of the axis range to match the extent of all the items currently plotted on that axis.  If you want the axes to continue auto-scaling in realtime as the data within them changes, use enableAutoScaleBottom(), enableAutoScaleLeft(), and enableAutoScaleRight().

\note Leaving auto-scaling enabled requires more CPU resources, especially for large datasets. Several approaches are used to optimize this, including deferring computation of the new range limits until absolutely necessary.  This can allow the plot data to change several times for a single autoscale recomputation, which is done right before re-drawing the plot.  If you need the autoscale to occur immediately (for example, when using MPlot outside of a Qt event loop, or doing off-screen rendering), you can call doDelayedAutoScale().

  <b>Transformations</b>

  Beyond auto-scaling, MPlot offers convenience functions to apply transformations to the items within the plots.  You can use enableAxisNormalizationBottom(), enableAxisNormalizationLeft(), and enableAxisNormalizationRight() to keep all MPlotSeries items always scaled within a given range. (This is useful, for example, when wanting to comparing several series of very different magnitudes on the same plot.  Note that this mode is merely a convenient way to automatically enable normalization for all current and future MPlotAbstractSeries added to the plot; alternatively, you can configure MPlotAbstractSeries::enableYAxisNormalization() / MPlotAbstractSeries::enableXAxisNormalization() individually for each series.)

  Another convenience transformation is provided by setWaterfallLeft() and setWaterfalRight(), which will stagger plot series items by a constant vertical offset.  This is most useful when combined with enableAxisNormalizationLeft()/Right(), since in this case all the series will share a common vertical scale.  (Again, this mode is just a convenient way to call MPlotAbstractSeries::setOffset() on all of the current and future plot series items -- You can always configure each series individually for finer control.)  To disable the waterfall effect, simply call setWaterfallLeft(0) / setWaterfallRight(0).

  <b>Look and Feel</b>
  The look and feel of plots is configured by accessing the individual components of the plot and setting their properties (for example, setting pens, brushes and fonts using the usual Qt function calls). These components include:

  - The plot area: QGraphicsRectItem* plotArea(); (Plot area color/brush)
  - The background: QGraphicsRectItem* background(); (Background color/brush)
  - The axes: MPlotAxis* axisBottom(), axisTop(), axisLeft(), axisRight();  (Ticks, labels, axis names and fonts, gridlines, etc.)
  - The legend: MPlotLegend* legend();  (Title and description text, alignment, whether to show legend entries for each plot item, etc.)

  The margin sizes are configured directly (in percent of the total plot size) using setMargin() or setMarginLeft(), setMarginBottom(), etc.

  To set up your favorite look-and feel for a commonly-used plot style, you can subclass MPlot and re-implement setDefaults().

  <b> Plot Tools and Interaction</b>
  In addition to adding plot items with addItem(), various kinds of interactive tools can be added to a plot using addTool().  These tools include cursors, zoom tools, and item selection tools:
  - MPlotPlotSelectorTool: select an item on the plot by clicking
  - MPlotWheelZoomerTool: zoom in and out on the mouse pointer location using the scroll wheel. (Similar to CAD program zoom navigation)
  - MPlotDragZoomerTool: zoom in to a selected area by clicking and dragging a rubber band.
  - MPlotCursorTool: place one or more cursors on the plot, and read their locations.
  */

class MPlot : public QGraphicsItem {

public:
	MPlot(QRectF rect = QRectF(0,0,100,100), QGraphicsItem* parent = 0);
	virtual ~MPlot();
	/// Required paint function. (All painting is done by children)
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	virtual QRectF boundingRect() const;

	/// Use this to append a new data-item to a plot:
	void addItem(MPlotItem* newItem) { insertItem(newItem, -1); }
	/// Use this to insert a new data-item into the plot, at a given index. \c index must be between 0 and numItems(), or -1 to append.
	void insertItem(MPlotItem* newItem, int index = -1);
	/// Remove a data-item from a plot. (Note: Does not delete the item...)
	bool removeItem(MPlotItem* removeMe);


	/// Returns the number of items currently displayed in the plot:
	int numItems() const { return items_.count(); }
	/// Returns one of the plot items, by index:
	MPlotItem* item(int index) const { if(index>=0 && index<items_.count()) return items_.at(index); else return 0; }
	/// Returns all the plot items in this plot
	QList<MPlotItem*> plotItems() const { return items_; }


	/// Add a tool to the plot:
	void addTool(MPlotAbstractTool* newTool);

	/// Remove a tool from a plot. (Note: Does not delete the tool...)
	bool removeTool(MPlotAbstractTool* removeMe);

	QGraphicsRectItem* plotArea() const { return plotArea_; }
	// access elements of the canvas:

	MPlotAxis* axisBottom() { return axes_[MPlotAxis::Bottom]; }

	MPlotAxis* axisTop() { return axes_[MPlotAxis::Top]; }

	MPlotAxis* axisLeft() { return axes_[MPlotAxis::Left]; }

	MPlotAxis* axisRight() { return axes_[MPlotAxis::Right]; }

	MPlotLegend* legend() { return legend_; }

	QGraphicsRectItem* background() { return background_; }

	/// returns the rectangle filled by this plot (in scene or parent QGraphicsItem coordinates)
	QRectF rect() const { return rect_; }

	/// Sets the rectangle to be filled by this plot (in scene or parent QGraphicsItem coordinates).
	/*! Also rescales and re-applies the margins and transform for the plotArea). Can call with setRect(rect()) to re-compute margins.)*/
	void setRect(const QRectF& rect);

	// Margins: are set in logical coordinates (ie: as a percentage of the chart width or chart height);
	double margin(MPlotAxis::AxisID margin) const {
		return margins_[margin];
	}

	double marginLeft() const {
		return margins_[MPlotAxis::Left];
	}

	double marginRight() const {
		return margins_[MPlotAxis::Right];
	}

	double marginTop() const {
		return margins_[MPlotAxis::Top];
	}

	double marginBottom() const {
		return margins_[MPlotAxis::Bottom];
	}

	void setMargin(MPlotAxis::AxisID margin, double value) {
		margins_[margin] = value; setRect(rect_);
	}

	void setMarginLeft(double value) {
		setMargin(MPlotAxis::Left, value);
	}

	void setMarginRight(double value) {
		setMargin(MPlotAxis::Right, value);
	}

	void setMarginTop(double value) {
		setMargin(MPlotAxis::Top, value);
	}

	void setMarginBottom(double value) {
		setMargin(MPlotAxis::Bottom, value);
	}


	double xMin() {
		return xmin_;
	}

	double xMax() {
		return xmax_;
	}

	double yLeftMin() {
		return yleftmin_;
	}

	double yRightMin() {
		return yrightmin_;
	}

	double yLeftMax() {
		return yleftmax_;
	}

	double yRightMax() {
		return yrightmax_;
	}

	QTransform leftAxisTransform() {
		return leftAxisTransform_;
	}

	QTransform rightAxisTransform() {
		return rightAxisTransform_;
	}


	void enableAutoScaleBottom(bool autoScaleOn);
	void enableAutoScaleLeft(bool autoScaleOn);
	void enableAutoScaleRight(bool autoScaleOn);
	void enableAutoScale(int axisFlags);


	void enableAxisNormalizationBottom(bool normalizationOn, double min = 0, double max = 1);
	void enableAxisNormalizationLeft(bool normalizationOn, double min = 0, double max = 1);
	void enableAxisNormalizationRight(bool normalizationOn, double min = 0, double max = 1);
	void enableAxisNormalization(int axisFlags);

	void setWaterfallLeft(double amount = 0.2);
	void setWaterfallRight(double amount = 0.2);

	void setScalePadding(double percent);

	double scalePadding();

	void setXDataRange(double min, double max, bool autoscale = false, bool applyPadding = true);

	void setYDataRangeLeft(double min, double max, bool autoscale = false, bool applyPadding = true);

	void setYDataRangeRight(double min, double max, bool autoscale = false, bool applyPadding = true);

	/// called automatically when control returns to the event loop, this completes a delayed autoscale. (Recomputing the scale limits is optimized to be only done when necessary, rather than whenever the data values change.)  If you need the scene to be updated NOW! (for example, you're working outside of an event loop, or rendering before returning to the event loop), you can call this manually.
	void doDelayedAutoScale();


protected: // "slots" (proxied through MPlotSignalHandler)
	/// called when the x-y data in a plot item might have changed, such that a re-autoscale is necessary
	void onBoundsChanged(MPlotItem* source);
	/// called when the selected state of a plot item changes
	void onSelectedChanged(MPlotItem* source, bool isSelected);
	/// called when the legend content (color, description, etc.) of a plot item changes
	void onPlotItemLegendContentChanged(MPlotItem* changedItem);




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
	/// The rectangle containing the plotting area, in scene coordinates. (plotArea_ and dataArea_ are scaled so that their local coordinates are from (0,0) to (1,1) instead.)
	QRectF plotAreaRect_;

	bool autoScaleBottomEnabled_, autoScaleLeftEnabled_, autoScaleRightEnabled_;

	bool normBottomEnabled_, normLeftEnabled_, normRightEnabled_;
	QPair<double, double> normBottomRange_, normLeftRange_, normRightRange_;

	double waterfallLeftAmount_, waterfallRightAmount_;

	/// Caching/optimization: counts the number of MPlotAbstractSeries plotted on the left and right axes
	mutable int seriesCounterLeft_, seriesCounterRight_;

	/// Indicates that a re-autoscale has been scheduled (Actually doing it is deferred until returning back to the event loop)
	bool autoScaleScheduled_;
	/// Or-combination of MPlotAxis::AxisId flags, indicating which axes need to be auto-scaled.
	int axesNeedingAutoScale_;
	/// Request a deferred auto-scale:
	void scheduleDelayedAutoScale();

	/// Normally, when plot items are removed, they can trigger a re-autoscale. This is expensive if the MPlot is just about to be deleted anyway, and we have a lot of plots. This optimization omits this useless process and speeds up the destructor.
	bool gettingDeleted_;



	// Data ranges: (specify axis range __prior to padding__)
	double xmin_, xmax_, yleftmin_, yleftmax_, yrightmin_, yrightmax_;
	QTransform leftAxisTransform_, rightAxisTransform_;

	double scalePadding_;

	MPlotSignalHandler* signalHandler_;
	friend class MPlotSignalHandler;

	/// Applies the leftAxis or rightAxis transformation matrix (depending on the yAxis target)
	void placeItem(MPlotItem* theItem);

	/// Sets the defaults for the drawing options: margins, scale padding, background colors, initial data range.
	void setDefaults();

	// These implementations leave out the loop that applies the new transforms to all the items.
	// If this happens to be expensive, then internally we can just do that loop once after a combination of x- and y-scaling
	// (Cuts down on dual x- y- autoscale time)
	void setXDataRangeImp(double min, double max, bool autoscale = false, bool applyPadding = true);

	void setYDataRangeLeftImp(double min, double max, bool autoscale = false, bool applyPadding = true);

	void setYDataRangeRightImp(double min, double max, bool autoscale = false, bool applyPadding = true);
};

#include <QGraphicsWidget>
#include <QGraphicsSceneResizeEvent>

/// this class is used instead of MPlot when you need a QGraphicsWidget (instead of a simple QGraphicsItem).
class MPlotGW : public QGraphicsWidget {
	Q_OBJECT
public:
	MPlotGW(QGraphicsItem* parent = 0, Qt::WindowFlags flags = 0);
	virtual ~MPlotGW();

	MPlot* plot() const;



protected:
	MPlot* plot_;
	virtual void resizeEvent ( QGraphicsSceneResizeEvent * event );
};

#endif
