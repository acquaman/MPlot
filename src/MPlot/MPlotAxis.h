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
	int numTicks() const { return numTicks_; }
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
	void onAxisDrawingSizeChanged() { axisPlacementRequired_ = true; update(); }
	/// Calll to update the axis when the data range changes.
	void onAxisDataRangeAboutToChange() { prepareGeometryChange(); }
	void onAxisDataRangeChanged() { axisPlacementRequired_ = true; update(); }

protected:


	/// The axis scaling object which maps data values to plot values
	MPlotAxisScale* axisScale_;

	/// Style of the ticks: inside, outside, or middle
	TickStyle tickStyle_;
	/// Approximate number of ticks to draw.  We may draw more ticks than this, in order to land the ticks on "nice" numbers.
	unsigned numTicks_;

	/// tick lengths, in fraction of plot width
	qreal tickLength_;

	qreal tickLabelOffset_;

	/// controls visibility of axis elements
	bool tickLabelsVisible_, gridVisible_, axisNameVisible_;

	/// name displayed along the axis
	QString name_;

	/// Controls appearance of axis elements
	QPen axisPen_, tickPen_, gridPen_;
	QFont tickLabelFont_, axisNameFont_;

	/// Actual tick values, as chosen by the intelliScale() algorithm
	qreal minTickVal_, tickIncVal_;

	/// Coordinates, computed by placeAxis and used for paint()ing:
	QLineF mainLine_, tickLine_, gridLine_;
	QPointF scStartP_, scIncP_;
	qreal scStart_, scIncrement_;

	// cached bounding rectangle of all the text labels:
	QRectF tickLabelBr_;
	// cached bounding rectangle of the axis name label:
	QRectF nameBr_;
	// textTransform: A transform based on our true device coordinates, used to draw undistorted text at a nice size:
	QTransform tt_;

	/// Where to draw this axis: OnLeft, OnBottom, OnRight, OnTop. This must be compatible with the orientation() of the MPlotAxisScale it is representing. (ex: vertical axis scales can be shown on the right or on the left, but not on the bottom or top.)
	Placement placement_;

	//Helper Functions:

	bool axisPlacementRequired_;
	// Adjust the length of the main line, and determine the locations for the ticks.
	void placeAxis();

	/// Calculates a transform suitable for applying to the painter to draw text.
	/*! The text will shrink and grow with the size of the plot, but only within a reasonable range. (ie: infinitely small text isn't helpful, and super-humongous text isn't helpful).
		Result is saved in tt_ */
	void calcTextTransform(QPainter* painter);

	void drawLabel(QPainter* painter, const QString& text, const QPointF& tickPos);

	/// Used to draw the axis name (ex: "x" or "time (s)") onto the plot.  Draw the axis labels first so we know where to put this.
	void drawAxisName(QPainter* painter);


/// IntelliScale: Calculate "nice" values for starting tick and tick increment.
/*! Sets minTickVal_ and tickIncVal_ for nice values of axis ticks.
- Prior to calling, numTicks() and max_ and min_ must be correct.
- Desired outcome: labels are nice values like "0.2 0.4 0.6..." or "0.024 0.026 0.028" instead of irrational numbers.
- Additionally, if the axis range passes through 0, it would be nice to have a tick at 0.

Implementation: This algorithm is based on one from "C++ Gui Programming with Qt" (below), but we move the starting tick position instead of the max and min values.

<i>
To obtain nice numbers along the axis, we must select the step with care. For example, a step value of 3.8 would lead to an axis with multiples of 3.8, which is difficult for people to relate to. For axes labeled in decimal notation, “nice” step values are numbers of the form 10n, 2·10n, or 5·10n.

We start by computing the “gross step”, a kind of maximum for the step value. Then we find the corresponding number of the form 10n that is smaller than or equal to the gross step. We do this by taking the decimal logarithm of the gross step, rounding that value down to a whole number, then raising 10 to the power of this rounded number. For example, if the gross step is 236, we compute log 236 = 2.37291...; then we round it down to 2 and obtain 102 = 100 as the candidate step value of the form 10n.

Once we have the first candidate step value, we can use it to calculate the other two candidates: 2·10n and 5·10n. For the example above, the two other candidates are 200 and 500. The 500 candidate is larger than the gross step, so we can’t use it. But 200 is smaller than 236, so we use 200 for the step size in this example.
</i>
\code
void PlotSettings::adjustAxis(qreal &min, qreal &max,
int &numTicks)
{
	const int MinTicks = 4;
	qreal grossStep = (max - min) / MinTicks;
	qreal step = pow(10.0, floor(log10(grossStep)));
	if (5 * step < grossStep) {
		step *= 5;
	} else if (2 * step < grossStep) {
		step *= 2;
	}
	numTicks = int(ceil(max / step) - floor(min / step));
	if (numTicks < MinTicks)
		numTicks = MinTicks;
	min = floor(min / step) * step;
	max = ceil(max / step) * step;
}
\endcode

We used to do it this way:
\code
// normalize range so difference between max and min goes to 10(max):
qreal norm = pow(10, trunc(log10(max_ - min_)) - 1);
// Round off to get nice numbers. Note that minTickVal_ must be > min_, otherwise it falls off the bottom of the plot.
minTickVal_ = ceil(min_/norm);
tickIncVal_  = trunc( (max_/norm-minTickVal_)/(numTicks()-1) );

// if the tickIncVal is 0, that'll be trouble. Normalize smaller to avoid this.
while((int)tickIncVal_ == 0) {
	norm /= 2;
	minTickVal_ = ceil(min_/norm);
	tickIncVal_  = trunc( (max_/norm-minTickVal_)/(numTicks()-1) );
}


// Hit Zero if possible: (while passing through origin)
if(min_ < 0 && max_ > 0) {
	qreal potentialminTickVal = minTickVal_ + ( (int)(-minTickVal_) % (int)tickIncVal_ );
	// Disabled: not necessary now that we draw an arbitrary number of ticks:
		// previously: // Just making sure we don't go past the end of the axis with this tweak
		// if( (potentialminTickVal + tickIncVal_*(numTicks()-1))*norm < max_)
		minTickVal_ = potentialminTickVal;
}

//Rescale:
minTickVal_ *= norm;
tickIncVal_ *= norm;
		}
\endcode
*/
	void intelliScale();
	void setDefaults();

};

#endif
