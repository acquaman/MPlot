#ifndef __MPlotSeries_H__
#define __MPlotSeries_H__

#include "MPlot/MPlot_global.h"

#include "MPlot/MPlotMarker.h"
#include "MPlot/MPlotItem.h"
#include "MPlot/MPlotSeriesData.h"

#include <QPen>
#include <QBrush>
class QPainter;



/// When the number of points exceeds this, we simply return the bounding box instead of the exact shape of the plot.  Makes selection less precise, but faster.
#define MPLOT_EXACTSHAPE_POINT_LIMIT 10000

class MPlotAbstractSeries;

/// This class receives and processes signals for MPlotAbstractSeriesData. You should never need to use it directly.
/*! To avoid multiple-inheritance restrictions, MPlotAbstractSeries does not inherit from QObject.  However, it needs a way to receive signals from MPlotAbstractSeriesData. This proxy signal handling is enabled by this class.*/
class MPlotSeriesSignalHandler : public QObject {
	Q_OBJECT
protected:
	/// Builds a signal source for a series.
	MPlotSeriesSignalHandler(MPlotAbstractSeries* parent);
	/// Giving access to the series methods.
	friend class MPlotAbstractSeries;

protected slots:
	/// Handles changes to the data in the series.
	void onDataChanged();

protected:
	/// Pointer to the series the signal handler is managing.
	MPlotAbstractSeries* series_;
};


/// MPlotAbstractSeries is the base class for all MPlotItems that display series curves (2D curve data) on a plot.
/*! Different drawing implementations of MPlotAbstractSeries are possible, with various performance trade-offs. One implementation is provided in MPlotSeriesBasic.

 Series curves have the ability to apply transformations on top of their underlying data (MPlotAbstractSeriesData).  This allows (for example) for two series to share the same data model(), but display it scaled or shifted in different ways:

 - applyTransform(qreal sx, qreal sy, qreal dx, qreal dy) applies a constant transformation to all the data points. They are first scaled by (sx, sy) and then shifted by (dx, dy).
 - Alternatively, you can use enableYAxisNormalization(bool on, qreal min, qreal max) or enableXAxisTransformation(bool on, qreal min, qreal max) to keep the data normalized within a specified range.  When normalization is enabled, regardless of how the data source changes, the minimum value will always appear at \c min and the maximum value will always appear at \c max.  This effectively disables applyTransform().
 - Finally, you can apply an offset (commonly used for "waterfall" plots) using setOffset(qreal dx, qreal dy). This offset is always applied last, after applyTransform() or normalization is applied.
 - currentTransform() returns the current transformation being applied on top of the data, either due to an explicit setTransform() call, or the transform that was calculated for the last normalization.
 */

class MPLOTSHARED_EXPORT MPlotAbstractSeries : public MPlotItem {

public:
	/// Convenience enum.
	enum { Type = MPlotItem::Series };
	/// Returns the type which is an MPlotItem::Series.
	virtual int type() const { return Type; }
	/// Returns the rank.  Because it is a series it always returns 1.
	virtual int rank() const { return 1; }

	/// Constructor.  Builds the series.
	MPlotAbstractSeries();
	/// Destructor.
	virtual ~MPlotAbstractSeries();

	// Properties:
	////////////////////
	/// Sets the pen used to draw the series in the plot.
	virtual void setLinePen(const QPen& pen);
	/// Returns the current pen used to draw the series.
	QPen linePen() const { return linePen_; }

	/// Returns the current marker, which can be used to access it's pen, brush, and size.
	/*! If the plot has no marker (or MPlotMarkerShape::None), then this will be a null pointer. Must check before setting.*/
	virtual MPlotAbstractMarker* marker() const;
	/// Sets a marker to the go with each point that makes up the series.
	virtual void setMarker(MPlotMarkerShape::Shape shape, qreal size = 6, const QPen& pen = QPen(QColor(Qt::red)), const QBrush& brush = QBrush());


	/// Sets this series to view the model in 'data'.  If the series should take ownership of the model (ie: delete the model when it gets deleted), set \c ownsModel to true. (If a model was previously set with \c ownsModel = true, then this function will delete the old model.)
	virtual void setModel(const MPlotAbstractSeriesData* data, bool ownsModel = false);

	/// Returns the data model used to draw the series.
	virtual const MPlotAbstractSeriesData* model() const;

	/// Re-implemented from MPlotItem to provide our line color as the legend color:
	virtual QBrush legendColor() const { return QBrush(linePen_.color()); }


	// Transformation and normalization
	//////////////////////////////
	/// Use this function to apply a constant transformation to the series, on top of the underlying data. All data points are first scaled by (\c sx, \c sy) and then shifted by (\c dx, \c dy).
	/*! Calling this function will only have an effect on axes which do not have normalization enabled (using enableYAxisNormalization() or enableXAxisNormalization()). If you want your changes to stick, be sure to disable normalization first.*/
	void applyTransform(qreal sx = 1, qreal sy = 1, qreal dx = 0, qreal dy = 0);
	/// Call this function to keep the data normalized within a specified range.  When normalization is enabled, regardless of how the data source changes, the minimum value will always appear at \c min and the maximum value will always appear at \c max.  This effectively disables applyTransform() in the y-axis.
	void enableYAxisNormalization(bool on = true, qreal min = 0, qreal max = 1);
	/// Overloaded.  Takes an MPlotAxisRange rather than a min and max.
	void enableYAxisNormalization(bool on, const MPlotAxisRange& normalizationRange) { enableYAxisNormalization(on, normalizationRange.min(), normalizationRange.max()); }
	/// Call this function to keep the data normalized within a specified range.  When normalization is enabled, regardless of how the data source changes, the minimum value will always appear at \c min and the maximum value will always appear at \c max.  This effectively disables applyTransform() in the x-axis.
	void enableXAxisNormalization(bool on = true, qreal min = 0, qreal max = 1);
	/// Overloaded.  Takes an MPlotAxisRange rather than a min and max.
	void enableXAxisNormalization(bool on, const MPlotAxisRange& normalizationRange) { enableXAxisNormalization(on, normalizationRange.min(), normalizationRange.max()); }
	/// You can apply an offset (commonly used for "waterfall" plots) using setOffset(qreal dx, qreal dy). This offset is always applied last, after applyTransform() or normalization is applied.
	void setOffset(qreal dx = 0, qreal dy = 0);

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

	// Required functions:
	//////////////////////////

	/// Data rect: the bounding box of the actual data coordinates. This is used by the auto-scaling to figure out the range of our data on an axis.
	virtual QRectF dataRect() const;

	/// Paint: must be implemented in subclass.
	virtual void paint(QPainter* painter,
					   const QStyleOptionGraphicsItem* option,
					   QWidget* widget) = 0;

	/// Returns the shape of the series as a QPainterPath. Contains all the information about drawing complex shapes.
	virtual QPainterPath shape() const;


private: // "slots"
	/// This implementation is called first when the source data changes. It flags the bounding rectangle for an update, warns the scene of geometry changes, and emits a boundsChanged signal to attached plots. Then it calls onDataChanged(), which can be re-implemented by subclasses.
	void onDataChangedPrivate();

protected: // "slots"
	/// This virtual function is called by the base class to let subclasses know when the internal data has changed, and let's them handle this however they need to.
	virtual void onDataChanged() = 0;

protected:
	/// Helper function to return a the transformed, normalized, offsetted x value. (Only call when model() is valid, and i<model().count()!)
	qreal xx(unsigned i) const { return data_->x(i)*sx_+dx_+offset_.x(); }
	/// Helper function to return a the transformed, normalized, offsetted x value. (Only call when model() is valid, and i<model().count()!)
	qreal yy(unsigned i) const { return data_->y(i)*sy_+dy_+offset_.y(); }
	/// Helper function that sets outputValues to a transformed, normalized, offsetted value.
	void xxValues(unsigned start, unsigned end, qreal *outputValues) const;
	/// Helper function that sets output values to a transformed, normalized, offsetted value.
	void yyValues(unsigned start, unsigned end, qreal *outputValues) const;

	/// Helper function that sets a default look and feel to the plot.
	virtual void setDefaults();

	/// Member holding the pen for drawing the series and a pen for drawing the series if it is selected.
	QPen linePen_, selectedPen_;
	/// Pointer to the marker used on each point of the series.
	MPlotAbstractMarker* marker_;

	/// Holds the name of the series.
	QString name_;

	/// Holds the data model for the series.
	const MPlotAbstractSeriesData* data_;
	/// Bool holding whether or not this series owns the data model or not.
	bool ownsModel_;

	/// Implements caching of the bounding rectangle
	mutable QRectF cachedDataRect_;
	/// If true, indicates that the cachedDataRect_ is stale. Set true when the model indicates data changed; set false when the cachedDataRect_ is updated inside boundingRect().
	mutable bool dataChangedUpdateNeeded_;


	// transformation and normalization
	/////////////////////////

	/// Scale and shift factors
	mutable qreal sx_, sy_, dx_, dy_;

	/// Offsets (applied last)
	QPointF offset_;

	/// Indicates whether normalization is on:
	bool yAxisNormalizationOn_, xAxisNormalizationOn_;
	/// Normalization ranges:
	qreal normYMin_, normYMax_, normXMin_, normXMax_;

	/// Receives signals for us, from MPlotAbstractSeriesData implementations
	MPlotSeriesSignalHandler* signalHandler_;
	friend class MPlotSeriesSignalHandler;
};


/// When drawing large datasets, we won't draw more than MPLOT_MAX_LINES_PER_PIXEL lines per x-axis pixel.
/// We sub-sample by plotting only the maximum and minimum values over the x-axis increment that corresponds to 1px.
/// The value that makes sense here is 1 (since you can't see any more... they would just look like vertical lines on top of each other anyway.)  When drawing anti-aliased, changing this to 2 makes smoother plots.
#define MPLOT_MAX_LINES_PER_PIXEL 2.0

/// If you're going to add a lot of points to the model (without caring about updates in between), recommend this for performance reasons:
/*!
 MPlotSeriesBasic series;
  ....
 series->setModel(0);	/// disconnect series from data model
 /// add points to model...
 series->setModel(model);	/// reconnect model to series
 */

/// MPlotSeriesBasic provides one drawing implementation for a 2D plot curve.  It is optimized to efficiently draw curves with 1,000,000+ data points along the x-axis, by only drawing as many lines as would be visible.

class MPLOTSHARED_EXPORT MPlotSeriesBasic : public MPlotAbstractSeries {

public:
	/// Constructor.  Builds a series of data.  This is a standard 1D line plot.
	MPlotSeriesBasic(const MPlotAbstractSeriesData* data = 0);
	/// Destructor.
	virtual ~MPlotSeriesBasic();


	/// Required functions:
	//////////////////////////
	/// boundingRect: using parent implementation, but adding extra room on edges for our selection highlight and markers.
	virtual QRectF boundingRect() const;

	/// Paint:
	virtual void paint(QPainter* painter,
					   const QStyleOptionGraphicsItem* option,
					   QWidget* widget);
	/// Helper function that paints the lines that connect the points of the series.
	virtual void paintLines(QPainter* painter);
	/// Helper function that paints the markers on the points that make up the series.
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
