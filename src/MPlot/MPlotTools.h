#ifndef MPLOTTOOLS_H
#define MPLOTTOOLS_H

#include "MPlot.h"
#include <QGraphicsSceneMouseEvent>

#include <QDebug>

// When selecting lines on plots with the mouse, this is how wide the selection ballpark is, in pixels. (Actually, in sceneCoordinates, but we prefer that you don't transform the view, so viewCoordinates = sceneCoordinates)
#define MPLOT_SELECTION_WIDTH 10


/// This class provides a plot tool that can be used to select a single series in a plot:
class MPlotPlotSelectorTool : public MPlotAbstractTool {
	Q_OBJECT
public:
	MPlotPlotSelectorTool() : MPlotAbstractTool() {
		selectedSeries_ = 0;
	}

	/// Returns the currently-selected series in the plot (0 if none).
	MPlotAbstractSeries* selectedSeries() { return selectedSeries_; }

signals:
	// Emitted by the view when a series in the plot is selected
		// Note that selection happens on a per-scene basis.  The same series can be in multiple views, and will be selected in multiple views.
	void seriesSelected(MPlotAbstractSeries*);
	// Emitted when nothing is selected:
	void deselected();

protected:
	// This is used to detect PlotSeries selection.
	// If multiple series are on top of each other (or are within the selection range), this will alternate between them on successive clicks.

	virtual void	mousePressEvent ( QGraphicsSceneMouseEvent * event ) {

		qDebug() << "press event: " << event;

		static unsigned int selIndex = 0;	// If two or more series are on top of each other, this is used to alternate between them
		MPlotAbstractSeries* s;
		QList<MPlotAbstractSeries*> selectedPossibilities;	// this will become a filtered list containing all the MPlotAbstractSeries that are in range from this click.

		// Construct a QPolygon "in the ballpark" of the mouse click: (switching from QPainterPath to QPolygon for performance)
		QRectF clickRegion(event->scenePos().x()-MPLOT_SELECTION_WIDTH, event->scenePos().y()-MPLOT_SELECTION_WIDTH, 2*MPLOT_SELECTION_WIDTH, 2*MPLOT_SELECTION_WIDTH);

		// Check all series for intersections
		foreach(MPlotAbstractSeries* s2, plot()->series() ) {

			// Have to verify that we actually intersect the shape...
			if(s2->shape().intersects(s2->mapRectFromScene(clickRegion))) {

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

	virtual void	mouseMoveEvent ( QGraphicsSceneMouseEvent * event ) { QGraphicsObject::mouseMoveEvent(event); }
	virtual void	mouseReleaseEvent ( QGraphicsSceneMouseEvent * event ) { QGraphicsObject::mouseReleaseEvent(event); }
	virtual void	wheelEvent ( QGraphicsSceneWheelEvent * event ) { QGraphicsObject::wheelEvent(event); }
	virtual void	mouseDoubleClickEvent ( QGraphicsSceneMouseEvent * event ) { QGraphicsObject::mouseDoubleClickEvent(event); }

	MPlotAbstractSeries* selectedSeries_;

};


#endif // MPLOTTOOLS_H
