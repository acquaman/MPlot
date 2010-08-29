#ifndef __MPlot_H__
#define __MPlot_H__

#include "MPlotAxis.h"
#include "MPlotLegend.h"
#include "MPlotItem.h"
#include "MPlotSeries.h"
#include "MPlotAbstractTool.h"
#include "MPlotObserver.h"

#include <QList>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsRectItem>

#include <float.h>

#include <QDebug>

/// Defines the minimum distance between min- and max- values for the range of an axis. Without this check, calling setXDataRange(3, 3) or set___DataRange(f, g=f) will cause a segfault within Qt's drawing functions... it can't handle a clipPath with a width of 0.
#define MPLOT_MIN_AXIS_RANGE 1e-60

/// This class provides plotting capabilities within a QGraphicsItem that can be added to any QGraphicsScene,
class MPlot : public QGraphicsItem, public MPlotObserver {

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

public: // "slots"

	// This is called when a item updates it's data.  We may have to autoscale/rescale.  Assumption: the only update messages we get are from MPlotItems. (Don't hook up anything else.)
	virtual void onObservableChanged(MPlotObservable* source, int code, const char* msg, int payload);

	/// called when item data changes in a way that could affect the plot scaling.  item1 could be a plot that was just added, and it could also be a plot item on it's way out. (ie: no longer part of items_, but just recently removed.)
	void onDataChanged(MPlotItem* item1);



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
