#ifndef MPLOTABSTRACTTOOL_CPP
#define MPLOTABSTRACTTOOL_CPP

#include "MPlot/MPlotAbstractTool.h"

#include "MPlot/MPlot.h"

MPlotAbstractTool::MPlotAbstractTool(const QString &name, const QString &description, const QRectF& geometry) :
	QGraphicsObject()
{
	plot_ = 0;
	rect_ = geometry;
	name_ = name;
	description_ = description;
}

MPlotAbstractTool::~MPlotAbstractTool() {
	if(plot()) {
		plot()->removeTool(this);
	}
}


#endif // MPLOTABSTRACTTOOL_CPP

