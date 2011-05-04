#ifndef MPLOTABSTRACTTOOL_CPP
#define MPLOTABSTRACTTOOL_CPP

#include "MPlotAbstractTool.h"

#include "MPlot.h"

MPlotAbstractTool::MPlotAbstractTool(const QRectF& geometry) :
	QGraphicsObject()
{
	plot_ = 0;
	rect_ = geometry;
}

MPlotAbstractTool::~MPlotAbstractTool() {
	if(plot()) {
		plot()->removeTool(this);
	}
}


#endif // MPLOTABSTRACTTOOL_CPP

