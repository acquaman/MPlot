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
	MPlotPlotSelectorTool();

	/// Returns the currently-selected item in the plot (0 if none).
	MPlotItem* selectedItem();

signals:
	// Emitted by the view when an item in the plot is selected
		// Note that selection happens on a per-scene basis.  The same item can be in multiple views, and will be selected in multiple views.
	void itemSelected(MPlotItem*);
	// Emitted when nothing is selected:
	void deselected();

protected:
	// This is used to detect PlotItem selection.
	// If multiple items are on top of each other (or are within the selection range), this will alternate between them on successive clicks.

	virtual void	mousePressEvent ( QGraphicsSceneMouseEvent * event );

	virtual void	mouseMoveEvent ( QGraphicsSceneMouseEvent * event );
	virtual void	mouseReleaseEvent ( QGraphicsSceneMouseEvent * event );
	virtual void	wheelEvent ( QGraphicsSceneWheelEvent * event );
	virtual void	mouseDoubleClickEvent ( QGraphicsSceneMouseEvent * event );

	MPlotItem* selectedItem_;

};


class MPlotWheelZoomerTool : public MPlotAbstractTool {
	Q_OBJECT
public:
	/// Constructor. By default, this tool operates on all axes (Left, Right, and Bottom), and adds/subtracts 25% to the axis range on each mousewheel click.  Use setZoomIncrement() and setYAxisTargets() to change these later.
	MPlotWheelZoomerTool(double zoomIncrement = 0.25, int axisTargets = (MPlotAxis::Left | MPlotAxis::Right | MPlotAxis::Bottom));

	/// returns the fraction of the axis scale that will be added/subtracted on each mouse wheel click. (0.25 = 25% by default)
	double zoomIncrement() const;

	/// set the zoom increment. On every mousewheel click, the range of the axis will be increased or decreased by this fraction.
	void setZoomIncrement(double zi);

signals:


protected:

	virtual void	mousePressEvent ( QGraphicsSceneMouseEvent * event );

	/// Handles drag events, redraws the selection retangle to follow the mouse, and handles state transitions between dragStarted_ and dragInProgress_
	virtual void	mouseMoveEvent ( QGraphicsSceneMouseEvent * event );

	virtual void	mouseReleaseEvent ( QGraphicsSceneMouseEvent * event );

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
	virtual void	wheelEvent ( QGraphicsSceneWheelEvent * event );

	virtual void	mouseDoubleClickEvent ( QGraphicsSceneMouseEvent * event );


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
	MPlotDragZoomerTool(int axisTargets = (MPlotAxis::Left | MPlotAxis::Right | MPlotAxis::Bottom));


signals:


protected:

	virtual void	mousePressEvent ( QGraphicsSceneMouseEvent * event );

	/// Handles drag events, redraws the selection retangle to follow the mouse, and handles state transitions between dragStarted_ and dragInProgress_
	virtual void	mouseMoveEvent ( QGraphicsSceneMouseEvent * event );

	/// Handles release events. If a drag was in progress and the user lets go of the left button, zoom to the new rectangle and save the old one on the recall stack.  If the user lets go of the right button, this is a restore to a zoom position on the stack.
	virtual void	mouseReleaseEvent ( QGraphicsSceneMouseEvent * event );

	virtual void	wheelEvent ( QGraphicsSceneWheelEvent * event );
	virtual void	mouseDoubleClickEvent ( QGraphicsSceneMouseEvent * event );

	QGraphicsRectItem* selectionRect_;
	QStack<QRectF> leftZoomStack_, rightZoomStack_, bottomZoomStack_;

	/// Means that a click has happened, but we might not yet have exceeded the drag deadzone to count as a zoom drag event.
	bool dragStarted_;
	/// Means that a zoom drag event is currently happening. We're in between exceeding the drag deadzone and finishing the drag.
	bool dragInProgress_;

};




#include "MPlotPoint.h"

/// this is a hack: plot markers for the cursor tool are created with this size (in pixels).  Make it as big as you expect screen resolution to be.  (NO! It's worse... because cursor may not be centered at center of data coordinates.  Needs to be REALLY BIG...) \todo solve this intelligently.
#define MPLOT_CURSOR_BIG_HACK 1.0e9

/// This class provides a plot tool that can be used to place one or more cursors on a plot and read the data value there.
/*! The visibility and scale is controlled by axisTargets().  If axisTargets() includes MPlotAxis::Bottom, a vertical bar cursor is displayed.  If axisTargets() includes MPlotAxis::Left or MPlotAxis::Right, the horizontal line cursor is displayed.  The y-value read from position() will correspond to the right-axis value only if axisTargets() includes MPlotAxis::Right.

 The tool supports multiple cursors.  One cursor is created by default and added to the plot.  More can be added with addCursor(), or removed with removeCursor().  For now, we alternate between cursors with every click.

 \todo set active cursor (how? by selection? click and drag? programmatically?)

  \todo multiple cursor modes: click, hover, dataSnap
  */
class MPlotCursorTool : public MPlotAbstractTool {
	Q_OBJECT
public:
	MPlotCursorTool(int axisTargets = (MPlotAxis::Left | MPlotAxis::Right | MPlotAxis::Bottom));

	virtual ~MPlotCursorTool();

	unsigned numCursors() const;

	/// Returns the currently-selected item in the plot (0 if none).
	QPointF value(unsigned cursorIndex = 0) const;

	/// Returns the MPlotPoint used to represent a specific cursor, so you can adjust it's color, marker, etc, or place it manually using MPlotPoint::setValue().
	MPlotPoint* cursor(unsigned cursorIndex = 0) const;

	/// remove a cursor. Note: you cannot remove the final cursor... there must always be 1.
	void removeCursor();

	/// add a cursor.  Cursors are added to the center of the existing plot.
	void addCursor(const QPointF& initialPos = QPointF(0,0));

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

	MPlotAxis::AxisID yAxis() const;

	virtual void	mousePressEvent ( QGraphicsSceneMouseEvent * event );

	virtual void	mouseMoveEvent ( QGraphicsSceneMouseEvent * event );
	virtual void	mouseReleaseEvent ( QGraphicsSceneMouseEvent * event );
	virtual void	wheelEvent ( QGraphicsSceneWheelEvent * event );
	virtual void	mouseDoubleClickEvent ( QGraphicsSceneMouseEvent * event );

};

#endif // MPLOTTOOLS_H
