#ifndef __MPlot_H__
#define __MPlot_H__

#include "MPlotAxis.h"
#include "MPlotLegend.h"
#include "MPlotItem.h"
#include "MPlotAbstractTool.h"


#include <QList>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsRectItem>

class MPlot;

/// This class handles signals as a proxy for MPlot.  You should never need to use this class directly.
/*! To avoid restrictions on multiple inheritance, MPlot does not inherit QObject.  Still, it needs a way to respond to events from MPlotItems (such as re-scale and selected events).  This QObject receives signals from MPlotItem and calls the appropriate functions within MPlot.
  */
class MPlotSignalHandler : public QObject {
	Q_OBJECT
protected:
	/// Constructor.  Builds a signal source for the MPlot given by \param parent.
	MPlotSignalHandler(MPlot* parent);
	/// Giving the signal handler access to MPlots protected members.
	friend class MPlot;

protected slots:
	/// Catches the boundsChanged signal and calls MPlot::onBoundsChanged method with with the pointer of whichever MPlotItem emitted the signal.
	void onBoundsChanged();
	/// \note This slot does nothing.  It calls MPlot::onSelectedChanged, but that is an unimplemented class.
	void onSelectedChanged(bool);
	/// Catches the plotItemLegendContentChanged signal and calls MPlot::onPlotItemLegendContentsChanged with the pointer to item that called it.  If the qobject_cast fails, then a 0 is passed.
	void onPlotItemLegendContentChanged();

	/// Catches the doDelayedAutoscale signal.  This calls MPlot::doDelayedAutoscale which causes the plot to update if the any item within the plot has flagged to the plot that the plot needs to be re-autoscaled.
	void doDelayedAutoscale();
	/// This slot flags the plot that it needs to perform an autoscale the next time it returns to the event loop.
	void onAxisScaleAutoScaleEnabledChanged(bool enabled);

signals:
	/// Notifer that the data position has been updated.  Passes the index of item inside the position indicator tool and the new position (in data coordinates).  Only emitted when the MPlotDataPositionTool has been added to the plot.
	void dataPositionChanged(unsigned index, const QPointF &point);

protected:
	/// The instance of MPlot that this signal source is connected to.
	MPlot* plot_;
};

/// This class provides plotting capabilities within a QGraphicsItem that can be added to any QGraphicsScene.  It can plot various types of geometric items, including 1D (x-y) series (MPlotAbstractSeries) and 2D images (MPlotAbstractImage).
/*!

  To add an item to a plot, simply create the plot, create the item, and then call MPlot::addItem().  Once added, items become children of the plot, and will be deleted when the plot is deleted.  Deleting an item automatically removes it from the plot.  To remove an item from a plot <i>without</i> deleting it, you can call MPlot::removeItem().

<b> Axes and Axes Ranges</b>
MPlot supports two independent (left and right) y-axes.  Whether an item is plotted on the right or on the left y-axis depends on its MPlotItem::yAxisTarget().

\todo (When changing the axis target on a plot item, this should probably trigger a re-autoscale and a re-application of the waterfall offsets. It currently doesn't, because the change isn't signalled to MPlot.)

THIS DOCUMENTATION IS OLD. UPDATE NEEDED FOR AXIS SCALING:

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

	/// These enum values can be used as axis scale indexes in axisScale() to retrieve one of the standard axis scales that all plots start with by default.
	enum StandardAxis { Left = 0, Bottom, Right, Top, VerticalRelative, HorizontalRelative };

	MPlot(const QRectF& rect = QRectF(0,0,100,100), QGraphicsItem* parent = 0);
	/// Destructor. Any items within the plot will be deleted as well, because they are QGraphicsItem children of the plot.
	virtual ~MPlot();

	/// Connect to this proxy object to receive MPlotItem signals:
	MPlotSignalHandler* signalSource() const { return signalHandler_; }

	/// Required paint function. (All painting is done by children)
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	virtual QRectF boundingRect() const;

	/// Use this to append a new data-item to a plot:
	void addItem(MPlotItem* newItem, int yAxisTargetIndex = MPlot::Left, int xAxisTargetIndex = MPlot::Bottom) { insertItem(newItem, -1, yAxisTargetIndex, xAxisTargetIndex); }
	/// Use this to insert a new data-item into the plot, at a given index. \c index must be between 0 and numItems(), or -1 to append.
	/*! If no yAxisTarget, xAxisTarget is specified, it will be targetted to the default (left, bottom) axes of this plot. You can call setYAxisTarget() and setXAxisTarget() on the item after adding it to change those afterwards, using axisScale() to get a pointer to the desired axis scale. */
	void insertItem(MPlotItem* newItem, int index = -1, int yAxisTargetIndex = MPlot::Left, int xAxisTargetIndex = MPlot::Bottom);
	/// Remove a data-item from a plot. (Note: Does not delete the item...)
	bool removeItem(MPlotItem* removeMe);

	/// Returns the number of items currently displayed in the plot:
	int numItems() const { return items_.count(); }
	/// Returns one of the plot items, by index:
	MPlotItem* item(int index) const { if(index>=0 && index<items_.count()) return items_.at(index); else return 0; }
	/// Returns all the plot items in this plot
	QList<MPlotItem*> plotItems() const { return items_; }
	/// Counts the number of MPlotAbstractSeries items in this plot.
	int seriesItemsCount() const;
	/// Counts the number of MPlotAbstractImage items in this plot
	int imageItemsCount() const;

	/// Add a tool to the plot:
	void addTool(MPlotAbstractTool* newTool);

	/// Remove a tool from a plot. (Note: Does not delete the tool...)
	bool removeTool(MPlotAbstractTool* removeMe);

	/// Returns the QGraphicsItem that contains the plot area.
	QGraphicsRectItem* plotArea() const { return plotArea_; }
	// access elements of the canvas:

	/// Returns the MPlot axis for the given \param axisIndex.  If the axisIndex is not valid, then 0 is returned.
	MPlotAxis* axis(int axisIndex) { if((unsigned)axisIndex >= (unsigned)axes_.count()) return 0;  return axes_.at(axisIndex); }
	/// Convenience getter for the left MPlotAxis.
	MPlotAxis* axisLeft() { return axes_.at(MPlot::Left); }
	/// Convenience getter for the bottom MPlotAxis.
	MPlotAxis* axisBottom() { return axes_.at(MPlot::Bottom); }
	/// Convenience getter for the right MPlotAxis.
	MPlotAxis* axisRight() { return axes_.at(MPlot::Right); }
	/// Convenience getter for the top MPlotAxis.
	MPlotAxis* axisTop() { return axes_.at(MPlot::Top); }
	/// Returns the axis index from the given MPlotAxis \param axis.
	int indexOfAxis(MPlotAxis* axis) const { return axes_.indexOf(axis); }

	/// Returns a modifiable reference to the MPlotAxisScale from a given \param axisScaleIndex.  If the axisScaleIndex is invalid, then 0 is returned.
	MPlotAxisScale* axisScale(int axisScaleIndex) { if((unsigned)axisScaleIndex >= (unsigned)axisScales_.count()) return 0;  return axisScales_.at(axisScaleIndex); }
	/// Returns a const reference to the MPlotAxisScale from a given \param axisScaleIndex.  If the axisScaleIndex is invalid, then 0 is return.
	const MPlotAxisScale* axisScale(int axisScaleIndex) const { if((unsigned)axisScaleIndex >= (unsigned)axisScales_.count()) return 0;  return axisScales_.at(axisScaleIndex); }
	/// Convenience getter for the left MPlotAxisScale.
	MPlotAxisScale* axisScaleLeft() { return axisScales_.at(MPlot::Left); }
	/// Convenience getter for the bottom MPlotAxisScale.
	MPlotAxisScale* axisScaleBottom() { return axisScales_.at(MPlot::Bottom); }
	/// Convenience getter for the right MPlotAxisScale.
	MPlotAxisScale* axisScaleRight() { return axisScales_.at(MPlot::Right); }
	/// Convenience getter for the top MPlotAxisScale.
	MPlotAxisScale* axisScaleTop() { return axisScales_.at(MPlot::Top); }
	/// Convenience getter for the relative horizontal MPlotAxisScale.  This MPlotAxisScale is used when plotting items that do not depend explicitly on the data contained within the plot.
	MPlotAxisScale* axisScaleHorizontalRelative() { return axisScales_.at(MPlot::HorizontalRelative); }
	/// Convenience getter for the relative vertical MPlotAxisScale.  This MPlotAxisScale is used when plotting items that do not depend explicitly on the data contained within the plot.
	MPlotAxisScale* axisScaleVerticalRelative() { return axisScales_.at(MPlot::VerticalRelative); }
	/// Returns the axis scale index from the given MPlotAxisScale \param axisScale.
	int indexOfAxisScale(const MPlotAxisScale* axisScale) const { return axisScales_.indexOf(const_cast<MPlotAxisScale*>(axisScale)); }
	/// Method that adds another MPlotAxisScale to the plot.  It is appended to the end of the axis scales list.
	void addAxisScale(MPlotAxisScale* newScale);

	/// Returns the legend for this MPlot.
	MPlotLegend* legend() { return legend_; }
	/// Returns the QGraphicsItem that contains the background information.
	QGraphicsRectItem* background() { return background_; }

	/// Returns the rectangle filled by this plot (in scene or parent QGraphicsItem coordinates)
	QRectF rect() const { return rect_; }

	/// Sets the rectangle to be filled by this plot (in scene or parent QGraphicsItem coordinates).
	/*! Also rescales and re-applies the margins and places the plotArea). Can call with setRect(rect()) to re-compute margins.)*/
	void setRect(const QRectF& rect);

	// Margins: are set in logical coordinates (ie: as a percentage of the chart width or chart height);
	/// Returns the margin (as a percentage) for the given MPlot::StandardAxis \param which.
	qreal margin(MPlot::StandardAxis which) const { return margins_[(int)which]; }
	/// Convenience getter for the left axis margin (expressed as a percentage).
	qreal marginLeft() const { return margins_[MPlot::Left]; }
	/// Convenience getter for the right axis margin (expressed as a percentage).
	qreal marginRight() const { return margins_[MPlot::Right]; }
	/// Convenience getter for the top axis margin (expressed as a percentage).
	qreal marginTop() const { return margins_[MPlot::Top]; }
	/// Convenience getter for the bottom axis margin (expressed as a percentage).
	qreal marginBottom() const { return margins_[MPlot::Bottom]; }

	/// Sets the margin for MPlot::StandardAxis \param which with value \param value.  Must be given as a percentage.
	void setMargin(MPlot::StandardAxis which, qreal value) { margins_[(int)which] = value; setRect(rect_); }
	/// Sets the left margin to the given \param value.
	void setMarginLeft(qreal value) { setMargin(MPlot::Left, value); }
	/// Sets the right margin to the given \param value.
	void setMarginRight(qreal value) { setMargin(MPlot::Right, value); }
	/// Sets the top margin to the given \param value.
	void setMarginTop(qreal value) { setMargin(MPlot::Top, value); }
	/// Sets the bottom margin to the given \param value.
	void setMarginBottom(qreal value) { setMargin(MPlot::Bottom, value); }

	/// Method that enables/disables the logarithmic scale for the give MPlotAxisScale \param axisScaleIndex.
	void enableLogScale(int axisScaleIndex, bool logScaleOn);
	/// Method that enables/disables axis normalization for the given MPlotAxisScale \param axisScaleIndex between the given MPlotAxisRange \param normalizationRange.  If no range is given, then the standard of 0 to 1 is used.
	void enableAxisNormalization(int axisScaleIndex, bool normalizationOn, const MPlotAxisRange& normalizationRange = MPlotAxisRange(0,1));
	/// Convenience method to enable/disable axis normalization for the given MPlotAxisScale \param axisScaleIndex by explicitly giving a \param min and \param max.
	void enableAxisNormalization(int axisScaleIndex, bool normalizationOn, qreal min, qreal max) {
		enableAxisNormalization(axisScaleIndex, normalizationOn, MPlotAxisRange(min, max));
	}

	/// Sets a waterfall amount to be applied to the given \param axisScaleIndex.  The \param amount defaults to 0.2 because it assumes that normalization has been enabled for the given MPlotAxisScale.  However, any amount that is within the MPlotAxisRange range is an acceptable value.
	void setAxisScaleWaterfall(int axisScaleIndex, qreal amount = 0.2);

	/// Called automatically when control returns to the event loop, this completes a delayed autoscale. (Recomputing the scale limits is optimized to be only done when necessary, rather than whenever the data values change.)  If you need the scene to be updated NOW! (for example, you're working outside of an event loop, or rendering before returning to the event loop), you can call this manually.
	void doDelayedAutoScale();

	/// Retrieves the overall minimum x-value from all the series contained within the plot.  If there are no series contained in the plot then MPLOT_NEG_INFINITY is returned.
	double minimumXSeriesValue();
	/// Retrieves the overall maximum x-value from all the series contained within the plot.  If there are no series contained in the plot then MPLOT_POS_INFINITY is returned.
	double maximumXSeriesValue();

	/// Retrieves the overall minimum y-value from all the series contained within the plot.  If there are no series contained in the plot then MPLOT_NEG_INFINITY is returned.
	double minimumYSeriesValue();
	/// Retrieves the overall maximum y-value from all the series contained within the plot.  If there are no series contained in the plot then MPLOT_POS_INFINITY is returned.
	double maximumYSeriesValue();

protected: // "slots" (proxied through MPlotSignalHandler)
	/// Called when the x-y data in a plot item might have changed, such that a re-autoscale is necessary.
	void onBoundsChanged(MPlotItem* source);
	/// Called when the selected state of a plot item changes.
	void onSelectedChanged(MPlotItem* source, bool isSelected);
	/// Called when the legend content (color, description, etc.) of a plot item changes.
	void onPlotItemLegendContentChanged(MPlotItem* changedItem);
	/// Called when the autoscaling of an axis scale changes.
	void onAxisScaleAutoScaleEnabledChanged(bool autoScaleEnabled);

protected:
	/// Request a deferred auto-scale:
	void scheduleDelayedAutoScale();
	/// Sets the defaults for the drawing options: margins, scale padding, background colors, initial data range.
	void setDefaults();

	// Members:
	/// The bounding rectangle for the MPlot.
	QRectF rect_;

	/// The reference to the MPlotLegend contained in this MPlot.
	MPlotLegend* legend_;

	/// The list of the MPlotAxes.  0 through 3 are provided by default (MPlot::Left, MPlot::Bottom, MPlot::Right, MPlot::Top).
	QList<MPlotAxis*> axes_;
	/// The list of the MPlotAxisScales.  0 through 5 are provided by default (MPlot::Left, MPlot::Bottom, MPlot::Right, MPlot::Top, MPlot::HorizontalRelative, MPlot::VerticalRelative).
	QList<MPlotAxisScale*> axisScales_;
	/// The list of the waterfall amounts for the different axis scales.  The amount for a given axis scale is indexed by the axisScaleIndex.
	QList<qreal> axisScaleWaterfallAmount_;

	/// The list of the MPlotAxisScales that have log scale enabled.  The state is indexed by the axisScaleIndex.
	QList<bool> axisScaleLogScaleOn_;

	/// The list of which MPlotAxisScales are normalized.  The state is indexed by the axisScaleIndex.
	QList<bool> axisScaleNormalizationOn_;
	/// The list of the MPlotAxisRanges for normalization.  The range is indexed by the axisScaleIndex.
	QList<MPlotAxisRange> axisScaleNormalizationRange_;

	/// The list of all the MPlotItems contained in the plot.
	QList<MPlotItem*> items_;
	/// The list of all the MPlotAbstractTools that are currently being used in the plot.
	QList<MPlotAbstractTool*> tools_;

	/// The margins for the left, bottom, top, and right axis.
	qreal margins_[4];			// left, bottom, right, top.

	/// Reference to the QGraphicsRectItem which contains the background that can be easily added to QGraphicsScences.
	QGraphicsRectItem* background_;
	/// References to the QGraphicsRectItem for both the plot area and the data area.  These are used for easily adding to QGraphicsScences.
	QGraphicsRectItem* plotArea_, *dataArea_;
	/// The rectangle containing the plotting area, in scene coordinates.
	QRectF plotAreaRect_;


	/// Indicates that a re-autoscale has been scheduled (Actually doing it is deferred until returning back to the event loop)
	bool autoScaleScheduled_;

	/// Normally, when plot items are removed, they can trigger a re-autoscale. This is expensive if the MPlot is just about to be deleted anyway, and we have a lot of plots. This optimization omits this useless process and speeds up the destructor.
	bool gettingDeleted_;

	/// This proxy object handles signals and slots for MPlot (so that MPlot does not need to be a QObject)
	MPlotSignalHandler* signalHandler_;
	friend class MPlotSignalHandler;
};

#include <QGraphicsWidget>
#include <QGraphicsSceneResizeEvent>

/// This class is used instead of MPlot when you need a QGraphicsWidget (instead of a simple QGraphicsItem).
class MPlotGW : public QGraphicsWidget {
	Q_OBJECT
public:
	/// Constructor.  Builds a QGraphicsWidget that encapuslates an MPlot.
	MPlotGW(QGraphicsItem* parent = 0, Qt::WindowFlags flags = 0);
	/// Destructor.
	virtual ~MPlotGW();

	/// Returns the MPlot encapsulated by this QGraphicsWidget.
	MPlot* plot() const;

protected:
	/// The reference to the MPlot encapsulated by this widget.
	MPlot* plot_;
	/// Method that has a custom outcome to standard resizeEvents from the event loop.
	virtual void resizeEvent ( QGraphicsSceneResizeEvent * event );
};

#endif
