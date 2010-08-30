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

protected:
	MPlot* plot_;
};

/// This class provides plotting capabilities within a QGraphicsItem that can be added to any QGraphicsScene,
class MPlot : public QGraphicsItem {

public:
	MPlot(QRectF rect = QRectF(0,0,100,100), QGraphicsItem* parent = 0);
	virtual ~MPlot();
	/// Required paint function. (All painting is done by children)
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	virtual QRectF boundingRect() const;

	/// Use this to add a new data-item to a plot:
	void addItem(MPlotItem* newItem);
	/// Remove a data-item from a plot. (Note: Does not delete the item...)
	bool removeItem(MPlotItem* removeMe);

	QList<MPlotItem*> plotItems() const;

	/// Add a tool to the plot:
	void addTool(MPlotAbstractTool* newTool);

	/// Remove a tool from a plot. (Note: Does not delete the tool...)
	bool removeTool(MPlotAbstractTool* removeMe);

	QGraphicsRectItem* plotArea() const;
	// access elements of the canvas:
	MPlotAxis* axisBottom();
	MPlotAxis* axisTop();
	MPlotAxis* axisLeft();
	MPlotAxis* axisRight();


	// Access properties of the Canvas: (TODO: add to property system)
	QGraphicsRectItem* background();

	/// returns the rectangle filled by this plot (in scene or parent QGraphicsItem coordinates)
	QRectF rect() const;

	/// Sets the rectangle to be filled by this plot (in scene or parent QGraphicsItem coordinates).
	/*! Also rescales and re-applies the margins and transform for the plotArea). Can call with setRect(rect()) to re-compute margins.)*/
	void setRect(const QRectF& rect);
	// Margins: are set in logical coordinates (ie: as a percentage of the chart width or chart height);
	double margin(MPlotAxis::AxisID margin) const;

	double marginLeft() const;
	double marginRight() const;
	double marginTop() const;
	double marginBottom() const;

	void setMargin(MPlotAxis::AxisID margin, double value);

	void setMarginLeft(double value);
	void setMarginRight(double value);
	void setMarginTop(double value);
	void setMarginBottom(double value);


	double xMin();
	double xMax();
	double yLeftMin();
	double yRightMin();
	double yLeftMax();
	double yRightMax();
	QTransform leftAxisTransform();
	QTransform rightAxisTransform();


	void enableAutoScaleBottom(bool autoScaleOn);
	void enableAutoScaleLeft(bool autoScaleOn);
	void enableAutoScaleRight(bool autoScaleOn);
	void enableAutoScale(int axisFlags);



	void setScalePadding(double percent);

	double scalePadding();

	void setXDataRange(double min, double max, bool autoscale = false, bool applyPadding = true);

	void setYDataRangeLeft(double min, double max, bool autoscale = false, bool applyPadding = true);

	void setYDataRangeRight(double min, double max, bool autoscale = false, bool applyPadding = true);

	/// called automatically when control returns to the event loop, this completes a delayed autoscale. (Recomputing the scale limits is optimized to be only done when necessary, rather than whenever the values change.)  If you need the scene to be updated NOW! (for example, you're working outside of an event loop, or rendering before returning to the event loop), you can call this manually.
	void doDelayedAutoScale();



protected: // "slots" (proxied through MPlotSignalHandler)
	/// called when the x-y data in a plot item might have changed, such that a re-autoscale is necessary
	void onBoundsChanged(MPlotItem* source);
	/// called when the selected state of a plot item changes
	void onSelectedChanged(MPlotItem* source, bool isSelected);




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
