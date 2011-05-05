#ifndef MPLOTAXISSCALE_H
#define MPLOTAXISSCALE_H

#include <QPair>
#include <QSizeF>
#include <QRectF>
#include <QObject>

class MPlotAxisRange {
public:
	/// Constructs a null axis range
	MPlotAxisRange() { min_ = 0.0; max_ = 0.0; valid_ = false; }
	/// Constructs an axis range from \c min to \c max
	MPlotAxisRange(qreal min, qreal max) { setRange(min, max); }
	/// Constructs an axis range from the vertical or horizontal extent of a QRectF
	MPlotAxisRange(const QRectF& rect, Qt::Orientation orientation) {
		if(!rect.isValid())
			*this = MPlotAxisRange();

		else if(orientation == Qt::Vertical) {
			min_ = rect.top();
			max_ = rect.bottom();
			valid_ = true;
		}
		else {
			min_ = rect.left();
			max_ = rect.right();
			valid_ = true;
		}
	}

	// using default copy constructor and assignment operator

	/// A null MPlotAxisRange has never had its range set.  A valid MPlotAxisRange comes from setting the range, either in the constructor or in setRange().
	bool isValid() const { return valid_; }

	/// Returns a new MPlotAxisRange that is guaranteed to have min() < max(), by swapping max and min if required.
	MPlotAxisRange normalized() const {
		MPlotAxisRange rv(min_, max_);
		if(min_>max_)
			qSwap(rv.min_, rv.max_);
		return rv;
	}

	/// Ensures that this MPlotAxisRange is guaranteed to have min() < max(), by swapping max and min if required.
	void normalize() {
		*this = normalized();
	}

	qreal min() const { return min_; }
	qreal max() const { return max_; }
	qreal length() const { return max_ - min_; }

	void setMin(qreal min) { min_ = min; }
	void setMax(qreal max) { max_ = max; }

	void setRange(qreal min, qreal max) { min_ = min; max_ = max; valid_ = true; }

	MPlotAxisRange& operator|=(const MPlotAxisRange& other) {
		if(!other.isValid())
			return *this;	// if other is not valid: don't change anything.

		if(!isValid()) {
			return (*this = other);	// if we weren't valid, but other is: take on values of other and become valid.
		}

		// normal situation, both valid. expand to min, max of both
		if(other.min_ < min_)
			min_ = other.min_;
		if(other.max_ > max_)
			max_ = other.max_;

		return *this;
	}

protected:
	qreal min_, max_;
	bool valid_;
};

class MPlotAxisScale : public QObject
{
	Q_OBJECT

public:
	MPlotAxisScale(Qt::Orientation orientation,
				   const QSizeF& drawingSize = QSizeF(100, 100),
				   const MPlotAxisRange& dataRange = MPlotAxisRange(0, 10),
				   QObject* parent = 0);

	qreal mapDataToDrawing(qreal dataValue) const {
		if(orientation_ == Qt::Vertical)
			return drawingSize_.height() * (1 - (dataValue-dataRange_.min())/(dataRange_.max()-dataRange_.min()));
		else
			return drawingSize_.width() * (dataValue-dataRange_.min())/(dataRange_.max()-dataRange_.min());
	}
	MPlotAxisRange mapDataToDrawing(const MPlotAxisRange& dataRange) const {
		return MPlotAxisRange(
					mapDataToDrawing(dataRange.min()),
					mapDataToDrawing(dataRange.max()));
	}
	qreal mapDrawingToData(qreal drawingValue) const {
		if(orientation_ == Qt::Vertical)
			return dataRange_.min() + (1 - drawingValue/drawingSize_.height())*(dataRange_.max() - dataRange_.min());
		else
			return dataRange_.min() + drawingValue/drawingSize_.width()*(dataRange_.max()-dataRange_.min());
	}
	MPlotAxisRange mapDrawingToData(const MPlotAxisRange& drawingRange) const {
		return MPlotAxisRange(
					mapDrawingToData(drawingRange.min()),
					mapDrawingToData(drawingRange.max()));
	}


	QSizeF drawingSize() const { return drawingSize_; }
	qreal drawingLength() const { return orientation_ == Qt::Vertical ? drawingSize_.height() : drawingSize_.width(); }

	MPlotAxisRange dataRange() const { return dataRange_; }
	qreal min() const { return dataRange_.min(); }
	qreal max() const { return dataRange_.max(); }

	Qt::Orientation orientation() const { return orientation_; }


	/// Indicates that this axis scale should be autoscaled.
	bool autoScaleEnabled() const { return autoScaleEnabled_; }
	/// Enable or disable autoscaling on this axis.  When autoscaling is enabled, plots will adjust the range of this axis to match the maximum range of all of the items which use it as their axis scale.
	void setAutoScaleEnabled(bool autoScaleEnabled = true) {
		if(autoScaleEnabled_ == autoScaleEnabled) return;

		if(autoScaleEnabled)
			autoScaleScheduled_ = true;

		emit autoScaleEnabledChanged(autoScaleEnabled_ = autoScaleEnabled);
	}

	/// Used by MPlot to flag that a re-autoScale is pending for this axis scale
	bool autoScaleScheduled() const { return autoScaleScheduled_; }
	/// Used by MPlot to flag that a re-autoScale is pending for this axis scale
	void setAutoScaleScheduled(bool autoScaleScheduled = true) { autoScaleScheduled_ = autoScaleScheduled; }

	qreal padding() const { return axisPadding_*100.0; }


	/// Returns a list of recommended tick values (in data coordinates) for this axis scale, based on a recommended \c minimumNumberOfTicks.  The tick values will be chosen to be within the data range, and will be "nice numbers".  They will be spaced according to what makes sense for the axis scale (ex: regularly for a linear axis, in powers of 10 for a logarithmic axis, etc.)
/*! IntelliScale: Calculate "nice" values for starting tick and tick increment.

- Uses the current values of the dataRange_ min() and max().
- Desired outcome: labels are nice values like "0.2 0.4 0.6..." or "0.024 0.026 0.028" instead of irrational numbers.
- Additionally, if the axis range passes through 0, it would be nice to have a tick at 0.

Implementation: This algorithm is based on one from "C++ Gui Programming with Qt" (below), but we move the starting tick position instead of the max and min values.

<i>
To obtain nice numbers along the axis, we must select the step with care. For example, a step value of 3.8 would lead to an axis with multiples of 3.8, which is difficult for people to relate to. For axes labeled in decimal notation, “nice” step values are numbers of the form 10n, 2·10n, or 5·10n.

We start by computing the “gross step”, a kind of maximum for the step value. Then we find the corresponding number of the form 10n that is smaller than or equal to the gross step. We do this by taking the decimal logarithm of the gross step, rounding that value down to a whole number, then raising 10 to the power of this rounded number. For example, if the gross step is 236, we compute log 236 = 2.37291...; then we round it down to 2 and obtain 102 = 100 as the candidate step value of the form 10n.

Once we have the first candidate step value, we can use it to calculate the other two candidates: 2·10n and 5·10n. For the example above, the two other candidates are 200 and 500. The 500 candidate is larger than the gross step, so we can’t use it. But 200 is smaller than 236, so we use 200 for the step size in this example.
</i>
\code
void PlotSettings::adjustAxis(qreal &min, qreal &max,
int &numTicks)
{
 const int MinTicks = 4;
 qreal grossStep = (max - min) / MinTicks;
 qreal step = pow(10.0, floor(log10(grossStep)));
 if (5 * step < grossStep) {
  step *= 5;
 } else if (2 * step < grossStep) {
  step *= 2;
 }
 numTicks = int(ceil(max / step) - floor(min / step));
 if (numTicks < MinTicks)
  numTicks = MinTicks;
 min = floor(min / step) * step;
 max = ceil(max / step) * step;
}
\endcode
*/
	QList<qreal> calculateTickValues(int minimumNumberOfTicks) const;


public slots:
	void setOrientation(Qt::Orientation orientation);
	void setDrawingSize(const QSizeF& newSize);
	void setDataRange(const MPlotAxisRange& newDataRange, bool applyPadding = true);
	void setPadding(qreal percent) {
		axisPadding_ = percent/100.0;
		setDataRange(unpaddedDataRange_, true);
	}

signals:
	/// Emitted before the drawing size (extent of the axis in screen/drawing coordinates) changes.  A drawing size change could imply a change in the data range as well, but because this event is more general, we don't emit both signals... Just this one.
	void drawingSizeAboutToChange();
	/// Emitted after the drawing size (extent of the axis in screen/drawing coordinates) changes.
	void drawingSizeChanged();
	/// Emitted before the data range (min and max data values displayed by the axis) changes.  This signal is emitted whenever the data range will change but the drawing size (physical extent of the axis in screen coordinates) will remain the same.
	void dataRangeAboutToChange();
	/// Emitted after the data range change is complete.
	void dataRangeChanged();
	/// Emitted when autoscaling is turned on or off.
	void autoScaleEnabledChanged(bool autoScaleEnabled);


protected:
	QSizeF drawingSize_;
	MPlotAxisRange dataRange_, unpaddedDataRange_;
	Qt::Orientation orientation_;
	qreal axisPadding_;	///< Amount to pad onto both the min and max of the data range provided; expressed as fraction of the total data range length.  This improves appearance by keeping data points from going right to the edges of the plots. Default is 5%.

	/// Used by MPlot to store whether this axis scale should be autoscaled.
	bool autoScaleEnabled_;
	/// Used by MPlot to flag that a re-autoScale is pending for this axis scale
	bool autoScaleScheduled_;

};

#endif // MPLOTAXISSCALE_H