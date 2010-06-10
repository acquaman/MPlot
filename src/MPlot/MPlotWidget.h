#ifndef __MPlotWidget_H__
#define __MPlotWidget_H__

#include <QGraphicsView>
#include <QResizeEvent>
#include "MPlot.h"

// TODO: test performance of:
// setItemIndexMethod(NoIndex);
// makes a big difference if drawing plots using many separate QGraphicsItem elements (for ex: separate QGraphicsLineItems for each line element in a series)


class MPlotSceneAndView : public QGraphicsView {
	Q_OBJECT

public:
	MPlotSceneAndView(QWidget* parent = 0) : QGraphicsView(parent) {

		setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

		enableAntiAliasing();


		// Create a scene that we will link to the size of the view:
		QGraphicsScene* scene = new QGraphicsScene(this);
		setScene(scene);

	}

	void enableAntiAliasing(bool antiAliasingOn = true) {
		if(antiAliasingOn)
			setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing /*| QPainter::HighQualityAntialiasing*/);
		else
			setRenderHints(QPainter::TextAntialiasing);
	}


	virtual ~MPlotSceneAndView() {
		QGraphicsScene* scene = this->scene();
		setScene(0);
		delete scene;
	}

protected:

	// On resize events: keep the scene the same size as the view, and make the view look at this part of the scene.
	virtual void resizeEvent ( QResizeEvent * event ) {
		QGraphicsView::resizeEvent(event);

		scene()->setSceneRect(QRectF(QPointF(0,0), event->size()));
		setSceneRect(scene()->sceneRect());
	}

};


class MPlotWidget : public MPlotSceneAndView {
    Q_OBJECT

public:
	MPlotWidget(QWidget* parent = 0) : MPlotSceneAndView(parent) {
		// Not holding a plot right now:
		plot_ = 0;
	}
	
	
	virtual ~MPlotWidget() {
	}

	/// Sets the plot attached to this widget. to remove a plot, pass \c plot = 0.
	void setPlot(MPlot* plot) {

		// remove old plot?
		if(plot_)
			scene()->removeItem(plot_);
		// todo: disconnect any signals?

		if(plot) {
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
		MPlotSceneAndView::resizeEvent(event);
		
		if(plot_) {
			plot_->setRect(scene()->sceneRect());
		}
	}

};

#endif
