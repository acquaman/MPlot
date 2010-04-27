#ifndef __MPlotWidget_H__
#define __MPlotWidget_H__

#include <QGraphicsView>
#include "MPlot.h"
#include <QResizeEvent>
#include <QMouseEvent>
#include <QPainterPath>

#include <QDebug>

// TODO: test performance of:
// setItemIndexMethod(NoIndex);
// makes a big difference if drawing plots using many separate QGraphicsItem elements (for ex: separate QGraphicsLineItems for each line element in a series)


class MPlotWidget : public QGraphicsView {
    Q_OBJECT

public:
	MPlotWidget(QWidget* parent = 0) : QGraphicsView(parent) {
		
		setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		// TODO: research this:
		//setDragMode(QGraphicsView::RubberBandDrag);
		
		setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing /*| QPainter::HighQualityAntialiasing*/);
		

		// Create a scene to hold a plot:
		QGraphicsScene* scene = new QGraphicsScene(this);
		setScene(scene);

		// Not holding a plot right now:
		plot_ = 0;
	}
	
	
	virtual ~MPlotWidget() {
	}

	void setPlot(MPlot* plot) {

		if(plot) {

			// remove old plot?
			if(plot_)
				scene()->removeItem(plot_);

			// todo: disconnect any signals?

			scene()->addItem(plot);
			plot_ = plot;
		}
	}
	
	MPlot* plot() {
		return plot_;
	}
	

	



	
protected:
	// Member variables:
	MPlot* plot_;
	
	// On resize events: notify the plot to resize it, and fill the viewport with the canvas.
	virtual void resizeEvent ( QResizeEvent * event ) {
		QGraphicsView::resizeEvent(event);
		
		if(plot_) {
			plot_->setRect(QRectF(QPointF(0,0), event->size()));
			scene()->setSceneRect(plot_->rect());
			fitInView(plot_->rect(), Qt::KeepAspectRatioByExpanding);
		}
	}

};

#endif
