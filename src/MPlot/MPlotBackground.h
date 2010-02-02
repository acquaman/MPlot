#ifndef __MPlotBackground_H__
#define __MPlotBackground_H__

#include <QGraphicsRectItem>

class MPlotBackground : public QGraphicsRectItem {


public:
	MPlotBackground(const QRectF & rect, QGraphicsItem * parent = 0 ) : QGraphicsRectItem(rect, parent) {
	}
};

#endif
