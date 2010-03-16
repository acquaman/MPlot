#ifndef __MPlotWindow_H__
#define __MPlotWindow_H__

#include <QGraphicsView>
#include "MPlot.h"
#include <QResizeEvent>
#include <QMouseEvent>
#include <QPainterPath>

#include <QDebug>

// Note on versions:
// GraphicsView in Qt 4.6.0 gets very slow with >= 10,000 items added to the scene.  This is due to a performance bug in QGraphicsScenePrivate::_q_polishItems,
// which is fixed in 4.6.1.  This library will build/work with all versions of Qt 4, but recommend using Qt >= 4.6.1.

// When selecting lines on plots with the mouse, this is how wide the selection ballpark is, in pixels
#define MPLOT_SELECTION_WIDTH 10

class MPlotWindow : public QGraphicsView {
    Q_OBJECT

public:
	MPlotWindow(QWidget* parent = 0) : QGraphicsView(parent) {
		
		setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		// TODO: research this:
		//setDragMode(QGraphicsView::RubberBandDrag);
		
		setRenderHints(/*QPainter::Antialiasing |*/ QPainter::TextAntialiasing);
		
		selectedSeries_ = 0;
		connect(this, SIGNAL(seriesSelected(MPlotAbstractSeries*)), this, SLOT(onSeriesSelected(MPlotAbstractSeries*)));
		connect(this, SIGNAL(deselected()), this, SLOT(onDeselected()));
	}
	
	
	virtual ~MPlotWindow() {
	}

	void setPlot(MPlot* plot) { 
		if(plot) {
			// todo: disconnect any signals? detach highlightSeries_ from old plot?
			setScene(plot);
		}
	}
	
	MPlot* plot() {
		return qobject_cast<MPlot*>(scene());
	}
	
	// Returns the currently-selected series in the plot (0 if none).
	MPlotAbstractSeries* selectedSeries() { return selectedSeries_; }
	

signals:
	// Emitted by the view when a series in the plot is selected
		// Note that selection happens on a per-view basis.  The same series can be in multiple views
	void seriesSelected(MPlotAbstractSeries*);
	// Emitted when nothing is selected:
	void deselected();
	
	
protected slots:
	
	void onSeriesSelected(MPlotAbstractSeries* newSeries) {
		qDebug() << newSeries->objectName() << " selected.";
		// Notify the plot (useful for drawing highlights, etc.)
		plot()->onSeriesSelected(newSeries);
	}
	
	void onDeselected() {
		qDebug() << "deselected";
		// Notify the plot (useful for removing highlights, etc.)
		plot()->onDeselected();
	}
	
	
	
protected:
	// Member variables:
	MPlotAbstractSeries* selectedSeries_;
	
	// On resize events: notify the canvas if the aspect ratio needs to change, and fill the viewport with the canvas.
	virtual void resizeEvent ( QResizeEvent * event ) {
		QGraphicsView::resizeEvent(event);
		
		if(plot()) {
			plot()->setAspectRatio( double(event->size().height()) / event->size().width() );
		
			fitInView(plot()->sceneRect(), Qt::KeepAspectRatioByExpanding);
		}
	}
	
	// This is used to detect PlotSeries selection.
	// If multiple series are on top of each other (or are within the selection range), this will alternate between them on successive clicks.
	virtual void mousePressEvent ( QMouseEvent * event ) {
		
		static unsigned int selIndex = 0;	// If two or more series are on top of each other, this is used to alternate between them
		MPlotAbstractSeries* s;
		QList<MPlotAbstractSeries*> selectedPossibilities;	// this will become a filtered list containing all the MPlotAbstractSeries that are in range from this click.
		
		// Construct a QPolygon "in the ballpark" of the mouse click: (switching from QPainterPath to QPolygon for performance)
		QRect clickRegion(event->pos().x()-MPLOT_SELECTION_WIDTH, event->pos().y()-MPLOT_SELECTION_WIDTH, 2*MPLOT_SELECTION_WIDTH, 2*MPLOT_SELECTION_WIDTH);
		//clickRegion.addEllipse(event->pos(), MPLOT_SELECTION_WIDTH, MPLOT_SELECTION_WIDTH);
		
		// Get the list of items at this location:
		QList<QGraphicsItem*> itemList = items(clickRegion, Qt::IntersectsItemShape);	// QT BUG REPORT... with this option for Qt::IntersectsItemShape...
		
		// Check all items within range...
		foreach(QGraphicsItem* item, itemList) {
			
			// This check should not be necessary, but Qt 4.6 is using the boundingBox() instead of shape, even when Qt::IntersectsItemShape is used.
				// Have to verify that we actually intersect the shape...
			QTransform dt = item->deviceTransform(this->viewportTransform());			// QT BUG REPORT... this line should not be required
			if(dt.map(item->shape()).intersects(clickRegion)) {							// QT BUG REPORT... this line should not be required

				// Is it actually a MPlotAbstractSeries? (not just the background, or some other GraphicsItem?
				s = dynamic_cast<MPlotAbstractSeries*>(item);
				if(s && s->objectName() != "MPLOT_HIGHLIGHT")
					selectedPossibilities << s;	// add it to the list of selected possibilities
			}
			else
				qDebug() << "should not happen... Qt bug using boundingBox() instead of shape() in items(region, Qt::IntersectsItemShape).";
		}
		
		// select from the list of possibilities using selIndex.  If there aren't any, s=0.
		if(selectedPossibilities.count() > 0)
			s = selectedPossibilities.at( (selIndex++) % selectedPossibilities.count() );
		else
			s = 0;
		
		// If we found one, and it's not the same as the old one:
		if(s && s != selectedSeries_) {
			emit seriesSelected(selectedSeries_ = s);
		}
			
		// If the click didn't land on any series, and there was one previously selected:
		if(!s && selectedSeries_) {
			selectedSeries_ = 0;
			emit deselected();
		}
		
	}

};

#endif
