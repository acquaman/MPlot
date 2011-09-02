#include "MPlotRectangle.h"
#include <QPainter>

MPlotRectangle::MPlotRectangle(const QRectF& rect, const QPen& pen, const QBrush& brush)
	: pen_(pen), brush_(brush), rect_(rect)
{
}

QRectF MPlotRectangle::boundingRect() const
{
	QRectF br = MPlotItem::boundingRect();

	// expand by the selection line width (in pixels...)
	QRectF hs = QRectF(0, 0, MPLOT_SELECTION_LINEWIDTH, MPLOT_SELECTION_LINEWIDTH);

	// expand by pen border size
	hs |= QRectF(0,0, pen_.widthF(), pen_.widthF());

	// really we just need 1/2 the marker size and 1/2 the selection highlight width. But extra doesn't hurt.
	br.adjust(-hs.width(),-hs.height(),hs.width(), hs.height());

	return br;
}

void MPlotRectangle::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	Q_UNUSED(option)
	Q_UNUSED(widget)

	QRectF drawingRect = MPlotItem::boundingRect();

	painter->setPen(pen_);
	painter->setBrush(brush_);
	painter->drawRect(drawingRect);

	if(selected()) {
		QColor selectionColor = MPLOT_SELECTION_COLOR;
		painter->setPen(QPen(QBrush(selectionColor), MPLOT_SELECTION_LINEWIDTH));
		selectionColor.setAlphaF(MPLOT_SELECTION_OPACITY);
		painter->setBrush(QBrush(selectionColor));
		painter->drawRect(drawingRect);
	}
}
