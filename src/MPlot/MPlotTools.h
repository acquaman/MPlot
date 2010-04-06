#ifndef MPLOTTOOLS_H
#define MPLOTTOOLS_H

#include "MPlot.h"
#include <QGraphicsSceneMouseEvent>

#include <QDebug>

// When selecting lines on plots with the mouse, this is how wide the selection ballpark is, in pixels. (Actually, in sceneCoordinates, but we prefer that you don't transform the view, so viewCoordinates = sceneCoordinates)
#define MPLOT_SELECTION_WIDTH 10

/// This is the width of the rubber-band cursor when selecting plot ranges:
#define MPLOT_RUBBERBAND_WIDTH 2


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


#include <QGraphicsRectItem>
#include <QStack>

/// This class provides a plot tool that can be used to select a single series in a plot:
class MPlotDragZoomerTool : public MPlotAbstractTool {
	Q_OBJECT
public:
	MPlotDragZoomerTool() : MPlotAbstractTool() {

		selectionRect_ = new QGraphicsRectItem(QRectF(), this);

		QPen selectionPen = QPen(QBrush(MPLOT_SELECTION_COLOR), MPLOT_RUBBERBAND_WIDTH);
		selectionPen.setCosmetic(true);

		selectionRect_->setPen(selectionPen);

		QColor brushColor = MPLOT_SELECTION_COLOR;
		brushColor.setAlphaF(0.35);
		selectionRect_->setBrush(brushColor);

		dragInProgress_ = false;
	}


signals:


protected:

	virtual void	mousePressEvent ( QGraphicsSceneMouseEvent * event ) {

		if(event->button() == Qt::LeftButton) {
			dragInProgress_ = true;
			selectionRect_->setRect(QRectF(event->buttonDownPos(Qt::LeftButton), event->buttonDownPos(Qt::LeftButton)));
			// Disable auto-scaling... we're taking over manual control...
			plot()->enableAutoScaleBottom(false);
			if(yAxisTarget_ == MPlotAxis::Right)
				plot()->enableAutoScaleRight(false);
			else
				plot()->enableAutoScaleLeft(false);
		}


	}

	virtual void	mouseMoveEvent ( QGraphicsSceneMouseEvent * event ) {
		if(dragInProgress_) {
			selectionRect_->setRect(QRectF(event->buttonDownPos(Qt::LeftButton), event->pos()));
		}
	}

	virtual void	mouseReleaseEvent ( QGraphicsSceneMouseEvent * event ) {
		if(dragInProgress_ && event->button() == Qt::LeftButton) {
			selectionRect_->setRect(QRectF());
			dragInProgress_ = false;

			QRectF oldZoom, newZoom;
			// Get old axis coordinates:
			if(yAxisTarget_ == MPlotAxis::Right) {
				oldZoom = QRectF(QPointF(plot()->xMin(), plot()->yRightMin()), QPointF(plot()->xMax(), plot()->yRightMax()));
				zoomStack_.push(oldZoom);

				QPointF new1(event->buttonDownPos(Qt::LeftButton).x(), event->buttonDownPos(Qt::LeftButton).y());
				QPointF new2(event->pos().x(), event->pos().y());
				newZoom = QRectF(new1, new2);
				newZoom = plot()->rightAxisTransform().inverted().mapRect(newZoom);
				plot()->setXDataRange(qMin(newZoom.left(), newZoom.right()), qMax(newZoom.left(), newZoom.right()));
				plot()->setYDataRangeRight(qMin(newZoom.bottom(), newZoom.top()), qMax(newZoom.bottom(), newZoom.top()));
			}
			else {
				oldZoom = QRectF(QPointF(plot()->xMin(), plot()->yLeftMin()), QPointF(plot()->xMax(), plot()->yLeftMax()));
				zoomStack_.push(oldZoom);

				QPointF new1(qMin(event->buttonDownPos(Qt::LeftButton).x(), event->pos().x()), qMin(event->buttonDownPos(Qt::LeftButton).y(), event->pos().y()));
				QPointF new2(qMax(event->buttonDownPos(Qt::LeftButton).x(), event->pos().x()), qMax(event->buttonDownPos(Qt::LeftButton).y(), event->pos().y()));
				newZoom = QRectF(new1, new2);
				newZoom = plot()->leftAxisTransform().inverted().mapRect(newZoom);
				plot()->setXDataRange(qMin(newZoom.left(), newZoom.right()), qMax(newZoom.left(), newZoom.right()));
				plot()->setYDataRangeLeft(qMin(newZoom.bottom(), newZoom.top()), qMax(newZoom.bottom(), newZoom.top()));

			}
			qDebug() << "zoom to rect:" << newZoom;
		}

		// Right mouse button let's you go back
		if(!dragInProgress_ && event->button() == Qt::RightButton) {
			// If we have old zoom settings to go back to:
			if(zoomStack_.count() > 0) {
				QRectF newZoom = zoomStack_.pop();
				plot()->setXDataRange(qMin(newZoom.left(), newZoom.right()), qMax(newZoom.left(), newZoom.right()));
				if(yAxisTarget_ == MPlotAxis::Right)
					plot()->setYDataRangeRight(qMin(newZoom.bottom(), newZoom.top()), qMax(newZoom.bottom(), newZoom.top()));
				else
					plot()->setYDataRangeLeft(qMin(newZoom.bottom(), newZoom.top()), qMax(newZoom.bottom(), newZoom.top()));
			}
			// otherwise go back to auto-scale:
			else {
				plot()->enableAutoScaleBottom(true);
				if(yAxisTarget_ == MPlotAxis::Right)
					plot()->enableAutoScaleRight(true);
				else
					plot()->enableAutoScaleLeft(true);
			}
		}
	}

	virtual void	wheelEvent ( QGraphicsSceneWheelEvent * event ) { QGraphicsObject::wheelEvent(event); }
	virtual void	mouseDoubleClickEvent ( QGraphicsSceneMouseEvent * event ) { QGraphicsObject::mouseDoubleClickEvent(event); }

	QGraphicsRectItem* selectionRect_;
	QStack<QRectF> zoomStack_;
	bool dragInProgress_;

};


#endif // MPLOTTOOLS_H
