#ifndef __MPlotBackground_H__
#define __MPlotBackground_H__

#include <QGraphicsRectItem>
#include <QObject>

#define MPLOT_BACKGROUND_ZVALUE -1000000000

class MPlotBackground : public QObject, public QGraphicsRectItem {

	Q_OBJECT

public:
	MPlotBackground(const QRectF & rect, QGraphicsItem * parent = 0 ) : QObject(), QGraphicsRectItem(rect, parent) {
		// put far back in the scene (ensure below everything):
		setZValue(MPLOT_BACKGROUND_ZVALUE);
	}
	
	virtual void mousePressEvent ( QGraphicsSceneMouseEvent * event ) {
		emit backgroundPressed();
		QGraphicsRectItem::mousePressEvent(event);
	}
	
signals:
	void backgroundPressed();	// notifies when a click has hit the background (useful for unselecting other objects, etc.)
	
};

#endif
