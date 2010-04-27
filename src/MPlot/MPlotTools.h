#ifndef MPLOTTOOLS_H
#define MPLOTTOOLS_H

#include "MPlot.h"
#include <QGraphicsSceneMouseEvent>

#include <QDebug>

/// When selecting lines on plots with the mouse, this is how wide the selection ballpark is, in pixels. (Actually, in sceneCoordinates, but we prefer that you don't transform the view, so viewCoordinates = sceneCoordinates)
#define MPLOT_SELECTION_BALLPARK 10

/// This is the width of the rubber-band cursor when selecting plot ranges:
#define MPLOT_RUBBERBAND_WIDTH 2

/// This is the distance (in pixels/scene coordinates) that a user must drag with the MPlotDragZoomerTool before it becomes active. This is to avoid a "click" or a "click and very small drag" causing an inadvertent zoom into a very small rectangle.
#define MPLOT_RUBBERBAND_DEADZONE 6

/// The opacity level (0=transparent, 1=opaque) of selection rectangles:
#define MPLOT_SELECTION_OPACITY 0.35


/// This class provides a plot tool that can be used to select a single data-item in a plot.
class MPlotPlotSelectorTool : public MPlotAbstractTool {
	Q_OBJECT
public:
	MPlotPlotSelectorTool() : MPlotAbstractTool() {
		selectedItem_ = 0;
	}

	/// Returns the currently-selected item in the plot (0 if none).
	MPlotItem* selectedItem() { return selectedItem_; }

signals:
	// Emitted by the view when an item in the plot is selected
		// Note that selection happens on a per-scene basis.  The same item can be in multiple views, and will be selected in multiple views.
	void itemSelected(MPlotItem*);
	// Emitted when nothing is selected:
	void deselected();

protected:
	// This is used to detect PlotItem selection.
	// If multiple items are on top of each other (or are within the selection range), this will alternate between them on successive clicks.

	virtual void	mousePressEvent ( QGraphicsSceneMouseEvent * event ) {

		static unsigned int selIndex = 0;	// If two or more items are on top of each other, this is used to alternate between them
		MPlotItem* s;
		QList<MPlotItem*> selectedPossibilities;	// this will become a filtered list containing all the MPlotItem that are in range from this click.

		// Construct a rectangle "in the ballpark" of the mouse click:
		QRectF clickRegion(event->scenePos().x()-MPLOT_SELECTION_BALLPARK, event->scenePos().y()-MPLOT_SELECTION_BALLPARK, 2*MPLOT_SELECTION_BALLPARK, 2*MPLOT_SELECTION_BALLPARK);

		// Check all items for intersections
		foreach(MPlotItem* s2, plot()->plotItems() ) {

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
		if(s && s != selectedItem_) {
			// tell the old item to unselect:
			if(selectedItem_)
				selectedItem_->setSelected(false);
			// Tell the new one to select:
			s->setSelected(true);
			// Assign, and emit signal:
			emit itemSelected(selectedItem_ = s);
		}

		// If the click didn't land on any item, and there was one previously selected:
		if(!s && selectedItem_) {
			// Tell the old one to unselect:
			selectedItem_->setSelected(false);
			selectedItem_ = 0;
			emit deselected();
		}

		// ignore the mouse press event, so that it will be propagated to other tools below us:
		event->ignore();

	}

	virtual void	mouseMoveEvent ( QGraphicsSceneMouseEvent * event ) { QGraphicsObject::mouseMoveEvent(event); }
	virtual void	mouseReleaseEvent ( QGraphicsSceneMouseEvent * event ) { QGraphicsObject::mouseReleaseEvent(event); }
	virtual void	wheelEvent ( QGraphicsSceneWheelEvent * event ) { QGraphicsObject::wheelEvent(event); }
	virtual void	mouseDoubleClickEvent ( QGraphicsSceneMouseEvent * event ) { QGraphicsObject::mouseDoubleClickEvent(event); }

	MPlotItem* selectedItem_;

};


#include <QGraphicsRectItem>
#include <QStack>

/// This class provides a plot tool that can be used to choose a selection rectangle with a "rubber-band", and zoom into that region. Right-clicking will restore the zoom to the previous state. (Infinite zoom/restore levels are available.)
/*! Limitation: when adding multiple MPlotTools to a plot, this one must be added first (ie: "on the bottom").  To track the mouse drag, it needs to become the QGraphicsScene::mouseGrabberItem(), which will prevent other tools from receiving mouse clicks if it is "on top" of them.
  */
class MPlotDragZoomerTool : public MPlotAbstractTool {
	Q_OBJECT
public:
	MPlotDragZoomerTool() : MPlotAbstractTool() {

		selectionRect_ = new QGraphicsRectItem(QRectF(), this);

		QPen selectionPen = QPen(QBrush(MPLOT_SELECTION_COLOR), MPLOT_RUBBERBAND_WIDTH);
		selectionPen.setCosmetic(true);

		selectionRect_->setPen(selectionPen);

		QColor brushColor = MPLOT_SELECTION_COLOR;
		brushColor.setAlphaF(MPLOT_SELECTION_OPACITY);
		selectionRect_->setBrush(brushColor);

		dragInProgress_ = false;
		dragStarted_ = false;
	}


signals:


protected:

	virtual void	mousePressEvent ( QGraphicsSceneMouseEvent * event ) {

		if(event->button() == Qt::LeftButton) {
			dragStarted_ = true;
			// don't display the rubberband rectangle until dragInProgress_
			// selectionRect_->setRect(QRectF(event->buttonDownPos(Qt::LeftButton), event->buttonDownPos(Qt::LeftButton)));
		}

	}

	/// Handles drag events, redraws the selection retangle to follow the mouse, and handles state transitions between dragStarted_ and dragInProgress_
	virtual void	mouseMoveEvent ( QGraphicsSceneMouseEvent * event ) {

		// Possible transition: A drag event has started, and the user exceeded the drag deadzone to count as a real drag.
		if(dragStarted_) {
			QPointF dragDistance = event->buttonDownScenePos(Qt::LeftButton) - event->scenePos();

			// if we've gone far enough, this counts as a real drag:
			if(dragDistance.manhattanLength() > MPLOT_RUBBERBAND_DEADZONE) {
				// flag drag event in progress
				dragInProgress_ = true;
				dragStarted_ = false;

				// Disable auto-scaling on the plot... the user probably wants to take over manual control...
				plot()->enableAutoScaleBottom(false);
				if(yAxisTarget_ == MPlotAxis::Right)
					plot()->enableAutoScaleRight(false);
				else
					plot()->enableAutoScaleLeft(false);
			}
		}

		// If we're dragging, draw/update the selection rectangle.
		// Figure out why this gets slow for high zoom settings.
		// attempted: Only do this if we've moved a true pixel, otherwise this gets really slow for high zoom settings.
		if(dragInProgress_ /*&& (event->buttonDownScenePos(Qt::LeftButton) - event->scenePos()).manhattanLength() > 1*/ ) {
			selectionRect_->setRect(QRectF(event->buttonDownPos(Qt::LeftButton), event->pos()));
		}
	}

	/// Handles release events. If a drag was in progress and the user lets go of the left button, zoom to the new rectangle and save the old one on the recall stack.  If the user lets go of the right button, this is a restore to a zoom position on the stack.
	virtual void	mouseReleaseEvent ( QGraphicsSceneMouseEvent * event ) {

		// left mouse button release: drag event is done.
		if(event->button() == Qt::LeftButton) {

			dragStarted_ = false;
			// disable the selection rectangle:
			selectionRect_->setRect(QRectF());

			if(dragInProgress_) {
				// This is a zoom change!
				dragInProgress_ = false;

				QRectF oldZoom, newZoom;

				// Zoom either the left y-axis or the right y-axis, depending on our axis affiliation:
				if(yAxisTarget_ == MPlotAxis::Right) {
					// Get old axis coordinates:
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
		}

		// Right mouse button let's you go back to an old zoom setting
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

	/// Means that a click has happened, but we might not yet have exceeded the drag deadzone to count as a zoom drag event.
	bool dragStarted_;
	/// Means that a zoom drag event is currently happening. We're in between exceeding the drag deadzone and finishing the drag.
	bool dragInProgress_;

};


#endif // MPLOTTOOLS_H
