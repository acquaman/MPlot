#ifndef MPLOTPOINT_CPP
#define MPLOTPOINT_CPP

#include "MPlotPoint.h"


/// Default constructor. Override setDefaults() for a custom look off the bat.
MPlotPoint::MPlotPoint(const QPointF& value)
	: MPlotItem(),
	point_(value)
{
	marker_ = 0;

	setDefaults();
}

MPlotPoint::~MPlotPoint() {
	delete marker_;
}


void MPlotPoint::setMarker(MPlotMarkerShape::Shape shape, qreal size, const QPen& pen, const QBrush& brush) {
	if(marker_)
		delete marker_;

	QPen realPen(pen);
	marker_ = MPlotMarker::create(shape, size, realPen, brush);
	update();
}



/// set the point location of this marker:
void MPlotPoint::setValue(const QPointF& point) {

	prepareGeometryChange();

	point_ = point;
	emitBoundsChanged();

	update();
}


// Required functions:
//////////////////////////
// Bounding rect: reported in drawing coordinates. This is used by the graphics view system to figure out how much we cover/need to redraw.  Subclasses that draw selection borders or markers (like us) need to add their size on top of the base class.
QRectF MPlotPoint::boundingRect() const {

	QRectF br = MPlotItem::boundingRect();

	// expand by the selection line width (in pixels...)
	QRectF hs = QRectF(0, 0, MPLOT_SELECTION_LINEWIDTH, MPLOT_SELECTION_LINEWIDTH);

	// expand by marker size (if expressed in pixels)
	if(marker())
		hs |= QRectF(0,0, marker()->size(), marker()->size());


	// these sizes so far are in pixels (hopefully scene coordinates... trusting on an untransformed view.) Converting to local (data) coordinates.
	// UNNECESSARY: hs = mapRectFromScene(hs);

	// really we just need 1/2 the marker size and 1/2 the selection highlight width. But extra doesn't hurt.
	br.adjust(-hs.width(),-hs.height(),hs.width(), hs.height());

	return br;
}

QRectF MPlotPoint::dataRect() const {
	return QRectF(point_, QSizeF(0,0));
}

void MPlotPoint::paint(QPainter* painter,
				   const QStyleOptionGraphicsItem* option,
				   QWidget* widget)
{
	Q_UNUSED(option)
	Q_UNUSED(widget)

	if(marker_) {

		// Paint marker:
		painter->save();
		painter->setPen(marker_->pen());
		painter->setBrush(marker_->brush());

		painter->translate(mapX(point_.x()), mapY(point_.y()));
		marker_->paint(painter);

		// repaint with selected pen, if selected...
		if(selected()) {
			painter->setPen(selectedPen_);
			marker_->paint(painter);
		}

		painter->restore();
	}
}



void MPlotPoint::setDefaults() {

	setMarker(MPlotMarkerShape::CrossCircle, 24, QPen(QColor(Qt::red), 0), QBrush());

	QColor selectionColor = MPLOT_SELECTION_COLOR;
	selectionColor.setAlphaF(MPLOT_SELECTION_OPACITY);
	selectedPen_ = QPen(QBrush(selectionColor), MPLOT_SELECTION_LINEWIDTH);
	selectedPen_.setCosmetic(true);

}


#endif // MPLOTPOINT_H

