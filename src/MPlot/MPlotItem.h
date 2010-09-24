#ifndef MPLOTITEM_H
#define MPLOTITEM_H

#include <QGraphicsItem>
#include "MPlotAxis.h"


/// This is the color of the selection highlight
#define MPLOT_SELECTION_COLOR QColor(255, 210, 129)
/// The opacity level (0=transparent, 1=opaque) of selection rectangles:
#define MPLOT_SELECTION_OPACITY 0.35
/// This is the width of selection highlight lines
#define MPLOT_SELECTION_LINEWIDTH 10

class MPlot;

/// This class is a proxy that emits signals for an MPlotItem.
/*! To avoid multipler-inheritance restrictions, MPlotItems do not inherit QObject.  However, they need some way to emit signals to notify plots of relevant events.  Therefore, they each contain an MPlotItemSignalSource, which emits signals on their behalf.  You can access it with MPlotItem::signalSource().

  There are two relevant signals:
  - boundsChanged() is emitted when the extent of this item's x- or y-data might have changed such that a re-autoscale is necessary.
  - selectedChanged(bool isSelected) is emitted whenever the selection state of this item changes.
  */

class MPlotItem;

class MPlotItemSignalSource : public QObject {
	Q_OBJECT
public:
	MPlotItem* plotItem() const { return plotItem_; }

protected:
	MPlotItemSignalSource(MPlotItem* parent);

	/// called within MPlotItem to forward this signal
	void emitBoundsChanged() { emit boundsChanged(); }
	/// called within MPlotItem to forward this signal
	void emitSelectedChanged(bool isSelected) { emit selectedChanged(isSelected); }
	/// called within MPlotItem to forward this signal
	void emitLegendContentChanged() { emit legendContentChanged(); }

	/// Allow MPlotItem access to these protected functions:
	friend class MPlotItem;

	MPlotItem* plotItem_;

signals:
	void boundsChanged();
	void selectedChanged(bool);
	void legendContentChanged();

};

/// This class defines the interface for all data-representation objects which can be added to an MPlot (ex: series/curves, images and spectrograms, contour maps, etc.)
class MPlotItem : public QGraphicsItem {


public:

	/// Used to distinguish types/subclasses of MPlotItem.  See MPlotItem::type() and QGraphicsItem::type().
	enum ItemTypes { PlotItem = QGraphicsItem::UserType + 3003, Series, Image };

	enum { Type = PlotItem };

	/// Returns the type of this item, to enable qgraphicsitem_cast() for casting to different MPlotItem subclasses.
	int type() const {
		return Type;
	}

	/// Constructor calls base class (QGraphicsObject)
	MPlotItem();



	/// \todo What to do about being connected to multiple plots?

	/// Removes this item from plot(), if it is attached to a plot.
	~MPlotItem();

	/// Connect to this proxy object to receive MPlotItem signals:
	MPlotItemSignalSource* signalSource() const { return signalSource_; }

	/// returns which y-axis this data should be plotted against
	MPlotAxis::AxisID yAxisTarget();

	/// set the y-axis this data should be plotted against
	void setYAxisTarget(MPlotAxis::AxisID axis);

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


	// Bounding rect: reported in our PlotItem coordinates, which are just the actual data coordinates. This is used by the graphics view system to figure out how much we cover/need to redraw.  Subclasses that draw selection borders or markers need to add their size on top of this.
	virtual QRectF boundingRect() const;

	// Data rect: also reported in our PlotItem coordinates, which are the actual data coordinates. This is used by the auto-scaling to figure out the range of our data on an axis.
	virtual QRectF dataRect() const = 0;

	/// Paint: must be implemented in subclass.
	virtual void paint(QPainter* painter,
					   const QStyleOptionGraphicsItem* option,
					   QWidget* widget) = 0;

	/// return the active shape where clicking will select this object in the plot. Subclasses can re-implement for more accuracy.
	virtual QPainterPath shape() const;


	/// signals: The signalSource() will emit boundsChanged() when the extent of this item's x- or y-data might have changed such that a re-autoscale is necessary.  It will emit selectedChanged(bool isSelected) whenever the selection state of this item changes.


private:
	bool isSelected_, isSelectable_;
	MPlotAxis::AxisID yAxisTarget_;

	MPlot* plot_;

	QString description_;

protected:
	MPlotItemSignalSource* signalSource_;
	friend class MPlotItemSignalSource;

	/// called within MPlotItem to forward this signal
	void emitBoundsChanged() { signalSource_->emitBoundsChanged(); }
	/// called within MPlotItem to forward this signal
	void emitSelectedChanged(bool isSelected) { signalSource_->emitSelectedChanged(isSelected); }
	/// called within MPlotItem to forward this signal
	void emitLegendContentChanged() { signalSource_->emitLegendContentChanged(); }
};

#endif // MPLOTITEM_H
