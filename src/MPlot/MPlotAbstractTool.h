#ifndef MPLOTABSTRACTTOOL_H
#define MPLOTABSTRACTTOOL_H

#include "MPlotAxisScale.h"
#include <QGraphicsObject>

class MPlot;

/// MPlotTools are objects that can be added to an MPlot to allow different kinds of interaction with it.
/*! This class defines the interface for all plot tools. */
class MPlotAbstractTool : public QGraphicsObject
{
	Q_OBJECT
public:
	MPlotAbstractTool(const QRectF& geometry = QRectF(0,0,100,100));

	virtual ~MPlotAbstractTool();

	// isEnabled() and setEnabled(true/false) are used to enable or disable a tool's functionality.

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
	virtual void	mousePressEvent ( QGraphicsSceneMouseEvent * event ) = 0;
	virtual void	mouseMoveEvent ( QGraphicsSceneMouseEvent * event ) = 0;
	virtual void	mouseReleaseEvent ( QGraphicsSceneMouseEvent * event ) = 0;
	virtual void	wheelEvent ( QGraphicsSceneWheelEvent * event ) = 0;
	virtual void	mouseDoubleClickEvent ( QGraphicsSceneMouseEvent * event ) = 0;

	/// The plot we're attached to
	MPlot* plot_;
	/// the position and size of the tool, in drawing coordinates, relative to the MPlot::plotArea_.
	QRectF rect_;
	/// The axes we're active on.
	QList<MPlotAxisScale*> targetAxes_;

};

#endif // MPLOTABSTRACTTOOL_H
