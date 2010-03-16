#ifndef __MPlotWindow_H__
#define __MPlotWindow_H__

#include <QGraphicsView>
#include "MPlot.h"
#include <QResizeEvent>
#include <QMouseEvent>
#include <QPainterPath>

#include <QDebug>

// When selecting lines on plots with the mouse, this is how wide the selection ballpark is, in pixels
#define MPLOT_SELECTION_WIDTH 10
// This is the color of the selection highlight
#define MPLOT_SELECTION_COLOR QColor(255, 210, 129)
#define MPLOT_SELECTION_HIGHLIGHT_ZVALUE -1000

class MPlotWindow : public QGraphicsView {
    Q_OBJECT

public:
	MPlotWindow(QWidget* parent = 0) : QGraphicsView(parent) {
		
		setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		// TODO: research this:
		//setDragMode(QGraphicsView::RubberBandDrag);
		
		setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
		
		selectedSeries_ = 0;
		connect(this, SIGNAL(seriesSelected(MPlotSeries*)), this, SLOT(onSeriesSelected(MPlotSeries*)));
		connect(this, SIGNAL(deselected()), this, SLOT(onDeselected()));
		
		// Prepare the plot series we use to draw highlights:
		highlightSeries_ = new MPlotSeries;
		QPen linePen(QBrush(MPLOT_SELECTION_COLOR), 10);
		highlightSeries_->setLinePen(linePen);
		highlightSeries_->setMarkerShape(MPlotMarkerShape::None);
		highlightSeries_->setZValue(MPLOT_SELECTION_HIGHLIGHT_ZVALUE);

	}
	
	virtual ~MPlotWindow() {
		// TODO: problem: if a plot is highlighted when the program quits, it will be deleted automatically by its parent QGraphicsItem (the MPlot)
		// If it's NOT highlighted, then we need to delete it.  Oops...
//		if(highlightSeries_) {
//			delete highlightSeries_;
//			highlightSeries_ = 0;
//		}
	}

	void setPlot(MPlot* plot) { 
		if(plot) {
			setScene(plot);
		}
	}
	
	MPlot* plot() {
		return qobject_cast<MPlot*>(scene());
	}
	
	// Returns the currently-selected series in the plot (0 if none).
	MPlotSeries* selectedSeries() { return selectedSeries_; }
	

signals:
	// Emitted by the view when a series in the plot is selected
		// Note that selection happens on a per-view basis.  The same series can be in multiple views
	void seriesSelected(MPlotSeries*);
	// Emitted when nothing is selected:
	void deselected();
	
	
protected slots:
	
	// These are used to draw a highlight (ideally specifically within this view) when a series is selected:
	void onSeriesSelected(MPlotSeries* newSeries) {
		qDebug() << newSeries->objectName() << " selected.";
		plot()->removeSeries(highlightSeries_);
		highlightSeries_->setModel(newSeries->model());
		highlightSeries_->setYAxisTarget(newSeries->yAxisTarget());
		plot()->addSeries(highlightSeries_);
	}
	
	void onDeselected() {
		qDebug() << "deselected";
		plot()->removeSeries(highlightSeries_);
	}
	
	
protected:
	// Member variables:
	MPlotSeries* selectedSeries_;
	MPlotSeries* highlightSeries_;	// used to draw highlights over selected plots
	
	// On resize events: notify the canvas if the aspect ratio needs to change, and fill the viewport with the canvas.
	virtual void resizeEvent ( QResizeEvent * event ) {
		QGraphicsView::resizeEvent(event);
		
		if(plot()) {
			plot()->setAspectRatio( double(event->size().height()) / event->size().width() );
		
			fitInView(plot()->sceneRect(), Qt::KeepAspectRatioByExpanding);
		}
	}
	
	// This is used to detect PlotSeries selection. Selects a maximum of one series.
	// TODO: if multiple series within selection range, pop up a selection menu?
	virtual void mousePressEvent ( QMouseEvent * event ) {
		
		MPlotSeries* newSeries = 0;
		
		// Construct a QPainterPath region "in the ballpark" of the mouse click:
		QPainterPath clickRegion;
		clickRegion.addEllipse(event->pos(), MPLOT_SELECTION_WIDTH, MPLOT_SELECTION_WIDTH);
		
		// Get the list of items at this location:
		QList<QGraphicsItem*> itemList = items(clickRegion, Qt::IntersectsItemShape);	// QT BUG REPORT... with this option for Qt::IntersectsItemShape...
		
		// Check all items within range...
		foreach(QGraphicsItem* item, itemList) {
			
			// This check should not be necessary, but Qt 4.6 is using the boundingBox() instead of shape, even when Qt::IntersectsItemShape is used.
				// Have to verify that we actually intersect the shape...
			QTransform dt = item->deviceTransform(this->viewportTransform());			// QT BUG REPORT... this line should not be required
			if(!clickRegion.intersects(dt.map(item->shape())))							// QT BUG REPORT... this line should not be required
				continue;
			
			// Is it actually a MPlotSeries? (not just the background?)
			newSeries = dynamic_cast<MPlotSeries*>(item);
			if(newSeries)
				break;	// just found one
		}
		
		// If we found one, and it's not the same as the old one:
		if(newSeries && newSeries != selectedSeries_) {
			emit seriesSelected(selectedSeries_ = newSeries);
		}
			
		// If the click didn't land on any series, and there was one previously selected:
		if(!newSeries && selectedSeries_) {
			selectedSeries_ = 0;
			emit deselected();
		}
		
	}

};

#endif
