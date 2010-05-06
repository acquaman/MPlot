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

	virtual void	mouseMoveEvent ( QGraphicsSceneMouseEvent * event ) { QGraphicsObject::mouseMoveEvent(event); }
	virtual void	mouseReleaseEvent ( QGraphicsSceneMouseEvent * event ) { QGraphicsObject::mouseReleaseEvent(event); }
	virtual void	wheelEvent ( QGraphicsSceneWheelEvent * event ) { QGraphicsObject::wheelEvent(event); }
	virtual void	mouseDoubleClickEvent ( QGraphicsSceneMouseEvent * event ) { QGraphicsObject::mouseDoubleClickEvent(event); }

	MPlotItem* selectedItem_;

};


class MPlotWheelZoomerTool : public MPlotAbstractTool {
	Q_OBJECT
public:
	/// Constructor. By default, this tool operates on all axes (Left, Right, and Bottom), and changes the axis scale by a factor of 0.75 on each mousewheel click.  Use setZoomFactor() and setYAxisTargets() to change these later.
	MPlotWheelZoomerTool(double zoomFactor = 0.75, int axisTargets = (MPlotAxis::Left | MPlotAxis::Right | MPlotAxis::Bottom)) : MPlotAbstractTool() {

		setZoomFactor(zoomFactor);

		setAxisTargets(axisTargets);
	}

	/// returns the factor that will multiply the range of an axis on each mouse-wheel "click"
	double zoomFactor() const {
		return zf_ / 120;
	}

	/// set the zoom factor. On every mousewheel click, the range of the axis will be multiplied by this amount.
	void setZoomFactor(double factor) {
		zf_ = factor * 120;
	}

signals:


protected:

	virtual void	mousePressEvent ( QGraphicsSceneMouseEvent * event ) {
		event->ignore();
		QGraphicsObject::mousePressEvent(event);
	}

	/// Handles drag events, redraws the selection retangle to follow the mouse, and handles state transitions between dragStarted_ and dragInProgress_
	virtual void	mouseMoveEvent ( QGraphicsSceneMouseEvent * event ) {
		QGraphicsObject::mouseMoveEvent(event);
	}

	virtual void	mouseReleaseEvent ( QGraphicsSceneMouseEvent * event ) {
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
	virtual void	wheelEvent ( QGraphicsSceneWheelEvent * event ) { 
	

		// delta: mouse wheel rotation amount. 120 corresponds to 1 "click", or 15 degrees rotation on most mice.  Units are 1/8th of a degree.

		double F = zf_ / event->delta();

		// If the rotation is backwards, we don't want negative zoom factors, we want reciprocal zoom factors.
		if(event->delta() < 0)
			F = -1/F;

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
	
	virtual void	mouseDoubleClickEvent ( QGraphicsSceneMouseEvent * event ) { QGraphicsObject::mouseDoubleClickEvent(event); }


	/// scale factor, corrected for (1 click == 120)
	double zf_;

};





#include <QGraphicsRectItem>
#include <QStack>

/// This class provides a plot tool that can be used to choose a selection rectangle with a "rubber-band", and zoom into that region. Right-clicking will restore the zoom to the previous state. (Infinite zoom/restore levels are available.)
/*! Limitation: when adding multiple MPlotTools to a plot, this one must be added first (ie: "on the bottom").  To track the mouse drag, it needs to become the QGraphicsScene::mouseGrabberItem(), which will prevent other tools from receiving mouse clicks if it is "on top" of them.
  */
class MPlotDragZoomerTool : public MPlotAbstractTool {
	Q_OBJECT
public:
	/// Constructor.  \c axisTargets specifies an OR combination of MPlotAxis::AxisID flags that set which axes this tool has zoom control over.
	MPlotDragZoomerTool(int axisTargets = (MPlotAxis::Left | MPlotAxis::Right | MPlotAxis::Bottom)) : MPlotAbstractTool() {

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
	virtual void	mouseReleaseEvent ( QGraphicsSceneMouseEvent * event ) {

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





	virtual void	wheelEvent ( QGraphicsSceneWheelEvent * event ) { QGraphicsObject::wheelEvent(event); }
	virtual void	mouseDoubleClickEvent ( QGraphicsSceneMouseEvent * event ) { QGraphicsObject::mouseDoubleClickEvent(event); }

	QGraphicsRectItem* selectionRect_;
	QStack<QRectF> leftZoomStack_, rightZoomStack_, bottomZoomStack_;

	/// Means that a click has happened, but we might not yet have exceeded the drag deadzone to count as a zoom drag event.
	bool dragStarted_;
	/// Means that a zoom drag event is currently happening. We're in between exceeding the drag deadzone and finishing the drag.
	bool dragInProgress_;

};




#include "MPlotPoint.h"

/// this is a hack: plot markers for the cursor tool are created with this size (in pixels).  Make it as big as you expect screen resolution to be.
#define MPLOT_CURSOR_SCREEN_SIZE 3000

/// This class provides a plot tool that can be used to place one or more cursors on a plot and read the data value there.
/*! The visibility and scale is controlled by axisTargets().  If axisTargets() includes MPlotAxis::Bottom, a vertical bar cursor is displayed.  If axisTargets() includes MPlotAxis::Left or MPlotAxis::Right, the horizontal line cursor is displayed.  The y-value read from position() will correspond to the right-axis value only if axisTargets() includes MPlotAxis::Right.

 The tool supports multiple cursors.  One cursor is created by default and added to the plot.  More can be added with addCursor(), or removed with removeCursor().  For now, we alternate between cursors with every click.

 \todo set active cursor (how? by selection? click and drag? programmatically?)

  \todo multiple cursor modes: click, hover, dataSnap
  */
class MPlotCursorTool : public MPlotAbstractTool {
	Q_OBJECT
public:
	MPlotCursorTool(int axisTargets = (MPlotAxis::Left | MPlotAxis::Right | MPlotAxis::Bottom))
		: MPlotAbstractTool() {

		setAxisTargets(axisTargets);
		addCursor();
	}

	virtual ~MPlotCursorTool() {
		foreach(MPlotPoint* c, cursors_) {
			if(plot())
				plot()->removeItem(c);
			delete c;
		}
		cursors_.clear();
	}

	unsigned numCursors() const { return cursors_.count(); }

	/// Returns the currently-selected item in the plot (0 if none).
	QPointF value(unsigned cursorIndex = 0) const {
		if(cursorIndex < numCursors())
			return cursors_.at(cursorIndex)->value();
		else
			return QPointF(0,0);
	}

	/// Returns the MPlotPoint used to represent a specific cursor, so you can adjust it's color, marker, etc, or place it manually using MPlotPoint::setValue().
	MPlotPoint* cursor(unsigned cursorIndex = 0) const {
		if(cursorIndex < numCursors())
			return cursors_.at(cursorIndex);
		else
			return 0;
	}

	/// remove a cursor. Note: you cannot remove the final cursor... there must always be 1.
	void removeCursor() {
		if(numCursors() > 1) {
			MPlotPoint* removeMe = cursors_.takeLast();
			if(plot())
				plot()->removeItem(removeMe);
			delete removeMe;
		}
	}

	/// add a cursor.  Cursors are added to the center of the existing plot.
	void addCursor(const QPointF& initialPos = QPointF(0,0)) {
		MPlotPoint* newCursor = new MPlotPoint();
		newCursor->setSelectable(false);
		newCursor->setMarker(MPlotMarkerShape::Cross, MPLOT_CURSOR_SCREEN_SIZE);
		/// \todo automatically vary the colors.
		newCursor->setValue(initialPos);

		if(plot()) {
			plot()->addItem(newCursor);
		}
		cursors_ << newCursor;
	}

signals:
	// emitted when a new point is selected
	void valueChanged(unsigned cursorIndex, const QPointF& position);

protected:

	/// list of plot point markers used as cursors
	QList<MPlotPoint*> cursors_;

	/// a helper function to return the y-Axis that we belong to, based on axisTargets().
	/*! Since axisTargets() could contain both MPlotAxis::Left and MPlotAxis::Right, we resolve ambiguity like this:
	- If axisTargets() includes the left axis (and anything else), we belong to the left axis.
	- If axisTargets() doesn't include the left axis, but includes the right axis, we belong to the right axis.
	- If axisTargets() doesn't include any y-axis, we belong to the left axis.
	*/

	MPlotAxis::AxisID yAxis() const {
		if(axisTargets() & MPlotAxis::Left)
			return MPlotAxis::Left;
		if(axisTargets() & MPlotAxis::Right)
			return MPlotAxis::Right;
		return MPlotAxis::Left;
	}

	virtual void	mousePressEvent ( QGraphicsSceneMouseEvent * event ) {

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

	virtual void	mouseMoveEvent ( QGraphicsSceneMouseEvent * event ) { QGraphicsObject::mouseMoveEvent(event); }
	virtual void	mouseReleaseEvent ( QGraphicsSceneMouseEvent * event ) { QGraphicsObject::mouseReleaseEvent(event); }
	virtual void	wheelEvent ( QGraphicsSceneWheelEvent * event ) { QGraphicsObject::wheelEvent(event); }
	virtual void	mouseDoubleClickEvent ( QGraphicsSceneMouseEvent * event ) { QGraphicsObject::mouseDoubleClickEvent(event); }

};

#endif // MPLOTTOOLS_H
