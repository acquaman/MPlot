#include "MPlotAxisScale.h"
#include <QDebug>
#include <cmath>

MPlotAxisScale::MPlotAxisScale(Qt::Orientation orientation,
							   const QSizeF& drawingSize,
							   const MPlotAxisRange& dataRange,
							   QObject* parent)
	: QObject(parent)
{
	orientation_ = orientation;
	drawingSize_ = drawingSize;
	logScaleEnabled_ = false;

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

	bool dataRangeIsInverted = (newDataRange.min() > newDataRange.max());
	unpaddedDataRange_ = newDataRange.normalized();	// make sure min<=max. We'll do the calculations on a normal ordered scale, and then invert back.

	if(applyPadding) {
		// if log scale in effect:
		if(logScaleEnabled_ && unpaddedDataRange_.min() > 0 && unpaddedDataRange_.max() > 0) {
			qreal logMin = log10(unpaddedDataRange_.min());
			qreal logMax = log10(unpaddedDataRange_.max());
			qreal logPadding = (logMax-logMin)*axisPadding_;

			dataRange_ = MPlotAxisRange( pow(10, logMin-logPadding),
										 pow(10, logMax+logPadding) );
		}
		else {
			qreal padding = unpaddedDataRange_.length()*axisPadding_;
			dataRange_ = MPlotAxisRange(unpaddedDataRange_.min()-padding,
										unpaddedDataRange_.max()+padding);
		}
	}
	else {
		dataRange_ = unpaddedDataRange_;
	}

	// if axis scale should be inverted: invert back.
	if(dataRangeIsInverted) {
		unpaddedDataRange_ = MPlotAxisRange(unpaddedDataRange_.max(), unpaddedDataRange_.min());
		dataRange_ = MPlotAxisRange(dataRange_.max(), dataRange_.min());
	}

	emit dataRangeChanged();
}

void MPlotAxisScale::setPadding(qreal percent) {
	axisPadding_ = percent/100.0;
	setDataRange(unpaddedDataRange_, true);
}



QList<qreal> MPlotAxisScale::calculateTickValues(int minimumNumberOfTicks) const
{
	QList<qreal> rv;

	if(minimumNumberOfTicks == 0)
		return rv;

	if(minimumNumberOfTicks == 1) {
		rv << (dataRange_.min() + dataRange_.max())/2.0;
		return rv;
	}

	qreal min = dataRange_.min();
	qreal max = dataRange_.max();

	// is the data range backward? That's allowed, but we need to deal with it explictly here.
	if(max<min)
		qSwap(min,max);

	// numTicks() is a suggestion for the minimum number of ticks.
	qreal crudeStep = (max - min) / minimumNumberOfTicks;

	qreal step = pow(10, floor(log10(crudeStep)));
	if(5*step < crudeStep)
		step *= 5;
	else if(2*step < crudeStep)
		step *= 2;

	qreal tickIncVal = step;
	qreal minTickVal = ceil(min/step) * step;

	// Hit Zero if possible: (while passing through origin)
	if(min < 0 && max > 0) {
		// the distance between 0 and the nearest tick is... the remainder in division of (0-minTickVal)/tickIncVal.
		qreal offset = remainder(-minTickVal, tickIncVal);
		minTickVal += offset;
	}

	while(minTickVal <= max) {
		rv << minTickVal;
		minTickVal += tickIncVal;
	}

	return rv;
}

void MPlotAxisScale::setAutoScaleEnabled(bool autoScaleEnabled) {
	if(autoScaleEnabled_ == autoScaleEnabled) return;

	if(autoScaleEnabled)
		autoScaleScheduled_ = true;

	emit autoScaleEnabledChanged(autoScaleEnabled_ = autoScaleEnabled);
}

void MPlotAxisScale::setLogScaleEnabled(bool logScaleEnabled)
{
	if(logScaleEnabled_ == logScaleEnabled)
		return;

	logScaleEnabled_ = logScaleEnabled;
	setDataRange(unpaddedDataRange_, true);	// need to re-apply padding in log-scale mode... Otherwise the linear padding on an otherwise log-valid axis range will force it into the negatives. (ex: a range from (1,100000) will be padded with, say, 5% of 100000 on the bottom and top, resulting in a negative value on the bottom, which would disable log scaling.

	// setDataRange will also emit dataRangeAboutToChange() and dataRangeChanged(), which we would need to do anyways.
}
