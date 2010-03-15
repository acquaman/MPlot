#ifndef __MPlotWindow_H__
#define __MPlotWindow_H__

#include <QGraphicsView>
#include "MPlot.h"
#include <QResizeEvent>

class MPlotWindow : public QGraphicsView {
    Q_OBJECT

public:
	MPlotWindow(QWidget* parent = 0) : QGraphicsView(parent) {
		
		setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		// TODO: research this:
		//setDragMode(QGraphicsView::RubberBandDrag);
		
		setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
	}

	void setPlot(MPlot* plot) { 
		if(plot) {
			setScene(plot);
		}
	}
	
	MPlot* plot() {
		return qobject_cast<MPlot*>(scene());
	}
	
	
protected:
	// On resize events: notify the canvas if the aspect ratio needs to change, and fill the viewport with the canvas.
	virtual void resizeEvent ( QResizeEvent * event ) {
		QGraphicsView::resizeEvent(event);
		
		if(plot()) {
			plot()->setAspectRatio( double(event->size().height()) / event->size().width() );
		
			fitInView(plot()->sceneRect(), Qt::KeepAspectRatioByExpanding);
		}
	}

};

#endif
