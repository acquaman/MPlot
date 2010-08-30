#ifndef __MPlotSeries_H__
#define __MPlotSeries_H__

#include "MPlotMarker.h"
#include "MPlotItem.h"
#include "MPlotSeriesData.h"

#include <QPen>
#include <QBrush>
#include <QPainter>
#include <QDebug>


/// When the number of points exceeds this, we simply return the bounding box instead of the exact shape of the plot.  Makes selection less precise, but faster.
#define MPLOT_EXACTSHAPE_POINT_LIMIT 10000

class MPlotAbstractSeries;

/// This class receives and processes signals for MPlotAbstractSeriesData. You should never need to use it directly.
/*! To avoid multiple-inheritance restrictions, MPlotAbstractSeries does not inherit from QObject.  However, it needs a way to receive signals from MPlotAbstractSeriesData. This proxy signal handling is enabled by this class.*/
class MPlotSeriesSignalHandler : public QObject {
	Q_OBJECT
protected:
	MPlotSeriesSignalHandler(MPlotAbstractSeries* parent);
	friend class MPlotAbstractSeries;

protected slots:
	void onDataChanged();

protected:
	MPlotAbstractSeries* series_;
};


/// MPlotAbstractSeries is the base class for all MPlotItems that display series curves (2D curve data) on a plot.  Different drawing implementations are possible. One implementation is provided in MPlotSeriesBasic.
class MPlotAbstractSeries : public MPlotItem {

public:

	MPlotAbstractSeries(const MPlotAbstractSeriesData* data = 0);

	virtual ~MPlotAbstractSeries();

	/// Properties:
	virtual void setLinePen(const QPen& pen);

	/// Returns the current marker, which can be used to access it's pen, brush, and size.
	/*! If the plot has no marker (or MPlotMarkerShape::None), then this will be a null pointer. Must check before setting.*/
	virtual MPlotAbstractMarker* marker() const;

	virtual void setMarker(MPlotMarkerShape::Shape shape, double size = 6, const QPen& pen = QPen(QColor(Qt::red)), const QBrush& brush = QBrush());


	/// Sets this series to view the model in 'data';
	virtual void setModel(const MPlotAbstractSeriesData* data);

	virtual QString name() const;
	virtual void setName(const QString& name);

	virtual const MPlotAbstractSeriesData* model() const;


	/// Required functions:
	//////////////////////////
	/// Bounding rect: reported in our PlotSeries coordinates, which are just the actual data coordinates. This is used by the graphics view system to figure out how much we cover/need to redraw.  Subclasses that draw selection borders or markers need to add their size on top of this.  This value is cached to the last redraw/update(), so that it is in sync with what is on the screen.
	virtual QRectF boundingRect() const;

	/// Data rect: also reported in our PlotSeries coordinates, which are the actual data coordinates. This is used by the auto-scaling to figure out the range of our data on an axis.  This value is not cached -- it is the real-time extent of the data, as reported by the model.
	virtual QRectF dataRect() const;

	/// Paint: must be implemented in subclass.
	virtual void paint(QPainter* painter,
					   const QStyleOptionGraphicsItem* option,
					   QWidget* widget) = 0;

	virtual QPainterPath shape() const;


private: // "slots"
	/// This implementation is called first when the source data changes. It flags the bounding rectangle for an update, warns the scene of geometry changes, and emits a boundsChanged signal to attached plots. Then it calls onDataChanged(), which can be re-implemented by subclasses.
	void onDataChangedPrivate();

protected: // "slots"
	/// This virtual function is called by the base class to let subclasses know when the internal data has changed, and let's them handle this however they need to.
	virtual void onDataChanged() = 0;

protected:
	QPen linePen_, selectedPen_;
	MPlotAbstractMarker* marker_;

	QString name_;

	const MPlotAbstractSeriesData* data_;

	/// Implements caching of the bounding rectangle
	mutable QRectF cachedDataRect_;
	/// If true, indicates that the cachedDataRect_ is stale. Set true when the model indicates data changed; set false when the cachedDataRect_ is updated inside boundingRect().
	mutable bool dataChangedUpdateNeeded_;

	virtual void setDefaults();



	/// Receives signals for us, from MPlotAbstractSeriesData implementations
	MPlotSeriesSignalHandler* signalHandler_;
	friend class MPlotSeriesSignalHandler;
};






/// When drawing large datasets, we won't draw more than MPLOT_MAX_LINES_PER_PIXEL lines per x-axis pixel.
/// We sub-sample by plotting only the maximum and minimum values over the x-axis increment that corresponds to 1px.
/// The value that makes sense here is 1 (since you can't see any more... they would just look like vertical lines on top of each other anyway.)  When drawing anti-aliased, changing this to 2 makes smoother plots.
#define MPLOT_MAX_LINES_PER_PIXEL 2.0

/// If you're going to add a lot of points to the model (without caring about updates in between), recommend this for performance reasons:
/*
	MPlotSeriesBasic series;
	 ....
	series->setModel(0);	/// disconnect series from data model
	/// add points to model...
	series->setModel(model);	/// reconnect model to series
 */

/// MPlotSeriesBasic provides one drawing implementation for a 2D plot curve.  It is optimized to efficiently draw curves with 1,000,000+ data points along the x-axis, by only drawing as many lines as would be visible.

class MPlotSeriesBasic : public MPlotAbstractSeries {

public:

	MPlotSeriesBasic(const MPlotAbstractSeriesData* data = 0);
	virtual ~MPlotSeriesBasic();


	/// Required functions:
	//////////////////////////
	/// boundingRect: reported in our PlotSeries coordinates, which are just the actual data coordinates.
	/// using parent implementation, but adding extra room on edges for our selection highlight and markers.
	virtual QRectF boundingRect() const;

	/// Paint:
	virtual void paint(QPainter* painter,
					   const QStyleOptionGraphicsItem* option,
					   QWidget* widget);

	virtual void paintLines(QPainter* painter);

	virtual void paintMarkers(QPainter* painter);

	/// re-implemented from MPlotItem base to draw an update if we're now selected (with our selection highlight)
	virtual void setSelected(bool selected = true);

	virtual void setModel(const MPlotAbstractSeriesData *data) {
		MPlotAbstractSeries::setModel(data);
		update();
	}


protected: //"slots"

	/// Handle implementation-specific drawing updates
	virtual void onDataChanged();


protected:

	/// Customize this if needed for MPlotSeries. For now we use the parent class implementation
	/*
	 virtual void setDefaults() {

	 }*/

};

#endif
