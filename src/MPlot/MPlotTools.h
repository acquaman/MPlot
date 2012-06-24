#ifndef MPLOTTOOLS_H
#define MPLOTTOOLS_H

#include "MPlot/MPlot_global.h"

#include "MPlot/MPlotAbstractTool.h"
#include <QGraphicsSceneMouseEvent>

class MPlotItem;
class MPlotRectangle;

/// When selecting lines on plots with the mouse, this is how wide the selection ballpark is, in pixels. (Actually, in sceneCoordinates, but we prefer that you don't transform the view, so viewCoordinates = sceneCoordinates)
#define MPLOT_SELECTION_BALLPARK 10

/// This is the width of the rubber-band cursor when selecting plot ranges:
#define MPLOT_RUBBERBAND_WIDTH 2

/// This is the distance (in pixels/scene coordinates) that a user must drag with the MPlotDragZoomerTool before it becomes active. This is to avoid a "click" or a "click and very small drag" causing an inadvertent zoom into a very small rectangle.
#define MPLOT_RUBBERBAND_DEADZONE 6



/// This class provides a plot tool that can be used to select a single data-item in a plot.
class MPLOTSHARED_EXPORT MPlotPlotSelectorTool : public MPlotAbstractTool {
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


class MPLOTSHARED_EXPORT MPlotWheelZoomerTool : public MPlotAbstractTool {
	Q_OBJECT
public:
	/// Constructor. By default, this tool operates on all axes (Left, Right, and Bottom), and adds/subtracts 25% to the axis range on each mousewheel click.  Use setZoomIncrement() and setAxisTargets() to change these later.
	MPlotWheelZoomerTool(qreal zoomIncrement = 0.25);

	/// returns the fraction of the axis scale that will be added/subtracted on each mouse wheel click. (0.25 = 25% by default)
	qreal zoomIncrement() const;

	/// set the zoom increment. On every mousewheel click, the range of the axis will be increased or decreased by this fraction.
	void setZoomIncrement(qreal zi);

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
	qreal zf_;

};





#include <QGraphicsRectItem>
#include <QStack>

/// This class provides a plot tool that can be used to choose a selection rectangle with a "rubber-band", and zoom into that region. Right-clicking will restore the zoom to the previous state. (Infinite zoom/restore levels are available.)
/*! Limitation: when adding multiple MPlotTools to a plot, this one must be added first (ie: "on the bottom").  To track the mouse drag, it needs to become the QGraphicsScene::mouseGrabberItem(), which will prevent other tools from receiving mouse clicks if it is "on top" of them.
  */
class MPLOTSHARED_EXPORT MPlotDragZoomerTool : public MPlotAbstractTool {
	Q_OBJECT
public:
	/// Constructor. By default, after adding to a plot, this tool will be active on all axes.  If you want to restrict which axes it zooms, call setAxisTargets() after it is added to the plot.
	MPlotDragZoomerTool();


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
	/// for each new level of zoom, we push an item onto this stack. These items allow returning to previous zoom levels by right-clicking.  Each item is a list of the MPlotAxisScale's and their corresponding MPlotAxisRanges, which were active for this tool at the time the zoom happened (ie: included in targetAxes()).
	QStack<QList<QPair<MPlotAxisScale*, MPlotAxisRange> > > oldZooms_;

	/// Means that a click has happened, but we might not yet have exceeded the drag deadzone to count as a zoom drag event.
	bool dragStarted_;
	/// Means that a zoom drag event is currently happening. We're in between exceeding the drag deadzone and finishing the drag.
	bool dragInProgress_;

};




#include "MPlot/MPlotPoint.h"

/// this is a hack: plot markers for the cursor tool are created with this size (in pixels).  Make it as big as you expect screen resolution to be.  (NO! It's worse... because cursor may not be centered at center of data coordinates.  Needs to be REALLY BIG...) \todo solve this intelligently.
#define MPLOT_CURSOR_BIG_HACK 4000

/// This class provides a plot tool that can be used to place one or more cursors on a plot and read the data value there.
/*! The tool supports multiple cursors.  Cursors can be added with addCursor(), or removed with removeCursor().  Each cursor can be targetted to a different set of axis scales on the plot.  For now, we alternate between cursors with every click.

  \note It is not supported to move this tool from one plot to another.  (The cursors within the tool become and remain targetted to specific axes of the plot.)

 \todo set active cursor (how? by selection? click and drag? programmatically?)

  \todo multiple cursor modes: click, hover, dataSnap
  */
class MPLOTSHARED_EXPORT MPlotCursorTool : public MPlotAbstractTool {
	Q_OBJECT
public:
	MPlotCursorTool();

	virtual ~MPlotCursorTool();

	unsigned numCursors() const;

	/// Returns the currently-selected item in the plot (0 if none).
	QPointF value(unsigned cursorIndex = 0) const;

	/// Returns the MPlotPoint used to represent a specific cursor, so you can adjust it's color, marker, etc, or place it manually using MPlotPoint::setValue().
	MPlotPoint* cursor(unsigned cursorIndex = 0) const;

	/// remove the last cursor
	void removeCursor();

	/// add a cursor.  By default, cursors are added to the center of the existing plot.  You must specify the axis scales to attach this cursor to (and the axis scales must be valid for the current plot.)   (Use 0 for the y-axis scale if you want a vertical bar cursor, or 0 for the x-axis scale if you want a horizontal bar cursor.  If you don't provide any axis scales, the cursor won't be visible.)
	void addCursor(MPlotAxisScale* yAxisScale, MPlotAxisScale* xAxisScale, const QPointF& initialPos = QPointF(0,0));

signals:
	/// emitted when a new point is selected.  \c position is in coordinates based on the xAxisScale and yAxisScale set for that cursor
	void valueChanged(unsigned cursorIndex, const QPointF& position);

protected:

	/// list of plot point markers used as cursors
	QList<MPlotPoint*> cursors_;


	virtual void	mousePressEvent ( QGraphicsSceneMouseEvent * event );

	virtual void	mouseMoveEvent ( QGraphicsSceneMouseEvent * event );
	virtual void	mouseReleaseEvent ( QGraphicsSceneMouseEvent * event );
	virtual void	wheelEvent ( QGraphicsSceneWheelEvent * event );
	virtual void	mouseDoubleClickEvent ( QGraphicsSceneMouseEvent * event );

};

/// This class provides a tool that enables feedback on the x and y position inside a plot.
class MPLOTSHARED_EXPORT MPlotDataPositionTool : public MPlotAbstractTool
{
	Q_OBJECT

public:
	/// Constructor.
	MPlotDataPositionTool();
	/// Destructor.
	virtual ~MPlotDataPositionTool();

	/// Returns the number of data position indicators currently in the plot.
	unsigned count() const;

	/// Returns the current position (in data coordinates) for a given \param index.  Returns a null point if an invalid index is passed.
	QPointF currentPosition(unsigned index) const;
	/// Returns the current selection rect (in data coordinates) for a given \param index.  Returns a null point if an invalid index is passed.
	QRectF currentRect(unsigned index) const;

	/// Adds a data position indicator.  The axis scales for both x and y must be provided and must both be different.  If an indicator already exists with both axis scales being the same, this function returns false.
	bool addDataPositionIndicator(MPlotAxisScale *xAxisScale, MPlotAxisScale *yAxisScale);
	/// Removes a data position indicator from \param index.
	bool removeDataPositionIndicator(unsigned index);

signals:
	/// Notifier that the position inside the plot has changed.  Passes the new position.
	void positionChanged(unsigned indicatorIndex, const QPointF &position);
	/// Notifier of the size of the data rectangle that has been drawn once it is finished.
	void selectedDataRectChanged(unsigned indicatorIndex, const QRectF &rect);

protected:
	/// Re-implemented for the mouse press event.  Moves the indicators to the position of the mouse click.
	virtual void mousePressEvent ( QGraphicsSceneMouseEvent * event );
	/// No added functionality.
	virtual void mouseMoveEvent ( QGraphicsSceneMouseEvent * event );
	/// No added functionality.
	virtual void mouseReleaseEvent ( QGraphicsSceneMouseEvent * event );
	/// No added functionality.
	virtual void wheelEvent ( QGraphicsSceneWheelEvent * event );
	/// No added functionality.
	virtual void mouseDoubleClickEvent ( QGraphicsSceneMouseEvent * event );

	/// List of points.
	QList<MPlotPoint *> indicators_;
	/// List of selection rects.
	QList<MPlotRectangle *> selectedRects_;

	/// The selection rectangle that shows the region selected in the plot.
	QGraphicsRectItem* selectionRect_;
	/// Means that a click has happened, but we might not yet have exceeded the drag deadzone to count as a drag event.
	bool dragStarted_;
	/// Means that a drag event is currently happening. We're in between exceeding the drag deadzone and finishing the drag.
	bool dragInProgress_;
};

#endif // MPLOTTOOLS_H
