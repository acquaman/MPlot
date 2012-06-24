#include "MPlot/MPlotAxisScale.h"
#include <QDebug>


MPlotAxisScale::MPlotAxisScale(Qt::Orientation orientation, const QSizeF& drawingSize, const MPlotAxisRange& dataRange, qreal axisPaddingPercent, QObject* parent)
	: QObject(parent)
{
	orientation_ = orientation;
	drawingSize_ = drawingSize;
	logScaleEnabled_ = false;

	axisPadding_ = axisPaddingPercent/100.0;

	dataRangeConstraint_ = MPlotAxisRange(MPLOT_NEG_INFINITY, MPLOT_POS_INFINITY);

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

	// constraints
	//////////////////////
	dataRange_ = dataRange_.constrainedTo(dataRangeConstraint_);
	unpaddedDataRange_ = unpaddedDataRange_.constrainedTo(dataRangeConstraint_);

	// safety protection limits...
	/////////////////////////////////

	qreal minRange = std::numeric_limits<qreal>::epsilon()*fabs(dataRange_.min())*8.0;	// epsilon is smallest representable difference between 1 and the next number. minRange will be smaller for numbers with small exponents
	if(dataRange_.length() < minRange) {
		qWarning() << "MPlotAxisScale: Set the data range too small. Min:" << dataRange_.min() << ". Automatically expanding by epsilon:" << minRange;
		dataRange_.setMax(dataRange_.min() + minRange); // this prevents crashing at high zoom scales.
	}
	qreal maxRange = std::numeric_limits<qreal>::max()/1e10;
	if(dataRange_.length() > maxRange) {
		qWarning() << "MPlotAxisScale: Set the data range too large. Min:" << dataRange_.min() << ". Max: " << dataRange_.max() << ".  Automatically capping to" << dataRange_.min() + maxRange;
		dataRange_.setMax(dataRange_.min() + maxRange);
	}
	////////////////////////

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



	qreal min = dataRange_.min();
	qreal max = dataRange_.max();

	// is the data range backward? That's allowed, but we need to deal with it explictly here.
	if(max<min)
		qSwap(min,max);

	if(minimumNumberOfTicks == 1) {
		rv << (dataRange_.min() + dataRange_.max())/2.0;
		return rv;
	}

	bool logScaleOn = (logScaleEnabled_ && min>0 && max>0);
	// log scale on? Then we need a different algorithm
	if(logScaleOn) {

		qreal log5 = log10(5);
		qreal log2 = log10(2);

		// what is the range, in powers of ten?
		qreal logMin = log10(min);
		qreal logMax = log10(max);
		qreal logRange = logMax - logMin;

		qreal maxPowerOfTen = floor(logMax);	// might be out of range if the range is less than 2
		qreal outerMaxPowerOfTen = ceil(logMax);
		qreal minPowerOfTen = ceil(logMin);	// might be out of range if the range is less than 2.
		qreal outerMinPowerOfTen = floor(logMin);

		qreal step = 1;
		bool include5s = false;
		bool include2s = false;

		if((logRange < 2) || (logRange +1 < minimumNumberOfTicks)) {	// Might need to add 2s and 5s to get enough ticks.  Here we set include5s and include2s as required, and leave step at 1.
			int ticksFound = 0;

			// How many powers of 10 ticks do we have?  Depending on how the min and max values line up, we might not have two power of ten values within the range where we can place ticks.
			if(logRange < 2) {
				if(minPowerOfTen <= logMax) {	// got one power of ten.
					ticksFound++;
				}
				if(maxPowerOfTen >= logMin) {	// got the other
					ticksFound++;
				}
			}
			else
				ticksFound += int(floor(logRange));

			// add 0.005, 0.5, 5, 50, 500, etc.?
			if(ticksFound < minimumNumberOfTicks) {
				include5s = true;
				for(int d=int(outerMinPowerOfTen); d<outerMaxPowerOfTen; d++) {
					qreal potential5 = d + log5;
					if(potential5 >= logMin && potential5 <=logMax)
						ticksFound++;
				}
			}
			// add 0.02, 0.2, 2, 20, 200, etc.
			if(ticksFound < minimumNumberOfTicks) {
				include2s = true;
			}
		}

		else if(logRange > minimumNumberOfTicks) {
			step = floor(logRange / minimumNumberOfTicks);
		}


		// finally, insert all the ticks
		for(qreal d=outerMinPowerOfTen; d<outerMaxPowerOfTen; d+=step) {
			if(d<=logMax && d>=logMin)
				rv << pow(10,d);
			if(include2s) {
				qreal potential2 = d + log2;
				if(potential2 <=logMax && potential2>=logMin)
					rv << pow(10, potential2);
			}
			if(include5s) {
				qreal potential5 = d + log5;
				if(potential5 <=logMax && potential5>=logMin)
					rv << pow(10, potential5);
			}
		}
	}	// end of log scale mode

	// linear scale mode:
	else {
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

		while(minTickVal <= max) {	/// \todo For short axis scale ranges... make sure that tickIncVal is > than 0... Otherwise this loop takes forever.
			rv << minTickVal;
			minTickVal += tickIncVal;
		}

	}	// end of linear scale mode algorithm

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

void MPlotAxisScale::setDataRangeConstraint(const MPlotAxisRange &constraintsOnDataRange)
{
	if(constraintsOnDataRange.isValid())
		dataRangeConstraint_ = constraintsOnDataRange;
	else
		dataRangeConstraint_ = MPlotAxisRange(MPLOT_NEG_INFINITY, MPLOT_POS_INFINITY);

	setDataRange(unpaddedDataRange_, false);
}
