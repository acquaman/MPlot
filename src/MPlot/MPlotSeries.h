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


/// MPlotAbstractSeries is the base class for all MPlotItems that display series curves (2D curve data) on a plot.
/*! Different drawing implementations of MPlotAbstractSeries are possible, with various performance trade-offs. One implementation is provided in MPlotSeriesBasic.

	Series curves have the ability to apply transformations on top of their underlying data (MPlotAbstractSeriesData).  This allows (for example) for two series to share the same data model(), but display it scaled or shifted in different ways:

	- applyTransform(double sx, double sy, double dx, double dy) applies a constant transformation to all the data points. They are first scaled by (sx, sy) and then shifted by (dx, dy).
	- Alternatively, you can use enableYAxisNormalization(bool on, double min, double max) or enableXAxisTransformation(bool on, double min, double max) to keep the data normalized within a specified range.  When normalization is enabled, regardless of how the data source changes, the minimum value will always appear at \c min and the maximum value will always appear at \c max.  This effectively disables applyTransform().
	- Finally, you can apply an offset (commonly used for "waterfall" plots) using setOffset(double dx, double dy). This offset is always applied last, after applyTransform() or normalization is applied.
	- currentTransform() returns the current transformation being applied on top of the data, either due to an explicit setTransform() call, or the transform that was calculated for the last normalization.
	*/

class MPlotAbstractSeries : public MPlotItem {

public:

	enum { Type = MPlotItem::Series };
	int type() const { return Type; }

	MPlotAbstractSeries();

	virtual ~MPlotAbstractSeries();

	/// Properties:
	virtual void setLinePen(const QPen& pen);

	/// Returns the current marker, which can be used to access it's pen, brush, and size.
	/*! If the plot has no marker (or MPlotMarkerShape::None), then this will be a null pointer. Must check before setting.*/
	virtual MPlotAbstractMarker* marker() const;

	virtual void setMarker(MPlotMarkerShape::Shape shape, double size = 6, const QPen& pen = QPen(QColor(Qt::red)), const QBrush& brush = QBrush());


	/// Sets this series to view the model in 'data'.  If the series should delete the model when it gets deleted, set \c ownsModel to true.
	virtual void setModel(const MPlotAbstractSeriesData* data, bool ownsModel = false);


	virtual const MPlotAbstractSeriesData* model() const;

	/// Re-implemented from MPlotItem to provide our line color as the legend color:
	virtual QBrush legendColor() const { return QBrush(linePen_.color()); }


	// Transformation and normalization
	//////////////////////////////
	/// Use this function to apply a constant transformation to the series, on top of the underlying data. All data points are first scaled by (\c sx, \c sy) and then shifted by (\c dx, \c dy).
	/*! Calling this function will only have an effect on axes which do not have normalization enabled (using enableYAxisNormalization() or enableXAxisNormalization()). If you want your changes to stick, be sure to disable normalization first.*/
	void applyTransform(double sx = 1, double sy = 1, double dx = 0, double dy = 0);
	/// Call this function to keep the data normalized within a specified range.  When normalization is enabled, regardless of how the data source changes, the minimum value will always appear at \c min and the maximum value will always appear at \c max.  This effectively disables applyTransform() in the y-axis.
	void enableYAxisNormalization(bool on = true, double min = 0, double max = 1);
	/// Call this function to keep the data normalized within a specified range.  When normalization is enabled, regardless of how the data source changes, the minimum value will always appear at \c min and the maximum value will always appear at \c max.  This effectively disables applyTransform() in the x-axis.
	void enableXAxisNormalization(bool on = true, double min = 0, double max = 1);

	/// You can apply an offset (commonly used for "waterfall" plots) using setOffset(double dx, double dy). This offset is always applied last, after applyTransform() or normalization is applied.
	void setOffset(double dx = 0, double dy = 0);

	/// This function returns the current transformation being applied on top of the data, either due to an explicit applyTransform() call, or due to the transform that was calculated for the last normalization.  It does not include the waterfall offset().
	/*! For performance, re-normalization is only carried out every paint event. The transform due to the last normalization will not be valid until a paint event occurs. */
	QTransform currentTransform() const { return QTransform(	sx_, 0, 0,
															0, sy_, 0,
															dx_, dy_, 1.0); }

	/// Same as currentTransform(), but includes the waterfall offset() in the translation
	QTransform completeTransform() const { return QTransform(	sx_, 0, 0,
															0, sy_, 0,
															dx_+offset_.x(), dy_+offset_.y(), 1.0); }

	/// Perfectly flat lines can't be amplified to anything, even with an infinite scale factor. This is a limit on the smallest normalization range
#define MPLOT_MIN_NORMALIZATION_RANGE 1e-30

	/// This function returns the current offset, which is a shift applied after all transformation and normalization is complete.
	QPointF offset() const { return offset_; }



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
	bool ownsModel_;

	/// Implements caching of the bounding rectangle
	mutable QRectF cachedDataRect_;
	/// If true, indicates that the cachedDataRect_ is stale. Set true when the model indicates data changed; set false when the cachedDataRect_ is updated inside boundingRect().
	mutable bool dataChangedUpdateNeeded_;


	// transformation and normalization
	/////////////////////////

	/// Scale and shift factors
	mutable double sx_, sy_, dx_, dy_;

	/// Offsets (applied last)
	QPointF offset_;

	/// Indicates whether normalization is on:
	bool yAxisNormalizationOn_, xAxisNormalizationOn_;
	/// Normalization ranges:
	double normYMin_, normYMax_, normXMin_, normXMax_;

	/// Helper function to return a the transformed, normalized, offsetted x value. (Only call when model() is valid, and i<model().count()!)
	double xx(unsigned i) const { return data_->x(i)*sx_+dx_+offset_.x(); }
	/// Helper function to return a the transformed, normalized, offsetted x value. (Only call when model() is valid, and i<model().count()!)
	double yy(unsigned i) const { return data_->y(i)*sy_+dy_+offset_.y(); }


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
