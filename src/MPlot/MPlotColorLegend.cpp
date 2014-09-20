#include "MPlotColorLegend.h"

#include "MPlot.h"
#include "MPlotItem.h"
#include "MPlot/MPlotImage.h"
#include "MPlot/MPlotImageRangeDialog.h"

#include <QPainter>

MPlotColorLegendSignalHandler::MPlotColorLegendSignalHandler(MPlotColorLegend *parent)
	: QObject(0)
{
	legend_ = parent;
}

void MPlotColorLegendSignalHandler::onDataChanged()
{
	legend_->onDataChangedPrivate();
}

MPlotColorLegend::MPlotColorLegend(MPlot *plot, QGraphicsItem *parent)
	: QGraphicsItem(parent)
{
	plot_ = plot;
	image_ = 0;
	signalHandler_ = new MPlotColorLegendSignalHandler(this);

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

		if (!image_){

			MPlotAbstractImage *image = 0;

			for (int i = 0, size = plot_->numItems(); i < size && image == 0; i++)
				if (qgraphicsitem_cast<MPlotAbstractImage*>(plot_->item(i)))
					image = qgraphicsitem_cast<MPlotAbstractImage*>(plot_->item(i));

			if (!image)
				return;

			image_ = image;
			QObject::connect(image_->model()->signalSource(), SIGNAL(dataChanged()), signalHandler_, SLOT(onDataChanged()));
		}

		MPlotInterval dataRange = image_->range();

		qreal height = 0.75*plot_->rect().height();
		MPlotColorMap colorMap = image_->colorMap();
		QBrush brush(colorMap.colorAt(dataRange.second));
		qreal delH = height/boxNumber_;
		qreal delD = (dataRange.second-dataRange.first)/boxNumber_;

		for (int i = 0; i <= boxNumber_; i++){

			brush.setColor(colorMap.colorAt((dataRange.second-i*delD), dataRange));
			painter->setBrush(brush);
			painter->drawRect(QRectF(30, i*delH+40, 25, delH));
		}

		painter->drawText(QRectF(5, 20, 60, 40), QString("%1").arg(dataRange.second, 0, 'e', 2));
		painter->drawText(QRectF(5, height+delH+40, 60, 40), QString("%1").arg(dataRange.first, 0, 'e', 2));

		painter->restore();
	}
}

void MPlotColorLegend::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
	MPlotImageRangeDialog dialog(image_);
	dialog.exec();
	update();
	QGraphicsItem::mouseDoubleClickEvent(event);
}

void MPlotColorLegend::onDataChangedPrivate()
{
	update();
}
