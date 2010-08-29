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

// Returns the current marker, which can be used to access it's pen, brush, and size.
// If the plot has no marker (or MPlotMarkerShape::None), then this will be a null pointer. Must check before setting.
MPlotAbstractMarker* MPlotPoint::marker() const { return marker_; }

void MPlotPoint::setMarker(MPlotMarkerShape::Shape shape, double size, const QPen& pen, const QBrush& brush) {
	if(marker_)
		delete marker_;

	QPen realPen(pen);
	realPen.setCosmetic(true);
	marker_ = MPlotMarker::create(shape, size, realPen, brush);
	update();
}

/// returns the point location of this marker:
QPointF MPlotPoint::value() const {
	return point_;
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
// Bounding rect: reported in our PlotSeries coordinates, which are just the actual data coordinates. This is used by the graphics view system to figure out how much we cover/need to redraw.  Subclasses that draw selection borders or markers need to add their size on top of this.
QRectF MPlotPoint::boundingRect() const {

	QRectF br = dataRect();

	// expand by the selection line width (in pixels...)
	QRectF hs = QRectF(0, 0, MPLOT_SELECTION_LINEWIDTH, MPLOT_SELECTION_LINEWIDTH);

	// expand by marker size (if expressed in pixels)
	if(marker())
		hs |= QRectF(0,0, marker()->size(), marker()->size());


	// these sizes so far are in pixels (hopefully scene coordinates... trusting on an untransformed view.) Converting to local (data) coordinates.
	hs = mapRectFromScene(hs);



	// really we just need 1/2 the marker size and 1/2 the selection highlight width. But extra doesn't hurt.
	br.adjust(-hs.width(),-hs.height(),hs.width(), hs.height());

	return br;
}

// Data rect: also reported in our PlotSeries coordinates, which are the actual data coordinates. This is used by the auto-scaling to figure out the range of our data on an axis.
QRectF MPlotPoint::dataRect() const {
	return QRectF(point_, QSizeF(0,0));
}

// Paint: must be implemented in subclass.
void MPlotPoint::paint(QPainter* painter,
				   const QStyleOptionGraphicsItem* option,
				   QWidget* widget)
{
	Q_UNUSED(option)
	Q_UNUSED(widget)

	if(marker_) {
		QTransform wt = painter->deviceTransform();	// equivalent to worldTransform() and combinedTransform()


		QTransform wtInverse(QTransform::fromScale(1/wt.m11(), 1/wt.m22()));
		//wtInverse.scale(1/wt.m11(), 1/wt.m22());

		// Paint marker:
		painter->save();
		painter->setPen(marker_->pen());
		painter->setBrush(marker_->brush());

		painter->translate(point_.x(), point_.y());
		painter->setTransform(wtInverse, true);
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

	setYAxisTarget(MPlotAxis::Left);

	setMarker(MPlotMarkerShape::CrossCircle, 24, QPen(QColor(Qt::red), 0), QBrush());

	QColor selectionColor = MPLOT_SELECTION_COLOR;
	selectionColor.setAlphaF(MPLOT_SELECTION_OPACITY);
	selectedPen_ = QPen(QBrush(selectionColor), MPLOT_SELECTION_LINEWIDTH);
	selectedPen_.setCosmetic(true);

}


#endif // MPLOTPOINT_H

