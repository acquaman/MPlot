#ifndef __MPlotAxis_CPP__
#define __MPlotAxis_CPP__

#include "MPlotAxis.h"
#include "MPlotAxisScale.h"

#include <QPainter>
#include <math.h>
#include <float.h>

#include <QDebug>


MPlotAxis::MPlotAxis(MPlotAxisScale* scale, Placement placement, const QString& name, QGraphicsItem* parent) :
	QGraphicsObject(parent)
{
	axisScale_ = scale;
	connect(axisScale_, SIGNAL(drawingSizeAboutToChange()), this, SLOT(onAxisDrawingSizeAboutToChange()));
	connect(axisScale_, SIGNAL(drawingSizeChanged()), this, SLOT(onAxisDrawingSizeChanged()));
	connect(axisScale_, SIGNAL(dataRangeAboutToChange()), this, SLOT(onAxisDataRangeAboutToChange()));
	connect(axisScale_, SIGNAL(dataRangeChanged()), this, SLOT(onAxisDataRangeChanged()));
	name_ = name;

	placement_ = placement;
	if(axisScale_->orientation() == Qt::Vertical && (placement_ == OnBottom || placement_ == OnTop)) {
		qWarning() << "MPlotAxis: It's impossible to place a vertical axis at the bottom or top of a plot. Moving to the left side.";
		placement_ = OnLeft;
	}
	if(axisScale_->orientation() == Qt::Horizontal && (placement_ == OnLeft || placement_ == OnRight)) {
		qWarning() << "MPlotAxis: It's impossible to place a horizontal axis at the left or right of a plot. Moving to the bottom.";
		placement_ = OnBottom;
	}

	numTicks_ = 5;
	setDefaults();

	// Put everything in the right spot:
	axisPlacementRequired_ = true;
}

MPlotAxis::~MPlotAxis() {
}


// Set the number and style of ticks:
/*! \c tStyle can be Inside, Outside, or Middle.  \c tickLength is in logical coordinates ("percent of plot") coordinates.
 \c num is really just suggestion... there might be one less or up to three(?) more, depending on what we think would make nice label values.*/
void MPlotAxis::setTicks(int num, TickStyle tstyle, qreal tickLength) {

	prepareGeometryChange();

	numTicks_ = num;
	tickStyle_ = tstyle;
	tickLength_ = tickLength/100;

	axisPlacementRequired_ = true;
	update();
}




/// show or hide the value labels along the axis
void MPlotAxis::showTickLabels(bool tickLabelsOn) {
	prepareGeometryChange();

	tickLabelsVisible_ = tickLabelsOn;
	axisPlacementRequired_ = true;
	update();
}

/// show or hide the grid lines
void MPlotAxis::showGrid(bool gridOn) {

	prepareGeometryChange();

	gridVisible_ = gridOn;
	update();
}

/// show or hide the axis name
void MPlotAxis::showAxisName(bool axisNameOn) {
	prepareGeometryChange();

	axisNameVisible_ = axisNameOn;
	axisPlacementRequired_ = true;
	update();
}

/// Set the pen for the axis line and axis name text:
void MPlotAxis::setAxisPen(const QPen& pen) {
	axisPen_ = pen;
	update();
}

/// set the pen for the ticks along the axis:
void MPlotAxis::setTickPen(const QPen& pen) {
	tickPen_ = pen;
	update();
}

/// set the pen for the grid lines
void MPlotAxis::setGridPen(const QPen& pen) {
	gridPen_ = pen;
	update();
}

/// set the font used for the values along the axis
void MPlotAxis::setTickLabelFont(const QFont& font) {
	prepareGeometryChange();

	tickLabelFont_ = font;
	axisPlacementRequired_ = true;
	update();
}

/// set the font used for the axis name
void MPlotAxis::setAxisNameFont(const QFont& font) {
	prepareGeometryChange();

	axisNameFont_ = font;
	axisPlacementRequired_ = true;
	update();
}

/// Set the axis name:
void MPlotAxis::setAxisName(const QString& name) {
	prepareGeometryChange();

	name_ = name;
	axisPlacementRequired_ = true;
	update();
}
// TODO: minor ticks

// Required functions:
//////////////////////////
// Bounding rect:
QRectF MPlotAxis::boundingRect() const {

	// For this to be accurate, make sure to keep mainLine_ going from (smallestX, smallestY) to (largestX, largestY)
	// and tickLine_ going from (smallestX, smallestY) in P1 to (largestX, largestY) in P2.
	QPointF bottomLeft(mainLine_.x1()+tickLine_.x1(), mainLine_.y1()+tickLine_.y1());
	QPointF topRight(mainLine_.x2()+tickLine_.x2(), mainLine_.y2()+tickLine_.y2());
	QRectF br(bottomLeft, topRight);

	// If the grids are on, they take up the whole rectangle
	if(gridVisible_)
		br |= QRectF(QPointF(0,0), axisScale_->drawingSize());

	return (br | tickLabelBr_ | nameBr_);
}
// Paint:
void MPlotAxis::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {

	Q_UNUSED(widget)
	Q_UNUSED(option)

	if(axisPlacementRequired_)
		placeAxis();

	// Disable anti-aliasing, because these horizontal and vertical lines look best non-AA'd.
	/// \todo: check for rotation, and leave AA on if rotated.
	/// \todo: figure out why gridlines and ticks don't round to the same pixel when AA is off. Leaving on until then.
	// painter->setRenderHint(QPainter::Antialiasing, false);

	// Draw the main axis line:
	painter->setPen(axisPen_);
	painter->drawLine(mainLine_);

	// Draw the ticks:
	painter->setPen(tickPen_);
	for(unsigned i=0; numTicks_ > 0 && scStart_ + i*scIncrement_ < 1; i++) {
		painter->drawLine(tickLine_.translated(scStartP_ + i*scIncP_));
	}

	QSizeF drawingSize = axisScale_->drawingSize();

	// clear/initialize the bounding box containing the tick labels. (It determines where to place the axis name. Needed if we're not drawing the axis names).
	tickLabelBr_ = QRectF();
	switch(placement_) {
	case OnBottom:
		tickLabelBr_ = QRectF(0,drawingSize.height(),drawingSize.width(),0);
		break;
	case OnTop:
		tickLabelBr_ = QRectF(0,0,drawingSize.width(),0);
		break;
	case OnLeft:
		tickLabelBr_ = QRectF(0,0,0,drawingSize.height());
		break;
	case OnRight:
		tickLabelBr_ = QRectF(drawingSize.width(),0,0,drawingSize.height());
		break;
	}

	// calculate a nice size/scaling for text:
	calcTextTransform(painter);

	// Draw Tick Labels:
	if(tickLabelsVisible_) {
		painter->setPen(axisPen_);
		painter->setFont(tickLabelFont_);
		if(placement_ == OnBottom || placement_ == OnTop)
			for(unsigned i=0; numTicks_ > 0 && scStart_ + i*scIncrement_ < drawingSize.width(); i++)
				drawLabel(painter, QString::number(minTickVal_+i*tickIncVal_), scStartP_ + i*scIncP_);
		else
			for(unsigned i=0; numTicks_ > 0 && scStart_ + i*scIncrement_ > 0; i++)
				drawLabel(painter, QString::number(minTickVal_+i*tickIncVal_), scStartP_ + i*scIncP_);


	}

	// Draw grids:
	if(gridVisible_) {
		painter->setPen(gridPen_);
		if(placement_ == OnBottom || placement_ == OnTop)
			for(unsigned i=0; numTicks_ > 0 && scStart_ + i*scIncrement_ < drawingSize.width(); i++)
				painter->drawLine(gridLine_.translated(scStartP_ + i*scIncP_));
		else
			for(unsigned i=0; numTicks_ > 0 && scStart_ + i*scIncrement_ > 0; i++)
				painter->drawLine(gridLine_.translated(scStartP_ + i*scIncP_));
	}


	// Draw axis name?
	if(axisNameVisible_) {
		painter->setFont(axisNameFont_);
		painter->setPen(axisPen_);
		drawAxisName(painter);
	}
}



// TODO: finer shape?
/*
 QPainterPath shape() {
  return
 }*/





//Helper Functions:

// Adjust the length of the main line, and determine the locations for the ticks.
void MPlotAxis::placeAxis() {

	QSizeF drawingSize = axisScale_->drawingSize();

	qreal min = axisScale_->min();
	qreal max = axisScale_->max();

	// Determine "nice" values for the axis labels. (Sets minTickVal_ and tickIncVal_)
	intelliScale();

	// Tick increment and initial value, in relative coordinates for now (0,1)
	scIncrement_ = tickIncVal_ / (max - min);
	scStart_ = (minTickVal_ - min) / (max - min);

	// Handle differences between vertical and horizontal axes.
	// Set QLineFs mainLine_, gridLine_, and tickLine_
	// and QPointFs scIncP_, scStartP_ for use by the paint() function
	if(placement_ == OnLeft || placement_ == OnRight) {

		scStart_ = drawingSize.height()*(1 - scStart_);	// flip and scale
		scIncrement_ = -drawingSize.height()*scIncrement_;	// flip and scale. Now scStart_ and scIncrement_ are in screen coordinates. scIncrement is negative because increasing ticks along the axis will have decreasing y values.  (Silly graphics coordinate system...)

		scIncP_ = QPointF(0, scIncrement_);
		qreal tickLength = tickLength_*drawingSize.width();

		// Left axis:
		if(placement_ == OnLeft) {
			mainLine_.setLine(0, 0, 0, drawingSize.height());
			scStartP_ = QPointF(0, scStart_);
			gridLine_.setLine(0, 0, drawingSize.width(), 0);

			switch(tickStyle_) {
			case Outside: tickLine_.setLine(-tickLength, 0, 0, 0); break;
			case Inside: tickLine_.setLine(0, 0, tickLength, 0); break;
			case Middle: tickLine_.setLine(-tickLength/2, 0, tickLength/2, 0); break;
			}
		}
		// Right axis:
		else {
			mainLine_.setLine(drawingSize.width(), 0, drawingSize.width(), drawingSize.height());
			scStartP_ = QPointF(drawingSize.width(), scStart_);
			gridLine_.setLine(0, 0, -drawingSize.width(), 0);
			switch(tickStyle_) {
			case Outside: tickLine_.setLine(0, 0, tickLength, 0); break;
			case Inside: tickLine_.setLine(-tickLength, 0, 0, 0); break;
			case Middle: tickLine_.setLine(-tickLength/2, 0, tickLength/2, 0); break;
			}
		}
	}

	// Same for horizontal axes (increment is horizontal, tickLine_ is vertical)
	else{

		scStart_ *= drawingSize.width();
		scIncrement_ *= drawingSize.width();

		scIncP_ = QPointF(scIncrement_, 0);
		qreal tickLength = tickLength_*drawingSize.height();

		if(placement_ == OnBottom) {
			mainLine_.setLine(0, drawingSize.height(), drawingSize.width(), drawingSize.height());
			scStartP_ = QPointF(scStart_, drawingSize.height());
			gridLine_.setLine(0, 0, 0, -drawingSize.height());
			switch(tickStyle_) {
			case Inside: default: tickLine_.setLine(0, 0, 0, -tickLength); break;
			case Outside: tickLine_.setLine(0, tickLength, 0, 0); break;
			case Middle: tickLine_.setLine(0, tickLength/2, 0, -tickLength/2); break;
			}
		}
		else {
			mainLine_.setLine(0, 0, drawingSize.width(), 0);
			scStartP_ = QPointF(scStart_, 0);
			gridLine_.setLine(0, 0, 0, drawingSize.height());
			switch(tickStyle_) {
			case Inside: default: tickLine_.setLine(0, tickLength, 0, 0); break;
			case Outside: tickLine_.setLine(0, 0, 0, -tickLength); break;
			case Middle: tickLine_.setLine(0, tickLength/2, 0, -tickLength/2); break;
			}
		}
	}

	axisPlacementRequired_ = false;
}

/// Calculates a transform suitable for applying to the painter to draw undistorted text.
/*! The text will shrink and grow with the size of the plot, but only within a reasonable range. (ie: infinitely small text isn't helpful, and super-humongous text isn't helpful).
  Result is saved in tt_ */
void MPlotAxis::calcTextTransform(QPainter* painter) {

	// debugging only... no transform
	tt_ = QTransform();
	return;


	// old:

	// World transform: tells how we would get back into actual pixel coordinates
	QTransform wt = painter->deviceTransform();	// equivalent to worldTransform and combinedTransform

	// "unscale" to get rid of different scaling in x and y:
	tt_ = QTransform::fromScale(1/wt.m11(), 1/wt.m22());

	// reintroduce a bit of dependence on the plot size, but constrain with a min/max range.
	// use the smaller dimension as the relevant one to base this on.
	qreal scaleFactor;
	if(fabs(wt.m11()) > fabs(wt.m22()))
		scaleFactor = qBound((qreal)0.8, fabs(wt.m22()/250), (qreal)1.2);
	else
		scaleFactor = qBound((qreal)0.8, fabs(wt.m11()/250), (qreal)1.2);
	tt_.scale(scaleFactor, scaleFactor);


}

void MPlotAxis::drawLabel(QPainter* painter, const QString& text, const QPointF& tickPos) {

	// Text Transform: will be applied to place text at correct location, scaled back to reasonable, undistorted size
	QTransform tt = QTransform::fromTranslate(tickPos.x(), tickPos.y());

	// Don't clip if we overflow the container (because we're making a tiny container)
	int flags = Qt::TextDontClip;
	// Create a container rectangle centered on the tick position:
	QRectF container(0,0,0,0);

	// set alignment and offset based on axis orientation:
	switch(placement_) {
	case OnBottom:
		flags |= (Qt::AlignHCenter | Qt::AlignTop);
		tt.translate(0, tickLine_.y1()+tickLabelOffset_);
		break;
	case OnTop:
		flags |= (Qt::AlignHCenter | Qt::AlignBottom);
		tt.translate(0, tickLine_.y2()-tickLabelOffset_);
		break;
	case OnLeft:
		flags |= (Qt::AlignRight | Qt::AlignVCenter);
		tt.translate(tickLine_.x1()-tickLabelOffset_, 0);
		break;
	case OnRight:
		flags |= (Qt::AlignLeft | Qt::AlignVCenter);
		tt.translate(tickLine_.x2()+tickLabelOffset_, 0);
		break;
	}

	// apply scaling to get rid of x-y distortion and reach a nice size for text.
	// (Note that tt_ was calculated once before all calls to drawLabel() )
	tt.scale(tt_.m11(), tt_.m22());
	// save the old painter transform,
	painter->save();
	// translate the painter into position and scale:
	painter->setTransform(tt, true);

	QRectF br;	// used to return the effective bounding rectangle
	// draw the text, and discover the bounding rectangle it filled (note: this will be in tt coordinates... must map back)
	painter->drawText(container, flags, text, &br);
	// remember the bounding box of all tick labels:
	tickLabelBr_ |= tt.mapRect(br);

	// restore the painter transform so we don't mess it up for the next person who draws...
	painter->restore();
}

/// Used to draw the axis name (ex: "x" or "time (s)") onto the plot.  Draw the axis labels first so we know where to put this.
void MPlotAxis::drawAxisName(QPainter* painter) {

	// Text Transform: will be applied to place text at correct location, scaled back to normal size
	QTransform tt;

	// Don't clip if we overflow the container (because we're making a tiny container)
	int flags = Qt::TextDontClip;
	// Create a container rectangle centered on the tick position:
	QRectF container(0,0,0,0);

	QSizeF drawingSize = axisScale_->drawingSize();

	switch(placement_) {
	case OnBottom:
		flags |= (Qt::AlignHCenter | Qt::AlignTop);
		tt.translate(0.5*drawingSize.width(), tickLabelBr_.top() );
		tt.scale(tt_.m11(), tt_.m22());
		break;
	case OnTop:
		flags |= (Qt::AlignHCenter | Qt::AlignBottom);
		tt.translate(0.5*drawingSize.width(), tickLabelBr_.bottom() );
		tt.scale(tt_.m11(), tt_.m22());
		break;
	case OnLeft:
		flags |= (Qt::AlignHCenter | Qt::AlignBottom);
		tt.translate(tickLabelBr_.left(), 0.5*drawingSize.height());
		tt.scale(tt_.m11(), tt_.m22());;
		tt.rotate(-90);
		break;
	case OnRight:
		flags |= (Qt::AlignHCenter | Qt::AlignTop);
		tt.translate(tickLabelBr_.right(), 0.5*drawingSize.height());
		tt.scale(tt_.m11(), tt_.m22());
		tt.rotate(-90);
		break;
	}

	// save the old painter transform,
	painter->save();
	// translate and scale using tt:
	painter->setTransform(tt, true);

	QRectF br;	// used to return the effective bounding rectangle
	painter->drawText(container, flags, name_, &br);
	nameBr_ = tt.mapRect(br);

	// restore the painter transform so we don't mess it up for the next person who draws...
	painter->restore();

}


/// IntelliScale: Calculate "nice" values for starting tick and tick increment.
/*! Sets minTickVal_ and tickIncVal_ for nice values of axis ticks.
- Prior to calling, numTicks() must be set and axisScale() must be ready and correct.
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
void MPlotAxis::intelliScale() {
	if(numTicks() > 1) {

		// numTicks() is a suggestion for the minimum number of ticks.
		qreal crudeStep = (axisScale_->max() - axisScale_->min()) / numTicks();

		qreal step = pow(10, floor(log10(crudeStep)));
		if(5*step < crudeStep)
			step *= 5;
		else if(2*step < crudeStep)
			step *= 2;

		tickIncVal_ = step;
		minTickVal_ = ceil(axisScale_->min()/step) * step;

		// Hit Zero if possible: (while passing through origin)

		if(axisScale_->min() < 0 && axisScale_->max() > 0) {
			// the distance between 0 and the nearest tick is... the remainder in division of (0-minTickVal)/tickIncVal_.
			qreal offset = remainder(-minTickVal_, tickIncVal_);
			minTickVal_ += offset;
		}
	}

	else {	// 1 or zero ticks: 1 tick should go at the average/middle of the axis
		minTickVal_ = (axisScale_->min() + axisScale_->max()) / 2;
		// make sure the next tick is _well_ past the end of the axis, so that it doesn't get drawn.
		tickIncVal_ = DBL_MAX / 1e10; // Setting this to DLB_MAX causes an overflow that breaks minTickVal_.
	}
}

void MPlotAxis::setDefaults() {

	tickStyle_ = Outside;
	tickLength_ = .02;
	numTicks_ = 4;

	tickLabelOffset_ = 0.02;
	tickLabelFont_.setPointSize(12);
	axisNameFont_.setPointSize(12);

	if(placement_ == OnTop || placement_ == OnRight) {
		tickLabelsVisible_ = false;
		axisNameVisible_ = false;
	}
	else {
		tickLabelsVisible_ = true;
		axisNameVisible_ = true;
	}

	gridPen_ = QPen(QBrush(QColor(Qt::blue)), 1, Qt::DotLine);
	QVector<qreal> dashes;
	dashes << 4 << 4;
	gridPen_.setDashPattern(dashes);

	if(placement_ == OnLeft)
		gridVisible_ = true;
	else
		gridVisible_ = false;
}

void MPlotAxis::setAxisScale(MPlotAxisScale *newScale)
{
	if(axisScale_ == newScale)
		return;


	disconnect(axisScale_, 0, this, 0);

	onAxisDrawingSizeAboutToChange();

	axisScale_ = newScale;
	connect(axisScale_, SIGNAL(drawingSizeAboutToChange()), this, SLOT(onAxisDrawingSizeAboutToChange()));
	connect(axisScale_, SIGNAL(drawingSizeChanged()), this, SLOT(onAxisDrawingSizeChanged()));
	connect(axisScale_, SIGNAL(dataRangeAboutToChange()), this, SLOT(onAxisDataRangeAboutToChange()));
	connect(axisScale_, SIGNAL(dataRangeChanged()), this, SLOT(onAxisDataRangeChanged()));

	onAxisDrawingSizeChanged();
}


#endif

