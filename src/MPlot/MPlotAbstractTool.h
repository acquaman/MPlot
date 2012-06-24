#ifndef MPLOTABSTRACTTOOL_H
#define MPLOTABSTRACTTOOL_H

#include "MPlot/MPlot_global.h"

#include "MPlot/MPlotAxisScale.h"
#include <QGraphicsObject>

class MPlot;

/// MPlotTools are objects that can be added to an MPlot to allow different kinds of interaction with it.
/*! This class defines the interface for all plot tools. */
class MPLOTSHARED_EXPORT MPlotAbstractTool : public QGraphicsObject
{
	Q_OBJECT
public:
	/// Constructor.  Builds the basis for any real MPlot tools.  Defines a default geometry if one is not provided.
	MPlotAbstractTool(const QRectF& geometry = QRectF(0,0,100,100));
	/// Destructor.  Removes the tool from the plot.
	virtual ~MPlotAbstractTool();

	/// Some tools don't need to draw anything. This provides an empty paint() function. Reimplement if drawing is needed.
	virtual void	paint ( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0 ) {
		Q_UNUSED(painter)
		Q_UNUSED(option)
		Q_UNUSED(widget)
	}

	/// boundingRect() is implemented by default.  Use setRect() to adjust the placement and size. (MPlot uses this to size the tools appropriately when the plot area size changes.)
	QRectF boundingRect() const { return rect_; }

	virtual void setRect(const QRectF& rect) { prepareGeometryChange(); rect_ = rect; update(); }

	/// Don't call this. Unfortunately public because it's required by MPlot::addTool and MPlot::removeTool.
	void setPlot(MPlot* plot) { plot_ = plot; targetAxes_.clear(); }
	MPlot* plot() const { return plot_; }

	/// Some plot tools might be able to operate on a single axis, or a combination of them.
	QList<MPlotAxisScale*> targetAxes() const { return targetAxes_; }
	void setTargetAxes(const QList<MPlotAxisScale*> targetAxes) { targetAxes_ = targetAxes; }


protected:
	/// Defining the mousePressEvent as a pure virtual function to ensure that all the subclasses must reimplement it.
	virtual void	mousePressEvent ( QGraphicsSceneMouseEvent * event ) = 0;
	/// Defining the mouseMoveEvent as a pure virtual function to ensure that all the subclasses must reimplement it.
	virtual void	mouseMoveEvent ( QGraphicsSceneMouseEvent * event ) = 0;
	/// Defining the mouseReleaseEvent as a pure virtual function to ensure that all the subclasses must reimplement it.
	virtual void	mouseReleaseEvent ( QGraphicsSceneMouseEvent * event ) = 0;
	/// Defining the wheelEvent as a pure virtual function to ensure that all the subclasses must reimplement it.
	virtual void	wheelEvent ( QGraphicsSceneWheelEvent * event ) = 0;
	/// Defining the mouseDoubleClickEvent as a pure virtual function to ensure that all the subclasses must reimplement it.
	virtual void	mouseDoubleClickEvent ( QGraphicsSceneMouseEvent * event ) = 0;

	/// The plot we're attached to
	MPlot* plot_;
	/// the position and size of the tool, in drawing coordinates, relative to the MPlot::plotArea_.
	QRectF rect_;
	/// The axes we're active on.
	QList<MPlotAxisScale*> targetAxes_;
};

#endif // MPLOTABSTRACTTOOL_H
