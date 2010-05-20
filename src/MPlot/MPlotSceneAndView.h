#ifndef MPLOTSCENEANDVIEW_H
#define MPLOTSCENEANDVIEW_H

#include <QGraphicsView>
#include <QResizeEvent>

class MPlotSceneAndView : public QGraphicsView {
	Q_OBJECT

public:
	MPlotSceneAndView(QWidget* parent = 0) : QGraphicsView(parent) {

		setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

		setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing /*| QPainter::HighQualityAntialiasing*/);


		// Create a scene that we will link to the size of the view:
		QGraphicsScene* scene = new QGraphicsScene(this);
		setScene(scene);

	}


	virtual ~MPlotSceneAndView() {
		QGraphicsScene* scene = this->scene();
		setScene(0);
		delete scene;
	}

protected:

	// On resize events: notify the plot to resize it, and fill the viewport with the canvas.
	virtual void resizeEvent ( QResizeEvent * event ) {
		QGraphicsView::resizeEvent(event);

		scene()->setSceneRect(QRectF(QPointF(0,0), event->size()));
	}

};


#endif // MPLOTSCENEANDVIEW_H
