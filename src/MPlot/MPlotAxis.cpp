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

	scaleFontsRequired_ = true;

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
}

MPlotAxis::~MPlotAxis() {
}


// Set the number and style of ticks:
/* \c tStyle can be Inside, Outside, or Middle.  \c tickLength is in logical coordinates ("percent of plot") coordinates.
 \c num is really just suggestion... there might be one less or up to three(?) more, depending on what we think would make nice label values.*/
void MPlotAxis::setTicks(int num, TickStyle tstyle, qreal tickLength) {

	prepareGeometryChange();

	numTicks_ = num;
	tickStyle_ = tstyle;
	tickLength_ = tickLength/100;

	update();
}

// show or hide the value labels along the axis
void MPlotAxis::showTickLabels(bool tickLabelsOn) {
	if(tickLabelsVisible_ == tickLabelsOn)
		return;

	prepareGeometryChange();

	tickLabelsVisible_ = tickLabelsOn;
	update();
}

// show or hide the grid lines
void MPlotAxis::showGrid(bool gridOn) {
	if(gridVisible_ == gridOn)
		return;

	prepareGeometryChange();

	gridVisible_ = gridOn;
	update();
}

// show or hide the axis name
void MPlotAxis::showAxisName(bool axisNameOn) {
	if(axisNameVisible_ == axisNameOn)
		return;

	prepareGeometryChange();

	axisNameVisible_ = axisNameOn;
	update();
}

// Set the pen for the axis line and axis name text:
void MPlotAxis::setAxisPen(const QPen& pen) {
	if(axisPen_ == pen)
		return;

	axisPen_ = pen;
	update();
}

// set the pen for the ticks along the axis:
void MPlotAxis::setTickPen(const QPen& pen) {
	if(tickPen_ == pen)
		return;

	tickPen_ = pen;
	update();
}

// set the pen for the grid lines
void MPlotAxis::setGridPen(const QPen& pen) {
	if(gridPen_ == pen)
		return;

	gridPen_ = pen;
	update();
}

// set the font used for the values along the axis
void MPlotAxis::setTickLabelFont(const QFont& font) {
	if(tickLabelFontU_ == font)
		return;

	prepareGeometryChange();

	tickLabelFontU_ = font;
	scaleFontsRequired_ = true;

	update();
}

// set the font used for the axis name
void MPlotAxis::setAxisNameFont(const QFont& font) {
	if(axisNameFontU_ == font)
		return;

	prepareGeometryChange();

	axisNameFontU_ = font;
	scaleFontsRequired_ = true;

	update();
}

// Set the axis name:
void MPlotAxis::setAxisName(const QString& name) {
	if(name_ == name)
		return;

	name_ = name;
	update();
}
// TODO: minor ticks

// Required functions:
//////////////////////////
// Bounding rect:
QRectF MPlotAxis::boundingRect() const {

	if(scaleFontsRequired_)
		scaleFonts();

	QSizeF drawingSize = axisScale_->drawingSize();

	qreal tickLength;
	QPointF topCorner;
	QSizeF size;

	switch(placement_) {
	case OnBottom:
		tickLength = drawingSize.height()*tickLength_;
		topCorner = QPointF(-5*tickLabelCharWidth_, drawingSize.height()-tickLength);
		size.setHeight(2*tickLength+2*tickLabelOffset_+axisNameHeight_+tickLabelHeight_);
		size.setWidth(10*tickLabelCharWidth_+drawingSize.width());
		break;
	case OnLeft:
		tickLength = drawingSize.width()*tickLength_;
		topCorner = QPointF(-tickLength-10*tickLabelCharWidth_-2*tickLabelOffset_-axisNameHeight_,
							-tickLabelHeight_/2);
		size.setHeight(drawingSize.height()+tickLabelHeight_);
		size.setWidth(2*tickLength+2*tickLabelOffset_+10*tickLabelCharWidth_+axisNameHeight_);
		break;
	case OnTop:
		tickLength = drawingSize.height()*tickLength_;
		topCorner = QPointF(-tickLength-2*tickLabelOffset_-tickLabelHeight_-axisNameHeight_,
							-5*tickLabelCharWidth_);
		size.setHeight(2*tickLength+2*tickLabelOffset_+tickLabelHeight_+axisNameHeight_);
		size.setWidth(10*tickLabelCharWidth_+drawingSize.width());
		break;
	case OnRight:
		tickLength = drawingSize.width()*tickLength_;
		topCorner = QPointF(-tickLength+drawingSize.width(), -axisNameHeight_/2);
		size.setWidth(2*tickLength+2*tickLabelOffset_+10*tickLabelCharWidth_+axisNameHeight_);
		size.setHeight(drawingSize.height()+tickLabelHeight_);
		break;
	}

	QRectF br(topCorner, size);

	// If the grids are on, they take up the whole rectangle
	if(gridVisible_)
		br |= QRectF(QPointF(0,0), drawingSize);

	return br;
}
// Paint:
void MPlotAxis::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {

	Q_UNUSED(widget)
	Q_UNUSED(option)

	if(scaleFontsRequired_)
		scaleFonts();

	QSizeF drawingSize = axisScale_->drawingSize();

	/// \todo Cache these so we don't have to re-calculate every time
	QList<qreal> ticks = axisScale_->calculateTickValues(numTicks_);

	switch(placement_) {
	case OnBottom: {
		// draw the main axis line:
		painter->setPen(axisPen_);
		painter->drawLine(QPointF(0, drawingSize.height()), QPointF(drawingSize.width(), drawingSize.height()));

		// draw the ticks
		qreal tickLength = drawingSize.height()*tickLength_;
		qreal tickTop = drawingSize.height(), tickBottom = drawingSize.height();
		switch(tickStyle_) {
		case Inside:
			tickTop -= tickLength;
			break;
		case Outside:
			tickBottom += tickLength;
			break;
		case Middle:
			tickTop -= tickLength/2;
			tickBottom += tickLength/2;
			break;
		}

		if(tickLabelsVisible_)
			painter->setFont(tickLabelFont_);

		foreach(qreal tickValue, ticks) {
			qreal x = axisScale_->mapDataToDrawing(tickValue);
			painter->setPen(tickPen_);
			// draw the tick
			painter->drawLine(QPointF(x, tickTop), QPointF(x, tickBottom));

			// draw the label

			if(tickLabelsVisible_) {
				painter->setPen(axisPen_);
				QRectF labelBox = QRectF(QPointF(x,tickBottom+tickLabelOffset_), QSizeF(0,0));
				painter->drawText(labelBox, Qt::AlignTop | Qt::AlignHCenter | Qt::TextDontClip, formatTickLabel(tickValue));
			}

			// draw the gridline
			if(gridVisible_) {
				painter->setPen(gridPen_);
				painter->drawLine(QPointF(x, 0), QPointF(x, drawingSize.height()));
			}
		}

		// draw the axis name
		if(axisNameVisible_) {
			painter->setFont(axisNameFont_);
			qreal axisNameTop = tickBottom + tickLabelOffset_;
			if(tickLabelsVisible_)
				axisNameTop += QFontMetrics(tickLabelFont_).height() + tickLabelOffset_;

			QRectF axisNameBox = QRectF(QPointF(drawingSize.width()/2, axisNameTop), QSizeF(0,0));
			painter->drawText(axisNameBox, Qt::AlignHCenter | Qt::AlignTop | Qt::TextDontClip, name_);
		}

	}
		break;



	case OnTop: {
		// draw the main axis line:
		painter->setPen(axisPen_);
		painter->drawLine(QPointF(0, 0), QPointF(drawingSize.width(), 0));

		// draw the ticks
		qreal tickLength = drawingSize.height()*tickLength_;
		qreal tickTop = 0, tickBottom = 0;
		switch(tickStyle_) {
		case Inside:
			tickTop -= tickLength;
			break;
		case Outside:
			tickBottom += tickLength;
			break;
		case Middle:
			tickTop -= tickLength/2;
			tickBottom += tickLength/2;
			break;
		}

		if(tickLabelsVisible_)
			painter->setFont(tickLabelFont_);

		foreach(qreal tickValue, ticks) {
			qreal x = axisScale_->mapDataToDrawing(tickValue);
			painter->setPen(tickPen_);
			// draw the tick
			painter->drawLine(QPointF(x, tickTop), QPointF(x, tickBottom));

			// draw the label
			if(tickLabelsVisible_) {
				painter->setPen(axisPen_);
				QRectF labelBox = QRectF(QPointF(x,tickTop-tickLabelOffset_), QSizeF(0,0));
				painter->drawText(labelBox, Qt::AlignBottom | Qt::AlignHCenter | Qt::TextDontClip, QString::number(tickValue));
			}

			// draw the gridline
			if(gridVisible_) {
				painter->setPen(gridPen_);
				painter->drawLine(QPointF(x, 0), QPointF(x, drawingSize.height()));
			}
		}

		// draw the axis name
		if(axisNameVisible_) {
			painter->setFont(axisNameFont_);
			qreal axisNameBottom = tickTop - tickLabelOffset_;
			if(tickLabelsVisible_)
				axisNameBottom -= (QFontMetrics(tickLabelFont_).height() + tickLabelOffset_);

			QRectF axisNameBox = QRectF(QPointF(drawingSize.width()/2, axisNameBottom), QSizeF(0,0));
			painter->drawText(axisNameBox, Qt::AlignHCenter | Qt::AlignBottom | Qt::TextDontClip, name_);
		}
	}


		break;
	case OnLeft: {
		// draw the main axis line:
		painter->setPen(axisPen_);
		painter->drawLine(QPointF(0, 0), QPointF(0, drawingSize.height()));

		// draw the ticks
		qreal tickLength = drawingSize.width()*tickLength_;
		qreal tickLeft = 0, tickRight = 0;
		switch(tickStyle_) {
		case Outside:
			tickLeft -= tickLength;
			break;
		case Inside:
			tickRight += tickLength;
			break;
		case Middle:
			tickLeft -= tickLength/2;
			tickRight += tickLength/2;
			break;
		}

		qreal maxLabelWidth = 0;

		if(tickLabelsVisible_)
			painter->setFont(tickLabelFont_);

		foreach(qreal tickValue, ticks) {
			qreal y = axisScale_->mapDataToDrawing(tickValue);
			painter->setPen(tickPen_);
			// draw the tick
			painter->drawLine(QPointF(tickLeft, y), QPointF(tickRight, y));

			// draw the label
			if(tickLabelsVisible_) {
				painter->setPen(axisPen_);
				QRectF labelBox = QRectF(QPointF(tickLeft-tickLabelOffset_,y), QSizeF(0,0));
				QRectF actualLabelBox;
				painter->drawText(labelBox, Qt::AlignRight | Qt::AlignVCenter | Qt::TextDontClip, QString::number(tickValue), &actualLabelBox);
				if(actualLabelBox.width() > maxLabelWidth)
					maxLabelWidth = actualLabelBox.width();
			}

			// draw the gridline
			if(gridVisible_) {
				painter->setPen(gridPen_);
				painter->drawLine(QPointF(0, y), QPointF(drawingSize.width(), y));
			}
		}

		// draw the axis name
		if(axisNameVisible_) {
			painter->setFont(axisNameFont_);
			qreal axisNameRight = tickLeft - tickLabelOffset_;
			if(tickLabelsVisible_)
				axisNameRight -= (maxLabelWidth + tickLabelOffset_);

			QTransform tt;
			tt.translate(axisNameRight, drawingSize.height()/2);
			tt.rotate(-90);
			painter->save();
			painter->setTransform(tt, true);
			painter->drawText(QRectF(0,0,0,0), Qt::AlignHCenter | Qt::AlignBottom | Qt::TextDontClip, name_);
			painter->restore();
		}
	}
		break;
	case OnRight: {
		// draw the main axis line:
		painter->setPen(axisPen_);
		painter->drawLine(QPointF(drawingSize.width(), 0), QPointF(drawingSize.width(), drawingSize.height()));

		// draw the ticks
		qreal tickLength = drawingSize.width()*tickLength_;
		qreal tickLeft = drawingSize.width(), tickRight = drawingSize.width();
		switch(tickStyle_) {
		case Inside:
			tickLeft -= tickLength;
			break;
		case Outside:
			tickRight += tickLength;
			break;
		case Middle:
			tickLeft -= tickLength/2;
			tickRight += tickLength/2;
			break;
		}

		qreal maxLabelWidth = 0;

		if(tickLabelsVisible_)
			painter->setFont(tickLabelFont_);

		foreach(qreal tickValue, ticks) {
			qreal y = axisScale_->mapDataToDrawing(tickValue);
			painter->setPen(tickPen_);
			// draw the tick
			painter->drawLine(QPointF(tickLeft, y), QPointF(tickRight, y));

			// draw the label
			if(tickLabelsVisible_) {
				painter->setPen(axisPen_);
				QRectF labelBox = QRectF(QPointF(tickRight+tickLabelOffset_,y), QSizeF(0,0));
				QRectF actualLabelBox;
				painter->drawText(labelBox, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextDontClip, QString::number(tickValue), &actualLabelBox);
				if(actualLabelBox.width() > maxLabelWidth)
					maxLabelWidth = actualLabelBox.width();
			}

			// draw the gridline
			if(gridVisible_) {
				painter->setPen(gridPen_);
				painter->drawLine(QPointF(0, y), QPointF(drawingSize.width(), y));
			}
		}

		// draw the axis name
		if(axisNameVisible_) {
			painter->setFont(axisNameFont_);
			qreal axisNameRight = tickRight + tickLabelOffset_;
			if(tickLabelsVisible_)
				axisNameRight += (maxLabelWidth + tickLabelOffset_);

			QTransform tt;
			tt.translate(axisNameRight, drawingSize.height()/2);
			tt.rotate(90);
			painter->save();
			painter->setTransform(tt, true);
			painter->drawText(QRectF(0,0,0,0), Qt::AlignHCenter | Qt::AlignBottom | Qt::TextDontClip, name_);
			painter->restore();
		}
	}
		break;
	}
}

// TODO: finer shape?
/*
 QPainterPath shape() {
  return
 }*/

///// Calculates a transform suitable for applying to the painter to draw undistorted text.
///*! The text will shrink and grow with the size of the plot, but only within a reasonable range. (ie: infinitely small text isn't helpful, and super-humongous text isn't helpful).
//  Result is saved in tt_ */
//void MPlotAxis::calcTextTransform(QPainter* painter) {

//	// debugging only... no transform
//	tt_ = QTransform();
//	return;


//	// old:

//	// World transform: tells how we would get back into actual pixel coordinates
//	QTransform wt = painter->deviceTransform();	// equivalent to worldTransform and combinedTransform

//	// "unscale" to get rid of different scaling in x and y:
//	tt_ = QTransform::fromScale(1/wt.m11(), 1/wt.m22());

//	// reintroduce a bit of dependence on the plot size, but constrain with a min/max range.
//	// use the smaller dimension as the relevant one to base this on.
//	qreal scaleFactor;
//	if(fabs(wt.m11()) > fabs(wt.m22()))
//		scaleFactor = qBound((qreal)0.8, fabs(wt.m22()/250), (qreal)1.2);
//	else
//		scaleFactor = qBound((qreal)0.8, fabs(wt.m11()/250), (qreal)1.2);
//	tt_.scale(scaleFactor, scaleFactor);


//}

///// Used to draw the axis name (ex: "x" or "time (s)") onto the plot.  Draw the axis labels first so we know where to put this.
//void MPlotAxis::drawAxisName(QPainter* painter) {

//	// Text Transform: will be applied to place text at correct location, scaled back to normal size
//	QTransform tt;

//	// Don't clip if we overflow the container (because we're making a tiny container)
//	int flags = Qt::TextDontClip;
//	// Create a container rectangle centered on the tick position:
//	QRectF container(0,0,0,0);

//	QSizeF drawingSize = axisScale_->drawingSize();

//	switch(placement_) {
//	case OnBottom:
//		flags |= (Qt::AlignHCenter | Qt::AlignTop);
//		tt.translate(0.5*drawingSize.width(), tickLabelBr_.top() );
//		tt.scale(tt_.m11(), tt_.m22());
//		break;
//	case OnTop:
//		flags |= (Qt::AlignHCenter | Qt::AlignBottom);
//		tt.translate(0.5*drawingSize.width(), tickLabelBr_.bottom() );
//		tt.scale(tt_.m11(), tt_.m22());
//		break;
//	case OnLeft:
//		flags |= (Qt::AlignHCenter | Qt::AlignBottom);
//		tt.translate(tickLabelBr_.left(), 0.5*drawingSize.height());
//		tt.scale(tt_.m11(), tt_.m22());;
//		tt.rotate(-90);
//		break;
//	case OnRight:
//		flags |= (Qt::AlignHCenter | Qt::AlignTop);
//		tt.translate(tickLabelBr_.right(), 0.5*drawingSize.height());
//		tt.scale(tt_.m11(), tt_.m22());
//		tt.rotate(-90);
//		break;
//	}

//	// save the old painter transform,
//	painter->save();
//	// translate and scale using tt:
//	painter->setTransform(tt, true);

//	QRectF br;	// used to return the effective bounding rectangle
//	painter->drawText(container, flags, name_, &br);
//	nameBr_ = tt.mapRect(br);

//	// restore the painter transform so we don't mess it up for the next person who draws...
//	painter->restore();

//}

void MPlotAxis::setDefaults() {

	tickStyle_ = Outside;
	tickLength_ = .02;
	numTicks_ = 4;

	fontsShouldScale_ = true;

	if(placement_ == OnTop || placement_ == OnBottom)
		tickLabelOffset_ = 2;
	else
		tickLabelOffset_ = 6; // in pixels.  A constant; doesn't grow with the plot size like the tickLength_, which is expressed in percent of plot width/height.

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

QFont MPlotAxis::scaleFontToDrawingSize(const QFont &sourceFont) const
{
	QFont rv = sourceFont;

	qreal diagonal = sqrt(pow(axisScale_->drawingSize().width(),2) + pow(axisScale_->drawingSize().height(), 2));

	rv.setPointSizeF(qBound(8.5, 12.0*diagonal/600,18.0));

	return rv;
}

void MPlotAxis::scaleFonts() const
{
	axisNameFont_ = fontsShouldScale_ ? scaleFontToDrawingSize(axisNameFontU_) : axisNameFontU_;
	tickLabelFont_ = fontsShouldScale_ ? scaleFontToDrawingSize(tickLabelFontU_) : tickLabelFontU_;

	axisNameHeight_ = QFontMetrics(axisNameFont_).height();

	QFontMetrics fm(tickLabelFont_);
	tickLabelCharWidth_ = fm.width(QChar('8'));
	tickLabelHeight_ = fm.height();

	scaleFontsRequired_ = false;
}

void MPlotAxis::setFontsScaleWithDrawingSize(bool fontsShouldScale) {
	if(fontsShouldScale_ == fontsShouldScale)
		return;

	prepareGeometryChange();
	fontsShouldScale_ = fontsShouldScale;
	scaleFontsRequired_ = true;
	update();
}

QString MPlotAxis::formatTickLabel(double tickValue)
{
	qreal rangeMin = axisScale_->min();
	qreal rangeMax = axisScale_->max();

	// in this situation, we need to look out for "almost-zero" values that need rounding to exactly zero
	if(rangeMin < 0 && rangeMax > 0) {
		// significance is, let's say, 8 orders of magnitude below the full range
		qreal significance = (rangeMax - rangeMin) / 1e8; //ex: range is (-0.5, 0.5)... significance = 1e-8

		qreal truncated = round(tickValue/significance) * significance;
		return QString::number(truncated);
	}
	else {
		return QString::number(tickValue);
	}
}

#endif

