#ifndef __MPlotWidget_CPP__
#define __MPlotWidget_CPP__

#include "MPlotWidget.h"

MPlotSceneAndView::MPlotSceneAndView(QWidget* parent) :
		QGraphicsView(parent)
{

	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	enableAntiAliasing();


	// Create a scene that we will link to the size of the view:
	QGraphicsScene* scene = new QGraphicsScene(this);
	setScene(scene);

}

void MPlotSceneAndView::enableAntiAliasing(bool antiAliasingOn) {
	if(antiAliasingOn)
		setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing /*| QPainter::HighQualityAntialiasing*/);
	else
		setRenderHints(QPainter::TextAntialiasing);
}

MPlotSceneAndView::~MPlotSceneAndView() {
	//QGraphicsScene* scene = this->scene();
	//setScene(0);

	if (scene())
		delete scene();
}

// On resize events: keep the scene the same size as the view, and make the view look at this part of the scene.
void MPlotSceneAndView::resizeEvent ( QResizeEvent * event ) {
	QGraphicsView::resizeEvent(event);

	scene()->setSceneRect(QRectF(QPointF(0,0), event->size()));
	setSceneRect(scene()->sceneRect());
}

MPlotWidget::MPlotWidget(QWidget* parent) :
		MPlotSceneAndView(parent)
{
	// Not holding a plot right now:
	plot_ = 0;
}

MPlotWidget::~MPlotWidget() {
}

// Sets the plot attached to this widget. to remove a plot, pass \c plot = 0.
void MPlotWidget::setPlot(MPlot* plot) {

	// remove old plot?
	if(plot_)
		scene()->removeItem(plot_);
	// todo: disconnect any signals?

	if(plot) {
		scene()->addItem(plot);
		plot_ = plot;
	}
}

MPlot* MPlotWidget::plot() {
	return plot_;
}

// On resize events: notify the plot to resize it, and fill the viewport with the canvas.
void MPlotWidget::resizeEvent ( QResizeEvent * event ) {
	MPlotSceneAndView::resizeEvent(event);

	if(plot_) {
		plot_->setRect(scene()->sceneRect());
	}
}

#endif

