#ifndef __MPlotLegend_H__
#define __MPlotLegend_H__

#include <QGraphicsItem>

class MPlotLegend: public QGraphicsItem {

	// Required functions:
	// Bounding rect:
	virtual QRectF boundingRect() const { return childrenBoundingRect(); }
	// Paint:
	virtual void paint(QPainter * /*painter*/,
			   const QStyleOptionGraphicsItem * /*option*/,
			   QWidget * /*widget*/) {
		// Do nothing... drawn with children
	}

};

#endif
