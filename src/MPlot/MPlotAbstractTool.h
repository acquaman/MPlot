#ifndef MPLOTABSTRACTTOOL_H
#define MPLOTABSTRACTTOOL_H

#include "MPlotAxis.h"
#include <QGraphicsObject>

class MPlot;

/// MPlotTools are objects that can be added to an MPlot to allow different kinds of interaction with it.
/*! This class defines the interface for all plot tools. */
class MPlotAbstractTool : public QGraphicsObject
{
	Q_OBJECT
public:
	MPlotAbstractTool() : QGraphicsObject() {
		plot_ = 0;
		yAxisTarget_ = MPlotAxis::Left;
	}

	// isEnabled() and setEnabled(true/false) are used to enable or disable a tool's functionality.

	/// Some tools don't need to draw anything. This provides an empty paint() function. Reimplement if drawing is needed.
	virtual void	paint ( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0 ) {
		Q_UNUSED(painter)
		Q_UNUSED(option)
		Q_UNUSED(widget)
	}

	/// boundingRect contains all contents drawn by this tool.
	/*! Most tools live in the plotArea from (0,0) to (1,1). Reimplement if this is not the case. */
	virtual QRectF	boundingRect () const { return QRectF(0, 0, 1, 1); }

	/// Don't call this. Unfortunately public because it's required by MPlot::addTool and MPlot::removeTool.
	void setPlot(MPlot* plot) { plot_ = plot; }
	MPlot* plot() const { return plot_; }

	MPlotAxis::AxisID yAxisTarget() const { return yAxisTarget_; }
	void setYAxisTarget(MPlotAxis::AxisID yAxisTarget) { yAxisTarget_ = yAxisTarget; }


protected:
	virtual void	mousePressEvent ( QGraphicsSceneMouseEvent * event ) = 0;
	virtual void	mouseMoveEvent ( QGraphicsSceneMouseEvent * event ) = 0;
	virtual void	mouseReleaseEvent ( QGraphicsSceneMouseEvent * event ) = 0;
	virtual void	wheelEvent ( QGraphicsSceneWheelEvent * event ) = 0;
	virtual void	mouseDoubleClickEvent ( QGraphicsSceneMouseEvent * event ) = 0;

	MPlot* plot_;

	MPlotAxis::AxisID yAxisTarget_;

};

#endif // MPLOTABSTRACTTOOL_H
