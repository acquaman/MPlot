#include "MPlotColorLegend.h"

#include "MPlot.h"
#include "MPlotItem.h"

#include <QPainter>

MPlotColorLegend::MPlotColorLegend(MPlot *plot, QGraphicsItem *parent)
	: QGraphicsItem(parent)
{
	plot_ = plot;
}

QRectF MPlotColorLegend::boundingRect() const
{
	return QRectF(0, 0, 50, plot_->rect().height());
}

void MPlotColorLegend::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{

}
