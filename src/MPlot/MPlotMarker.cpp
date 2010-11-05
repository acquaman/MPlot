#ifndef __MPlotMarker_CPP__
#define __MPlotMarker_CPP__


#include "MPlotMarker.h"


using namespace MPlotMarkerShape;

MPlotAbstractMarker::MPlotAbstractMarker(double size, const QPen& pen, const QBrush& brush) :
		size_(size), pen_(pen), brush_(brush)
{
}

MPlotAbstractMarker::~MPlotAbstractMarker()
{
}

void MPlotAbstractMarker::setSize(double size) {
	size_ = size;
}

double MPlotAbstractMarker::size() {
	return size_;
}

QPen MPlotAbstractMarker::pen() const {
	return pen_;
}

void MPlotAbstractMarker::setPen(const QPen &pen1) {
	pen_ = pen1;
	pen_.setCosmetic(true);
}

QBrush MPlotAbstractMarker::brush() const {
	return brush_;
}

void MPlotAbstractMarker::setBrush(const QBrush &brush) {
	brush_ = brush;
}



MPlotMarkerSquare::MPlotMarkerSquare(double size, const QPen& pen, const QBrush& brush) :
		MPlotAbstractMarker(size, pen, brush)
{
}

void MPlotMarkerSquare::paint(QPainter* painter) {
	QRectF me(-size_/2, -size_/2, size_, size_);
	painter->drawRect(me);
}



MPlotMarkerCircle::MPlotMarkerCircle(double size, const QPen& pen, const QBrush& brush) :
		MPlotAbstractMarker(size, pen, brush)
{
}

void MPlotMarkerCircle::paint(QPainter* painter) {
	painter->drawEllipse(QRectF(-size_/2, -size_/2, size_, size_));
}



MPlotMarkerTriangle::MPlotMarkerTriangle(double size, const QPen& pen, const QBrush& brush) :
		MPlotAbstractMarker(size, pen, brush) {
	setSize(size);
}

void MPlotMarkerTriangle::setSize(double width) {
	size_ = width;
	triangle_ << QPointF(-width/2, width/2/sqrt(3));
	triangle_ << QPointF(width/2, width/2/sqrt(3));
	triangle_ << QPointF(0, -width/sqrt(3));
	triangle_ << QPointF(-width/2, width/2/sqrt(3));
}

void MPlotMarkerTriangle::paint(QPainter* painter) {
	painter->drawPolygon(triangle_);
}



MPlotMarkerVerticalBeam::MPlotMarkerVerticalBeam(double size, const QPen& pen, const QBrush& brush) :
		MPlotAbstractMarker(size, pen, brush) {
	setSize(size);
}

void MPlotMarkerVerticalBeam::setSize(double width) {
	size_ = width;
	line_.setLine(0, size_/2, 0, -size_/2);
}

void MPlotMarkerVerticalBeam::paint(QPainter* painter) {
	painter->drawLine(line_);
}




MPlotMarkerHorizontalBeam::MPlotMarkerHorizontalBeam(double size, const QPen& pen, const QBrush& brush) :
		MPlotAbstractMarker(size, pen, brush)
{
	setSize(size);
}

void MPlotMarkerHorizontalBeam::setSize(double width) {
	size_ = width;
	line_.setLine(size_/2, 0, -size_/2, 0);
}

void MPlotMarkerHorizontalBeam::paint(QPainter* painter) {
	painter->drawLine(line_);
}




MPlotMarkerPoint::MPlotMarkerPoint(double size, const QPen& pen, const QBrush& brush) :
		MPlotAbstractMarker(size, pen, brush)
{
	setSize(size);
}

void MPlotMarkerPoint::setSize(double) {
	size_ = 0.1;
	line_.setLine(size_/2, 0, -size_/2, 0);
}

void MPlotMarkerPoint::paint(QPainter* painter) {
	painter->drawLine(line_);
}





MPlotMarkerDiagDownLeft::MPlotMarkerDiagDownLeft(double size, const QPen& pen, const QBrush& brush) :
		MPlotAbstractMarker(size, pen, brush)
{
	setSize(size);
}

void MPlotMarkerDiagDownLeft::setSize(double size) {
	size_ = size;
	line_.setLine(-size/2, size/2, size/2, -size/2);
}

void MPlotMarkerDiagDownLeft::paint(QPainter* painter) {
	painter->drawLine(line_);
}





MPlotMarkerDiagDownLeftR::MPlotMarkerDiagDownLeftR(double size, const QPen& pen, const QBrush& brush) :
		MPlotAbstractMarker(size, pen, brush)
{
	setSize(size);
}

void MPlotMarkerDiagDownLeftR::setSize(double size) {
	size_ = size;
	double lo2 = size_/2/sqrt(2);
	line_.setLine(-lo2, lo2, lo2, -lo2);
}

void MPlotMarkerDiagDownLeftR::paint(QPainter* painter) {
	painter->drawLine(line_);
}




MPlotMarkerDiagDownRight::MPlotMarkerDiagDownRight(double size, const QPen& pen, const QBrush& brush) :
		MPlotAbstractMarker(size, pen, brush)
{
	setSize(size);
}

void MPlotMarkerDiagDownRight::setSize(double size) {
	size_ = size;
	line_.setLine(-size/2, -size/2, size/2, size/2);
}

void MPlotMarkerDiagDownRight::paint(QPainter* painter) {
	painter->drawLine(line_);
}




MPlotMarkerDiagDownRightR::MPlotMarkerDiagDownRightR(double size, const QPen& pen, const QBrush& brush) :
		MPlotAbstractMarker(size, pen, brush)
{
	setSize(size);
}

void MPlotMarkerDiagDownRightR::setSize(double size) {
	size_ = size;
	double lo2 = size_/2/sqrt(2);
	line_.setLine(-lo2, -lo2, lo2, lo2);
}

void MPlotMarkerDiagDownRightR::paint(QPainter* painter) {
	painter->drawLine(line_);
}



MPlotMarkerCombined::MPlotMarkerCombined(int shapeCode, double size, const QPen& pen, const QBrush& brush) :
		MPlotAbstractMarker(size, pen, brush)
{

	if(shapeCode & MPlotMarkerShape::Square) {
		elements_ << new MPlotMarkerSquare(size, pen, brush);
	}
	if(shapeCode & MPlotMarkerShape::Circle) {
		elements_ << new MPlotMarkerCircle(size, pen, brush);
	}
	if(shapeCode & MPlotMarkerShape::Triangle) {
		elements_ << new MPlotMarkerTriangle(size, pen, brush);
	}
	if(shapeCode & MPlotMarkerShape::VerticalBeam) {
		elements_ << new MPlotMarkerVerticalBeam(size, pen, brush);
	}
	if(shapeCode & MPlotMarkerShape::HorizontalBeam) {
		elements_ << new MPlotMarkerHorizontalBeam(size, pen, brush);
	}
	if(shapeCode & MPlotMarkerShape::DiagDownLeft) {
		elements_ << new MPlotMarkerDiagDownLeft(size, pen, brush);
	}
	if(shapeCode & MPlotMarkerShape::DiagDownRight) {
		elements_ << new MPlotMarkerDiagDownRight(size, pen, brush);
	}
	if(shapeCode & MPlotMarkerShape::DiagDownLeftR) {
		elements_ << new MPlotMarkerDiagDownLeftR(size, pen, brush);
	}
	if(shapeCode & MPlotMarkerShape::DiagDownRightR) {
		elements_ << new MPlotMarkerDiagDownRightR(size, pen, brush);
	}
	if(shapeCode & MPlotMarkerShape::Point) {
		elements_ << new MPlotMarkerPoint(size, pen, brush);
	}

}

MPlotMarkerCombined::~MPlotMarkerCombined() {
	while(!elements_.isEmpty())
		delete elements_.takeFirst();
}

void MPlotMarkerCombined::setSize(double size) {
	foreach(MPlotAbstractMarker* element, elements_) {
		element->setSize(size);
	}
}

void MPlotMarkerCombined::setPen(const QPen &pen) {
	pen_ = pen;
	foreach(MPlotAbstractMarker* element, elements_) {
		element->setPen(pen);
	}
}

void MPlotMarkerCombined::setBrush(const QBrush &brush) {
	brush_ = brush;
	foreach(MPlotAbstractMarker* element, elements_) {
		element->setBrush(brush);
	}
}



void MPlotMarkerCombined::paint(QPainter* painter) {
	foreach(MPlotAbstractMarker* element, elements_) {
		element->paint(painter);
	}
}





// Static creator function:
MPlotAbstractMarker* MPlotMarker::create(MPlotMarkerShape::Shape type, double size, const QPen& pen, const QBrush& brush)
{
	switch(type) {
	case MPlotMarkerShape::None: return 0;
		break;
	case MPlotMarkerShape::Square: return new MPlotMarkerSquare(size, pen, brush);
		break;
	case MPlotMarkerShape::Circle: return new MPlotMarkerCircle(size, pen, brush);
		break;
	case MPlotMarkerShape::Triangle: return new MPlotMarkerTriangle(size, pen, brush);
		break;
	case MPlotMarkerShape::VerticalBeam: return new MPlotMarkerVerticalBeam(size, pen, brush);
		break;
	case MPlotMarkerShape::HorizontalBeam: return new MPlotMarkerHorizontalBeam(size, pen, brush);
		break;
	case MPlotMarkerShape::DiagDownLeft: return new MPlotMarkerDiagDownLeft(size, pen, brush);
		break;
	case MPlotMarkerShape::DiagDownRight: return new MPlotMarkerDiagDownRight(size, pen, brush);
		break;
	case MPlotMarkerShape::DiagDownLeftR: return new MPlotMarkerDiagDownLeftR(size, pen, brush);
		break;
	case MPlotMarkerShape::DiagDownRightR: return new MPlotMarkerDiagDownRightR(size, pen, brush);
		break;
	case MPlotMarkerShape::Cross: return new MPlotMarkerCombined(MPlotMarkerShape::VerticalBeam | MPlotMarkerShape::HorizontalBeam, size, pen, brush);
		break;
	case MPlotMarkerShape::CrossSquare: return new MPlotMarkerCombined(MPlotMarkerShape::Square | MPlotMarkerShape::VerticalBeam | MPlotMarkerShape::HorizontalBeam, size, pen, brush);
		break;
	case MPlotMarkerShape::CrossCircle: return new MPlotMarkerCombined(MPlotMarkerShape::Circle | MPlotMarkerShape::VerticalBeam | MPlotMarkerShape::HorizontalBeam, size, pen, brush);
		break;
	case MPlotMarkerShape::X: return new MPlotMarkerCombined(MPlotMarkerShape::DiagDownLeft | MPlotMarkerShape::DiagDownRight, size, pen, brush);
		break;
	case MPlotMarkerShape::XSquare: return new MPlotMarkerCombined(MPlotMarkerShape::DiagDownLeft | MPlotMarkerShape::DiagDownRight | MPlotMarkerShape::Square, size, pen, brush);
		break;
	case MPlotMarkerShape::XCircle: return new MPlotMarkerCombined(MPlotMarkerShape::DiagDownLeftR | MPlotMarkerShape::DiagDownRightR | MPlotMarkerShape::Circle, size, pen, brush);
		break;
	case MPlotMarkerShape::Star: return new MPlotMarkerCombined(MPlotMarkerShape::DiagDownLeftR | MPlotMarkerShape::DiagDownRightR | MPlotMarkerShape::VerticalBeam | MPlotMarkerShape::HorizontalBeam, size, pen, brush);
		break;
	case MPlotMarkerShape::StarCircle: return new MPlotMarkerCombined(MPlotMarkerShape::DiagDownLeftR | MPlotMarkerShape::DiagDownRightR | MPlotMarkerShape::VerticalBeam | MPlotMarkerShape::HorizontalBeam | MPlotMarkerShape::Circle, size, pen, brush);
		break;
	case MPlotMarkerShape::StarSquare: return new MPlotMarkerCombined(MPlotMarkerShape::DiagDownLeft | MPlotMarkerShape::DiagDownRight | MPlotMarkerShape::VerticalBeam | MPlotMarkerShape::HorizontalBeam | MPlotMarkerShape::Square, size, pen, brush);
		break;
	case MPlotMarkerShape::Point: return new MPlotMarkerPoint(size, pen, brush);
		break;
	case MPlotMarkerShape::PointSquare: return new MPlotMarkerCombined(MPlotMarkerShape::Point | MPlotMarkerShape::Square, size, pen, brush);
		break;
	case MPlotMarkerShape::PointCircle: return new MPlotMarkerCombined(MPlotMarkerShape::Point | MPlotMarkerShape::Circle, size, pen, brush);
		break;

	default: return new MPlotMarkerSquare(size, pen, brush);

		break;
	}

}



#endif

