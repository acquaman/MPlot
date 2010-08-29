#ifndef MPLOTTOOLS_CPP
#define MPLOTTOOLS_CPP

#include "MPlotTools.h"


MPlotPlotSelectorTool::MPlotPlotSelectorTool() :
		MPlotAbstractTool()
{
	selectedItem_ = 0;
}

/// Returns the currently-selected item in the plot (0 if none).
MPlotItem* MPlotPlotSelectorTool::selectedItem() {
	return selectedItem_;
}


// This is used to detect PlotItem selection.
// If multiple items are on top of each other (or are within the selection range), this will alternate between them on successive clicks.

void MPlotPlotSelectorTool::mousePressEvent ( QGraphicsSceneMouseEvent * event ) {

	static unsigned int selIndex = 0;	// If two or more items are on top of each other, this is used to alternate between them
	MPlotItem* s;
	QList<MPlotItem*> selectedPossibilities;	// this will become a filtered list containing all the MPlotItem that are in range from this click.

	// Construct a rectangle "in the ballpark" of the mouse click:
	QRectF clickRegion(event->scenePos().x()-MPLOT_SELECTION_BALLPARK, event->scenePos().y()-MPLOT_SELECTION_BALLPARK, 2*MPLOT_SELECTION_BALLPARK, 2*MPLOT_SELECTION_BALLPARK);

	// Check all items for intersections
	foreach(MPlotItem* s2, plot()->plotItems() ) {

		// Have to verify that we actually intersect the shape... and that this guy is selectable
		if(s2->selectable() && s2->shape().intersects(s2->mapRectFromScene(clickRegion))) {

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

void MPlotPlotSelectorTool::mouseMoveEvent ( QGraphicsSceneMouseEvent * event ) {
	QGraphicsObject::mouseMoveEvent(event);
}

void MPlotPlotSelectorTool::mouseReleaseEvent ( QGraphicsSceneMouseEvent * event ) {
	QGraphicsObject::mouseReleaseEvent(event);
}

void MPlotPlotSelectorTool::wheelEvent ( QGraphicsSceneWheelEvent * event ) {
	QGraphicsObject::wheelEvent(event);
}

void MPlotPlotSelectorTool::mouseDoubleClickEvent ( QGraphicsSceneMouseEvent * event ) {
	QGraphicsObject::mouseDoubleClickEvent(event);
}




/// Constructor. By default, this tool operates on all axes (Left, Right, and Bottom), and adds/subtracts 25% to the axis range on each mousewheel click.  Use setZoomIncrement() and setYAxisTargets() to change these later.
MPlotWheelZoomerTool::MPlotWheelZoomerTool(double zoomIncrement, int axisTargets) :
		MPlotAbstractTool()
{

	setZoomIncrement(zoomIncrement);

	setAxisTargets(axisTargets);
}

/// returns the fraction of the axis scale that will be added/subtracted on each mouse wheel click. (0.25 = 25% by default)
double MPlotWheelZoomerTool::zoomIncrement() const {
	return zf_;
}

/// set the zoom increment. On every mousewheel click, the range of the axis will be increased or decreased by this fraction.
void MPlotWheelZoomerTool::setZoomIncrement(double zi) {
	zf_ = fabs(zi);
}


void MPlotWheelZoomerTool::mousePressEvent ( QGraphicsSceneMouseEvent * event ) {
	event->ignore();
	QGraphicsObject::mousePressEvent(event);
}

/// Handles drag events, redraws the selection retangle to follow the mouse, and handles state transitions between dragStarted_ and dragInProgress_
void MPlotWheelZoomerTool::mouseMoveEvent ( QGraphicsSceneMouseEvent * event ) {
	QGraphicsObject::mouseMoveEvent(event);
}

void MPlotWheelZoomerTool::mouseReleaseEvent ( QGraphicsSceneMouseEvent * event ) {
	QGraphicsObject::mouseReleaseEvent(event);
}

/// The wheel zoomer implements scroll-in scroll-out zooming under the mouse cursor.
/*!
		<b>Basic equations: zooming in while maintaining a datapoint under the mouse cursor </b>

		-\c x: x datapoint value
		-\c min: previous axis minimum
		-\c max: previous axis maximum
		-\c min': new axis minimum
		-\c max': new axis maximum
		-\c F: zoom scale factor (ex: 0.6)

		1) The new scale is smaller than previous one; ie: multiplied by factor F

		\f[
				(max' - min') = F (max - min)
		\f]

		2) Also distance from \c x to \c min, as fraction of total axis range, stays constant

		\f[
				(x-min)/(max-min) = (x-min')/(max'-min')
		\f]

		Combining [1] and [2], and algebra:

		\f[
				min' = x + F(min - x)
		\f]
		\f[
				max' = x + F(max - x)
		\f]


	  */
void MPlotWheelZoomerTool::wheelEvent ( QGraphicsSceneWheelEvent * event ) {


	// delta: mouse wheel rotation amount. 120 corresponds to 1 "click", or 15 degrees rotation on most mice.  Units are 1/8th of a degree.

	double F = (1 - qMin(zf_ * fabs(event->delta()) / 120, 0.9));

	// negative scrolling: we don't want a negative, we want the reciprocal: (zoom out, instead of in)
	if(event->delta() < 0)
		F = 1/F;

	if(axisTargets() & MPlotAxis::Left) {

		double y = plot()->leftAxisTransform().inverted().map(event->pos()).y();
		double newMin, newMax;
		newMin = y + F*(plot()->axisLeft()->min() - y);
		newMax = y + F*(plot()->axisLeft()->max() - y);

		plot()->setYDataRangeLeft(newMin, newMax, false, false);
		plot()->enableAutoScaleLeft(false);

	}

	if(axisTargets() & MPlotAxis::Right) {
		double y = plot()->rightAxisTransform().inverted().map(event->pos()).y();
		double newMin, newMax;
		newMin = y + F*(plot()->axisRight()->min() - y);
		newMax = y + F*(plot()->axisRight()->max() - y);

		plot()->setYDataRangeRight(newMin, newMax, false, false);
		plot()->enableAutoScaleRight(false);

	}

	if(axisTargets() & MPlotAxis::Bottom) {
		double x = plot()->leftAxisTransform().inverted().map(event->pos()).x();
		double newMin, newMax;
		newMin = x + F*(plot()->axisBottom()->min() - x);
		newMax = x + F*(plot()->axisBottom()->max() - x);

		plot()->setXDataRange(newMin, newMax, false, false);
		plot()->enableAutoScaleBottom(false);
	}


}

void MPlotWheelZoomerTool::mouseDoubleClickEvent ( QGraphicsSceneMouseEvent * event ) {
	QGraphicsObject::mouseDoubleClickEvent(event);
}



/// Constructor.  \c axisTargets specifies an OR combination of MPlotAxis::AxisID flags that set which axes this tool has zoom control over.
MPlotDragZoomerTool::MPlotDragZoomerTool(int axisTargets) :
		MPlotAbstractTool()
{

	selectionRect_ = new QGraphicsRectItem(QRectF(), this);

	QPen selectionPen = QPen(QBrush(MPLOT_SELECTION_COLOR), MPLOT_RUBBERBAND_WIDTH);
	selectionPen.setCosmetic(true);

	selectionRect_->setPen(selectionPen);

	QColor brushColor = MPLOT_SELECTION_COLOR;
	brushColor.setAlphaF(MPLOT_SELECTION_OPACITY);
	selectionRect_->setBrush(brushColor);

	dragInProgress_ = false;
	dragStarted_ = false;

	setAxisTargets(axisTargets);
}


void MPlotDragZoomerTool::mousePressEvent ( QGraphicsSceneMouseEvent * event ) {

	if(event->button() == Qt::LeftButton) {
		dragStarted_ = true;
		// don't display the rubberband rectangle until dragInProgress_
		// selectionRect_->setRect(QRectF(event->buttonDownPos(Qt::LeftButton), event->buttonDownPos(Qt::LeftButton)));
	}

}

/// Handles drag events, redraws the selection retangle to follow the mouse, and handles state transitions between dragStarted_ and dragInProgress_
void MPlotDragZoomerTool::mouseMoveEvent ( QGraphicsSceneMouseEvent * event ) {

	// Possible transition: A drag event has started, and the user exceeded the drag deadzone to count as a real drag.
	if(dragStarted_) {
		QPointF dragDistance = event->buttonDownScenePos(Qt::LeftButton) - event->scenePos();

		// if we've gone far enough, this counts as a real drag:
		if(dragDistance.manhattanLength() > MPLOT_RUBBERBAND_DEADZONE) {
			// flag drag event in progress
			dragInProgress_ = true;
			dragStarted_ = false;

			// Disable auto-scaling on the plot... the user probably wants to take over manual control...

			if(axisTargets() & MPlotAxis::Right)
				plot()->enableAutoScaleRight(false);
			if(axisTargets() & MPlotAxis::Left)
				plot()->enableAutoScaleLeft(false);
			if(axisTargets() & MPlotAxis::Bottom)
				plot()->enableAutoScaleBottom(false);
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
void MPlotDragZoomerTool::mouseReleaseEvent ( QGraphicsSceneMouseEvent * event ) {

	// left mouse button release: drag event is done.
	if(event->button() == Qt::LeftButton) {

		dragStarted_ = false;
		// disable the selection rectangle:
		selectionRect_->setRect(QRectF());

		if(dragInProgress_) {
			// This is a zoom change!
			dragInProgress_ = false;

			QRectF oldZoomLeft, oldZoomRight, newZoomLeft, newZoomRight;

			// Get old axis coordinates and compute new zoom for right axis. Don't apply yet -- need to maintain old scale in x until we process MPlotAxis::Left
			if(axisTargets() & MPlotAxis::Right) {

				oldZoomRight = QRectF(QPointF(plot()->xMin(), plot()->yRightMin()), QPointF(plot()->xMax(), plot()->yRightMax()));
				rightZoomStack_.push(oldZoomRight);

				QPointF new1(event->buttonDownPos(Qt::LeftButton).x(), event->buttonDownPos(Qt::LeftButton).y());
				QPointF new2(event->pos().x(), event->pos().y());
				newZoomRight = QRectF(new1, new2);
				newZoomRight = plot()->rightAxisTransform().inverted().mapRect(newZoomRight);
			}


			// Get the axis coordinates and compute new zoom. This one used for left axis and/or bottom axis (same calculation; both use newZoomLeft)
			if(axisTargets() & MPlotAxis::Left || axisTargets() & MPlotAxis::Bottom) {

				oldZoomLeft = QRectF(QPointF(plot()->xMin(), plot()->yLeftMin()), QPointF(plot()->xMax(), plot()->yLeftMax()));
				if(axisTargets() & MPlotAxis::Left)
					leftZoomStack_.push(oldZoomLeft);
				if(axisTargets() & MPlotAxis::Bottom)
					bottomZoomStack_.push(oldZoomLeft);

				QPointF new1(qMin(event->buttonDownPos(Qt::LeftButton).x(), event->pos().x()), qMin(event->buttonDownPos(Qt::LeftButton).y(), event->pos().y()));
				QPointF new2(qMax(event->buttonDownPos(Qt::LeftButton).x(), event->pos().x()), qMax(event->buttonDownPos(Qt::LeftButton).y(), event->pos().y()));
				newZoomLeft = QRectF(new1, new2);
				newZoomLeft = plot()->leftAxisTransform().inverted().mapRect(newZoomLeft);

			}


			// can now apply new zooms:
			if(axisTargets() & MPlotAxis::Left)
				plot()->setYDataRangeLeft(newZoomLeft.bottom(), newZoomLeft.top(), false, false);

			if(axisTargets() & MPlotAxis::Bottom)
				plot()->setXDataRange(newZoomLeft.left(), newZoomLeft.right(), false, false);

			if(axisTargets() & MPlotAxis::Right)
				plot()->setYDataRangeRight(newZoomRight.bottom(), newZoomRight.top(), false, false);

		}
	}

	// Right mouse button: let's you go back to an old zoom setting
	if(!dragInProgress_ && event->button() == Qt::RightButton) {

		// right axis enabled?
		if(axisTargets() & MPlotAxis::Right) {
			// If we have old zoom settings to go back to:
			if(rightZoomStack_.count() > 0) {
				QRectF newZoom = rightZoomStack_.pop();
				plot()->setYDataRangeRight(newZoom.bottom(), newZoom.top(), false, false);
			}
			// otherwise go back to auto-scale:
			else {
				plot()->enableAutoScaleRight(true);
			}
		}

		// left axis enabled:
		if(axisTargets() & MPlotAxis::Left) {
			// If we have old zoom settings to go back to:
			if(leftZoomStack_.count() > 0) {
				QRectF newZoom = leftZoomStack_.pop();
				plot()->setYDataRangeLeft(newZoom.bottom(), newZoom.top(), false, false);
			}
			else {
				plot()->enableAutoScaleLeft(true);
			}
		}

		// bottom axis enabled:
		if(axisTargets() & MPlotAxis::Bottom) {
			// If we have old zoom settings to go back to:
			if(bottomZoomStack_.count() > 0) {
				QRectF newZoom = bottomZoomStack_.pop();
				plot()->setXDataRange(newZoom.left(), newZoom.right(), false, false);
			}
			else {
				plot()->enableAutoScaleBottom(true);
			}
		}
	}
}


void MPlotDragZoomerTool::wheelEvent ( QGraphicsSceneWheelEvent * event ) {
	QGraphicsObject::wheelEvent(event);
}

void MPlotDragZoomerTool::mouseDoubleClickEvent ( QGraphicsSceneMouseEvent * event ) {
	QGraphicsObject::mouseDoubleClickEvent(event);
}


MPlotCursorTool::MPlotCursorTool(int axisTargets)
	: MPlotAbstractTool() {

	setAxisTargets(axisTargets);
	addCursor();
}

MPlotCursorTool::~MPlotCursorTool() {
	foreach(MPlotPoint* c, cursors_) {
		if(plot())
			plot()->removeItem(c);
		delete c;
	}
	cursors_.clear();
}

unsigned MPlotCursorTool::numCursors() const {
	return cursors_.count();
}

/// Returns the currently-selected item in the plot (0 if none).
QPointF MPlotCursorTool::value(unsigned cursorIndex) const {
	if(cursorIndex < numCursors())
		return cursors_.at(cursorIndex)->value();
	else
		return QPointF(0,0);
}

/// Returns the MPlotPoint used to represent a specific cursor, so you can adjust it's color, marker, etc, or place it manually using MPlotPoint::setValue().
MPlotPoint* MPlotCursorTool::cursor(unsigned cursorIndex) const {
	if(cursorIndex < numCursors())
		return cursors_.at(cursorIndex);
	else
		return 0;
}

/// remove a cursor. Note: you cannot remove the final cursor... there must always be 1.
void MPlotCursorTool::removeCursor() {
	if(numCursors() > 1) {
		MPlotPoint* removeMe = cursors_.takeLast();
		if(plot())
			plot()->removeItem(removeMe);
		delete removeMe;
	}
}

/// add a cursor.  Cursors are added to the center of the existing plot.
void MPlotCursorTool::addCursor(const QPointF& initialPos) {
	MPlotPoint* newCursor = new MPlotPoint();
	newCursor->setSelectable(false);
	newCursor->setMarker(MPlotMarkerShape::Cross, MPLOT_CURSOR_BIG_HACK);
	newCursor->setValue(initialPos);

	if(plot()) {
		plot()->addItem(newCursor);
	}
	cursors_ << newCursor;
}


MPlotAxis::AxisID MPlotCursorTool::yAxis() const {
	if(axisTargets() & MPlotAxis::Left)
		return MPlotAxis::Left;
	if(axisTargets() & MPlotAxis::Right)
		return MPlotAxis::Right;
	return MPlotAxis::Left;
}

void MPlotCursorTool::mousePressEvent ( QGraphicsSceneMouseEvent * event ) {

	if(event->button() == Qt::LeftButton) {
		static unsigned activeCursor = 0;

		unsigned c = activeCursor % numCursors();

		QPointF newPos;
		if(yAxis() == MPlotAxis::Right)
			newPos = plot()->rightAxisTransform().inverted().map(event->pos());
		else
			newPos = plot()->leftAxisTransform().inverted().map(event->pos());

		/// \todo clean this up... If a cursor was added prior to this tool being assigned to a plot, it won't be on the plot.  Add it here:
		if(cursors_.at(c)->plot() != plot())
			plot()->addItem(cursors_.at(c));

		cursors_.at(c)->setValue(newPos);
		emit valueChanged(c, newPos);

		activeCursor++;
	}

	// ignore the mouse press event, so that it will be propagated to other tools below us:
	event->ignore();

}

void MPlotCursorTool::mouseMoveEvent ( QGraphicsSceneMouseEvent * event ) {
	QGraphicsObject::mouseMoveEvent(event);
}

void MPlotCursorTool::mouseReleaseEvent ( QGraphicsSceneMouseEvent * event ) {
	QGraphicsObject::mouseReleaseEvent(event);
}

void MPlotCursorTool::wheelEvent ( QGraphicsSceneWheelEvent * event ) {
	QGraphicsObject::wheelEvent(event);
}

void MPlotCursorTool::mouseDoubleClickEvent ( QGraphicsSceneMouseEvent * event ) {
	QGraphicsObject::mouseDoubleClickEvent(event);
}


#endif // MPLOTTOOLS_H

