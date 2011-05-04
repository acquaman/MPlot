#include "MPlotAxisScale.h"
#include <QDebug>

MPlotAxisScale::MPlotAxisScale(Qt::Orientation orientation,
							   const QSizeF& drawingSize,
							   const MPlotAxisRange& dataRange,
							   QObject* parent)
	: QObject(parent)
{
	orientation_ = orientation;
	drawingSize_ = drawingSize;

	axisPadding_ = 0.05;

	setDataRange(dataRange, true);

	autoScaleEnabled_ = false;
	autoScaleScheduled_ = false;
}

void MPlotAxisScale::setOrientation(Qt::Orientation orientation)
{
	if(orientation_ == orientation)
		return;

	emit drawingSizeAboutToChange();	// effective drawing size will change, because we're now using the opposite span (vertical instead of horizontal, or vice versa) for the scaling calculation.
	orientation_ = orientation;
	emit drawingSizeChanged();
}

void MPlotAxisScale::setDrawingSize(const QSizeF &newSize)
{
	emit drawingSizeAboutToChange();
	drawingSize_ = newSize;
	emit drawingSizeChanged();
}

void MPlotAxisScale::setDataRange(const MPlotAxisRange &newDataRange, bool applyPadding)
{
	if(!newDataRange.isValid()) {
		qWarning() << "MPlotAxisScale: Attempting to provide a null data range for the axis scale range.";
		return;
	}

	emit dataRangeAboutToChange();

	unpaddedDataRange_ = newDataRange;
	if(applyPadding) {
		qreal padding = unpaddedDataRange_.length()*axisPadding_;
		dataRange_ = MPlotAxisRange(unpaddedDataRange_.min()-padding, unpaddedDataRange_.max()+padding);
	}
	else
		dataRange_ = unpaddedDataRange_;

	autoScaleEnabled_ = false;	// axis range was manually set by user; this disables auto-scaling

	qDebug() << "MPlotAxisScale:" << orientation_ << "Data range changed:" << dataRange_.min() << dataRange_.max();
	emit dataRangeChanged();
}
