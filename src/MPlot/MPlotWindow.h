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
		
		setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing /*| QPainter::HighQualityAntialiasing*/);
		
		selectedSeries_ = 0;
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
		// Note that selection happens on a per-scene basis.  The same series can be in multiple views, and will be selected in multiple views.
	void seriesSelected(MPlotAbstractSeries*);
	// Emitted when nothing is selected:
	void deselected();

	
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
		
		// Check all series for intersections
		foreach(MPlotAbstractSeries* s2, plot()->series() ) {
			
			// Have to verify that we actually intersect the shape...
			QTransform dt = s2->deviceTransform(this->viewportTransform());
			if(dt.map(s2->shape()).intersects(clickRegion)) {

				if(s2->objectName() != "MPLOT_HIGHLIGHT")
					selectedPossibilities << s2;	// add it to the list of selected possibilities
			}
		}
		
		// select from the list of possibilities using selIndex.  If there aren't any, s=0.
		if(selectedPossibilities.count() > 0)
			s = selectedPossibilities.at( (selIndex++) % selectedPossibilities.count() );
		else
			s = 0;
		
		// If we found one, and it's not the same as the old one:
		if(s && s != selectedSeries_) {
			// tell the old series to unselect:
			if(selectedSeries_)
				selectedSeries_->setSelected(false);
			// Tell the new one to select:
			s->setSelected(true);
			// Assign, and emit signal:
			emit seriesSelected(selectedSeries_ = s);
		}
			
		// If the click didn't land on any series, and there was one previously selected:
		if(!s && selectedSeries_) {
			// Tell the old one to unselect:
			selectedSeries_->setSelected(false);
			selectedSeries_ = 0;
			emit deselected();
		}
		
	}

};

#endif
