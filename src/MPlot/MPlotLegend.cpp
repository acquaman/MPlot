#ifndef __MPlotLegend_CPP__
#define __MPlotLegend_CPP__

#include "MPlotLegend.h"
QRectF MPlotLegend::boundingRect() const {
	return childrenBoundingRect();
}

void MPlotLegend::paint(QPainter * /*painter*/,
					  const QStyleOptionGraphicsItem * /*option*/,
					  QWidget * /*widget*/) {
	// Do nothing... drawn with children
}

#endif
