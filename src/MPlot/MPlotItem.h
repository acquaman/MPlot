#ifndef MPLOTITEM_H
#define MPLOTITEM_H

#include <QGraphicsItem>
#include <QObject>
#include <QBrush>


/// This is the color of the selection highlight
#define MPLOT_SELECTION_COLOR QColor(255, 210, 129)
/// This is the alternate color of the selection highlight.
#define MPLOT_SELECTION_COLOR_ALT QColor(138, 43, 226)
/// The opacity level (0=transparent, 1=opaque) of selection rectangles:
#define MPLOT_SELECTION_OPACITY 0.35
/// This is the width of selection highlight lines
#define MPLOT_SELECTION_LINEWIDTH 10

class MPlot;
#include "MPlotAxisScale.h"

/// This class is a proxy that emits signals for an MPlotItem.
/*! To avoid multipler-inheritance restrictions, MPlotItems do not inherit QObject.  However, they need some way to emit signals to notify plots of relevant events.  Therefore, they each contain an MPlotItemSignalSource, which emits signals on their behalf.  You can access it with MPlotItem::signalSource().

  There are two relevant signals:
  - boundsChanged() is emitted when the extent of this item's x- or y-data might have changed such that a re-autoscale is necessary.
  - selectedChanged(bool isSelected) is emitted whenever the selection state of this item changes.

  And one important slot:
  - onAxisScaleAboutToChange() calls the plot item's onAxisScaleAboutToChange() to have the item re-paint itself on the new scale.
  */

class MPlotItem;

class MPlotItemSignalSource : public QObject {
	Q_OBJECT
public:
		/// Returns the plot item this signal source is managing.
	MPlotItem* plotItem() const { return plotItem_; }

public slots:
		/// Slot handling initial setup if the axis scale is about to change.
	void onAxisScaleAboutToChange() const;
		/// Slot handling the axis change.
	void onAxisScaleChanged() const;

protected:
		/// Constructor.  Builds a signal source to manage an MPlot item.
	MPlotItemSignalSource(MPlotItem* parent);

	/// called within MPlotItem to forward this signal
	void emitBoundsChanged() { emit boundsChanged(); }
	/// called within MPlotItem to forward this signal
	void emitSelectedChanged(bool isSelected) { emit selectedChanged(isSelected); }
	/// called within MPlotItem to forward this signal
	void emitLegendContentChanged() { emit legendContentChanged(); }

	/// Allow MPlotItem access to these protected functions:
	friend class MPlotItem;
		/// Pointer to the plot item this source is managing.
	MPlotItem* plotItem_;

signals:
		/// Notifier that the bounds for the plot item has changed.
	void boundsChanged();
		/// Notifier that whether the item is selected or not has changed.
	void selectedChanged(bool);
		/// Notifier that the legend has changed.
	void legendContentChanged();

};

/// This class defines the interface for all data-representation objects which can be added to an MPlot (ex: series/curves, images and spectrograms, contour maps, etc.)
class MPlotItem : public QGraphicsItem {


public:

	/// Used to distinguish types/subclasses of MPlotItem.  See MPlotItem::type() and QGraphicsItem::type().
	enum ItemTypes { PlotItem = QGraphicsItem::UserType + 3003, Series, Image };
		/// Convenience enum.
	enum { Type = PlotItem };

	/// Returns the type of this item, to enable qgraphicsitem_cast() for casting to different MPlotItem subclasses.  See qgraphicsitem_cast() for more information.
	virtual int type() const {
		return Type;
	}

	/// Returns the dimensionality of data this plot item is capable of representing.  This number should be equal to the number of <i>independent axis</i> that can be supported. (For ex: an X-Y series can represent one independent axis; an image colorplot could represent 2 independent axes). This function should be re-implemented by actual MPlotItems.
	virtual int rank() const {
		return 0;
	}

	/// Constructor calls base class (QGraphicsObject)
	MPlotItem();

	/// \todo What to do about being connected to multiple plots?

	/// Removes this item from plot(), if it is attached to a plot.
	~MPlotItem();

	/// Connect to this proxy object to receive MPlotItem signals:
	MPlotItemSignalSource* signalSource() const { return signalSource_; }

	/// returns which y-axis scale this data should use / be plotted against
	const MPlotAxisScale* yAxisTarget() const { return yAxisTarget_; }
	MPlotAxisScale* yAxisTarget() { return yAxisTarget_; }
	/// set the y-axis scale this data should be plotted against
	void setYAxisTarget(MPlotAxisScale* yAxisTarget);

	/// Returns the x-axis scale this data should use / be plotted against
	const MPlotAxisScale* xAxisTarget() const { return xAxisTarget_; }
	MPlotAxisScale* xAxisTarget() { return xAxisTarget_; }
	/// set the x-axis scale this data shoud use / be plotted against
	void setXAxisTarget(MPlotAxisScale* xAxisTarget);

	/// Indicates that this item should be ignored when auto-scaling (ie: it should not be considered to affect the range of its xAxisTarget() and yAxisTarget() when these target axis scales have autoScaling enabled.)
	bool ignoreWhenAutoScaling() const { return ignoreWhenAutoScaling_; }
	/// Set that this item should be ignored when auto-scaling (ie: it should not be considered to affect the range of its xAxisTarget() and yAxisTarget() when these target axis scales have autoScaling enabled.)
	void setIgnoreWhenAutoScaling(bool ignore);

	/// tell this item that it is 'selected' within the plot
	virtual void setSelected(bool selected = true);
	/// ask if this item is currently selected on the plot
	virtual bool selected();

	/// use this if you don't want a plot item to be selectable:
	virtual bool selectable();
	virtual void setSelectable(bool selectable = true);

	// Legend information
	//////////////////////////////
	/// The name of this plot item / data series. Plot Item descriptions can be displayed in an MPlotLegend.
	virtual QString description() const { return description_; }
	/// Set the name of this plot item
	virtual void setDescription(const QString& description) { description_ = description; emitLegendContentChanged(); }
	/// The color used to represent this plot item in the legend.  Subclasses can re-implement this for more detail.
	virtual QBrush legendColor() const { return QBrush(QColor(121, 121, 121)); }


	/// Don't call this. Unfortunately public because it's required by MPlot::addItem and MPlot::removeItem.
	void setPlot(MPlot* plot);
	/// returns the plot we are attached to
	MPlot* plot() const;


	/// Bounding rect: This is the rectangle enclosing the plot item, in drawing coordinates.  It is used by the graphics view system to figure out how much we cover/need to redraw.  Subclasses that draw selection borders or markers need to add their size on top of this.
	virtual QRectF boundingRect() const;

	/// Data rect: This is the rectangle enclosing the data points, in raw data coordinates.  It is used by the auto-scaling system to figure out the range of our data on an axis.
	/*! \note The default implementation of boundingRect() calls dataRect() to find out the extent of the item in data coordinates.  If you re-implement dataRect() so that this isn't true (for ex: you return a null QRectF() so that the item isn't included in auto-scaling), you must also re-implement boundingRect() to return the actual drawing-coordinate extent of the item. */
	virtual QRectF dataRect() const = 0;

	/// Paint: must be implemented in subclass.
	virtual void paint(QPainter* painter,
					   const QStyleOptionGraphicsItem* option,
					   QWidget* widget) = 0;

	/// return the active shape where clicking will select this object in the plot. Subclasses can re-implement for more accuracy.
	virtual QPainterPath shape() const;


	/// signals: The signalSource() will emit boundsChanged() when the extent of this item's x- or y-data might have changed such that a re-autoscale is necessary.  It will emit selectedChanged(bool isSelected) whenever the selection state of this item changes.


private:
		/// Bool holding whether the item is selectable and selected.
	bool isSelected_, isSelectable_;
		/// The x and y axis scale for the plot item.
	MPlotAxisScale* yAxisTarget_, *xAxisTarget_;

		/// The plot that this item belongs to.
	MPlot* plot_;

		/// Description for the plot item.
	QString description_;

		/// Bool holding whether or not the plot item should be considered when auto-scaling.
	bool ignoreWhenAutoScaling_;

protected:
		/// Pointer to the signal source managing the signals for the plot item.
	MPlotItemSignalSource* signalSource_;
		/// Giving access to the signal source.
	friend class MPlotItemSignalSource;

	/// called within MPlotItem to forward this signal
	void emitBoundsChanged() { signalSource_->emitBoundsChanged(); }
	/// called within MPlotItem to forward this signal
	void emitSelectedChanged(bool isSelected) { signalSource_->emitSelectedChanged(isSelected); }
	/// called within MPlotItem to forward this signal
	void emitLegendContentChanged() { signalSource_->emitLegendContentChanged(); }

	/// Triggered when the axis scale it uses will be changed, affecting its geometry.  You can re-implement this if you need to handle anything in custom subclasses, but call the base class implementation first.  The base class implementation notifies the scene that the geometry of this object (boundingBox) will be changing, and schedules a paint update().
	virtual void onAxisScaleAboutToChange() {
		prepareGeometryChange();
		update();
	}
	/// Triggered when the axis scale has changed, affecting this item's geometry.  This signal is received after the change is completed.  You can re-implement this if you need to handle anything in custom subclasses.  The base class implementation does nothing.
	virtual void onAxisScaleChanged() {}


	/// Shorthand to map a data coordinate to drawing coordinate.  Only call when xAxisTarget_ is valid.
	qreal mapX(qreal dataCoordinate) const { return xAxisTarget_->mapDataToDrawing(dataCoordinate); }
	/// Shorthand to map a data coordinate to drawing coordinate.  Only call when yAxisTarget_ is valid.
	qreal mapY(qreal dataCoordinate) const { return yAxisTarget_->mapDataToDrawing(dataCoordinate); }
};

#endif // MPLOTITEM_H
