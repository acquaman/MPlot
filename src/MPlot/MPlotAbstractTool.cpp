#ifndef MPLOTABSTRACTTOOL_CPP
#define MPLOTABSTRACTTOOL_CPP

#include "MPlotAbstractTool.h"

#include "MPlot.h"

MPlotAbstractTool::MPlotAbstractTool() :
		QGraphicsObject()
{
	plot_ = 0;
	axisTargets_ = MPlotAxis::Left | MPlotAxis::Right | MPlotAxis::Bottom;
}

MPlotAbstractTool::~MPlotAbstractTool() {
	if(plot()) {
		plot()->removeTool(this);
	}
}

// isEnabled() and setEnabled(true/false) are used to enable or disable a tool's functionality.

/// Some tools don't need to draw anything. This provides an empty paint() function. Reimplement if drawing is needed.
void MPlotAbstractTool::paint ( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget) {
	Q_UNUSED(painter)
	Q_UNUSED(option)
	Q_UNUSED(widget)
}

/// boundingRect contains all contents drawn by this tool.
/*! Most tools live in the plotArea from (0,0) to (1,1). Reimplement if this is not the case. */
QRectF	MPlotAbstractTool::boundingRect () const {
	return QRectF(0, 0, 1, 1);
}

/// Don't call this. Unfortunately public because it's required by MPlot::addTool and MPlot::removeTool.
void MPlotAbstractTool::setPlot(MPlot* plot) {
	plot_ = plot;
}

MPlot* MPlotAbstractTool::plot() const {
	return plot_;
}

/// Some plot tools might be able to operate on a single axis, or a combination of them. This is an OR-combination of MPlotAxis::AxisID flags for all the axes that this tool affects.
int MPlotAbstractTool::axisTargets() const {
	return axisTargets_;
}

void MPlotAbstractTool::setAxisTargets(int axisTargets) {
	axisTargets_ = axisTargets;
}

#endif // MPLOTABSTRACTTOOL_CPP

