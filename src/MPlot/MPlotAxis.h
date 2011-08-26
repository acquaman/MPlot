#ifndef __MPlotAxis_H__
#define __MPlotAxis_H__

#include <QGraphicsObject>
class QPainter;
#include <QFont>
#include <QPen>

class MPlotAxisScale;

/// Graphics item which draws a coordinate axis.  In most cases this class does not need to be used directly; it's used by MPlot to draw the plot axes.
class MPlotAxis : public QGraphicsObject {

	Q_OBJECT

public:

	/// Where to place this axis.  Has nothing to do with which axis scale is shown, except that if the axis scale has an orientation() of Qt::Vertical, you must use either OnLeft or OnRight.  (Same for Qt::Horizontal and using OnBottom or OnTop).
	enum Placement { OnLeft, OnBottom, OnRight, OnTop };
	/// Where to place the ticks: inside the plot area border, outside the plot area border, or straddling it.
	enum TickStyle { Outside, Inside, Middle };

	/// Constructor.  Every MPlotAxis needs to be associated with an MPlotAxisScale to tell it how to map data coordinates to drawing coordinates.
	MPlotAxis(MPlotAxisScale* scale, Placement position, const QString& name = QString(""), QGraphicsItem* parent = 0);
	virtual ~MPlotAxis();

	/// Set the MPlotAxisScale object used for this axis (to map data values to drawing coordinates, and setting the range)
	void setAxisScale(MPlotAxisScale* newScale);
	/// Returns the MPlotAxisScale object which is currently set on this axis (ie: mapping data values to drawing coordinates, and setting the range)
	const MPlotAxisScale* axisScale() const { return axisScale_; }

	/// Set the number and style of ticks:
	/*! \c tStyle can be Inside, Outside, or Middle.  \c tickLength is in logical coordinates ("percent of plot") coordinates.
	\c num is really just suggestion... there might be one less or up to three(?) more, depending on what we think would make nice label values.*/
	void setTicks(int num, TickStyle tstyle = Outside, qreal tickLength = 2);

	// Access properties:
	/// Returns the number of ticks that are drawn on this axis.
	int numTicks() const { return numTicks_; }
	/// The name of the axis that is painted.
	const QString& name() const { return name_; }
	// TODO: access font, labelOffset, etc.

	/// show or hide the value labels along the axis
	void showTickLabels(bool tickLabelsOn = true);
	/// show or hide the grid lines
	void showGrid(bool gridOn = true);
	/// show or hide the axis name
	void showAxisName(bool axisNameOn = true);

	/// Set the pen for the axis line and axis name text:
	void setAxisPen(const QPen& pen);
	/// set the pen for the ticks along the axis:
	void setTickPen(const QPen& pen);
	/// set the pen for the grid lines
	void setGridPen(const QPen& pen);
	/// set the font used for the values along the axis
	void setTickLabelFont(const QFont& font);
	/// set the font used for the axis name
	void setAxisNameFont(const QFont& font);

	/// Set the axis name:
	void setAxisName(const QString& name);
	// TODO: minor ticks

	/// Indicates whether the text size is scaled with the size of the plot.
	bool fontsScaleWithDrawingSize() const { return fontsShouldScale_; }
	/// Set whether the text size is scaled with the size of the plot.  If \c fontsShouldScale is true, the font point size used for the axisNameFont() and tickLabelFont() will be scaled with the drawing size, within a reasonable range.
	void setFontsScaleWithDrawingSize(bool fontsShouldScale);

	// Required functions:
	//////////////////////////

	/// Bounding rect, in parent item (drawing) coordinates
	virtual QRectF boundingRect() const;
	/// Paint function: draws the axis
	virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

	// TODO: finer shape?
	/*
	QPainterPath shape() {
		return
	}*/

public slots:
	/// Call to update the axis when the drawing size changes.
	void onAxisDrawingSizeAboutToChange() { prepareGeometryChange(); }
	/// Method that actually tells the axis to be repainted because the size has been changed.
	void onAxisDrawingSizeChanged() { scaleFontsRequired_ = true; update(); }
	/// Call to update the axis when the data range changes.
	void onAxisDataRangeAboutToChange() { prepareGeometryChange(); }
	/// Method that actually tells the axis to be repainted because the data has been updated.
	void onAxisDataRangeChanged() { scaleFontsRequired_ = true; update(); }

protected:

	/// Sets up the axis to be painted with a standard look.  Gives different aspects of the axis a standard look and feel.
	void setDefaults();

	/// Scales the drawing size to fit within some constraints.  The font must be between 8.5 and 18.
	QFont scaleFontToDrawingSize(const QFont& sourceFont) const;
	/// Scales the fonts based on scaleFontToDrawingSize if a rescaling is required.  Otherwise, leaves values as they were.
	void scaleFonts() const;

	/// Helper function to format, round as appropriate, and convert a double \c tickValue to a string for printing as an axis label.  If the tickValue should be interpreted as 0 within the context of the axis range (ex: -0.2, -0.1, 1.2343e-17, 0.1, 0.2...), this will take care of it.
	QString formatTickLabel(double tickValue);

	/// The axis scaling object which maps data values to plot values
	MPlotAxisScale* axisScale_;

	/// Style of the ticks: inside, outside, or middle
	TickStyle tickStyle_;
	/// Approximate number of ticks to draw.  We may draw more ticks than this, in order to land the ticks on "nice" numbers.
	unsigned numTicks_;

	/// tick lengths, in fraction of plot width
	qreal tickLength_;
	/// The offset between the tics and the label that is associated with.
	qreal tickLabelOffset_;

	/// controls visibility of axis elements
	bool tickLabelsVisible_, gridVisible_, axisNameVisible_;

	/// name displayed along the axis
	QString name_;

	/// Controls appearance of axis elements
	QPen axisPen_, tickPen_, gridPen_;
	QFont tickLabelFontU_, axisNameFontU_;
	mutable QFont tickLabelFont_, axisNameFont_;

	/// Cache the font size information for the axisNameFont_ and tickLabelFont_
	mutable qreal tickLabelCharWidth_, tickLabelHeight_, axisNameHeight_;

	/// Where to draw this axis: OnLeft, OnBottom, OnRight, OnTop. This must be compatible with the orientation() of the MPlotAxisScale it is representing. (ex: vertical axis scales can be shown on the right or on the left, but not on the bottom or top.)
	Placement placement_;

	/// Controls whether the text size scales with the size of hte plot (within a reasonable range)
	bool fontsShouldScale_;

	/// Flags that a re-computation of the font sizes needs to happen (due to changing a font, or changing the drawing size)
	mutable bool scaleFontsRequired_;
};

#endif
