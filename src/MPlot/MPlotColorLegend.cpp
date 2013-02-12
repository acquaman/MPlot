#include "MPlotColorLegend.h"

#include "MPlot.h"
#include "MPlotItem.h"
#include "MPlot/MPlotImage.h"

#include <QPainter>

MPlotColorLegend::MPlotColorLegend(MPlot *plot, QGraphicsItem *parent)
	: QGraphicsItem(parent)
{
	plot_ = plot;

	setFlags(flags() | QGraphicsItem::ItemIsMovable);
}

QRectF MPlotColorLegend::boundingRect() const
{
	return QRectF(topLeft_.x(), topLeft_.y(), 70, plot_->rect().height());
}

void MPlotColorLegend::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	Q_UNUSED(option)
	Q_UNUSED(widget)

	if (plot_->imageItemsCount() > 0){

		painter->save();
		painter->translate(topLeft_);

		MPlotAbstractImage *image = 0;

		for (int i = 0, size = plot_->numItems(); i < size && image == 0; i++)
			if (qgraphicsitem_cast<MPlotAbstractImage*>(plot_->item(i)))
				image = qgraphicsitem_cast<MPlotAbstractImage*>(plot_->item(i));

		if (!image)
			return;

		const MPlotAbstractImageData *data = image->model();
		MPlotInterval dataRange = data->range();

		qreal height = 0.8*plot_->rect().height();
		QBrush brush(image->colorMap().colorAt(dataRange.second));
		qreal delH = height/boxNumber_;
		qreal delD = (dataRange.second-dataRange.first)/boxNumber_;

		for (int i = 0; i <= boxNumber_; i++){

			brush.setColor(image->colorMap().colorAt((dataRange.second-i*delD), dataRange));
			painter->setBrush(brush);
			painter->drawRect(QRectF(30, i*delH+20, 25, delH));
		}

		painter->drawText(QRectF(0, 0, 70, 20), QString::number(dataRange.second, 'e', 2));
		painter->drawText(QRectF(0,  height+delH+20, 70, 20), QString::number(dataRange.first, 'e', 2));

		painter->restore();
	}
}
