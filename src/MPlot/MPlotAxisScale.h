#ifndef MPLOTAXISSCALE_H
#define MPLOTAXISSCALE_H

#include "MPlot/MPlot_global.h"

#include <QPair>
#include <QSizeF>
#include <QRectF>
#include <QObject>

#include <cmath>
#include <cfloat>

#include <limits>

#define MPLOT_POS_INFINITY std::numeric_limits<qreal>::infinity()
#define MPLOT_NEG_INFINITY -std::numeric_limits<qreal>::infinity()

class MPLOTSHARED_EXPORT MPlotAxisRange {
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

	/// Return the axis range formed by constraining this range within \c constraint.  The returned range will be shortened if necessary so that both min() and max() are inside of constraint.min() and constraint.max().
	MPlotAxisRange constrainedTo(const MPlotAxisRange& constraint) {
		qreal effectiveMin = qMin(constraint.min(), constraint.max());
		qreal effectiveMax = qMax(constraint.min(), constraint.max());

		return MPlotAxisRange(
					qBound(effectiveMin, min_, effectiveMax),
					qBound(effectiveMin, max_, effectiveMax));
	}

	/// Ensures that this MPlotAxisRange is within \c constraint.  Both min() and max() will be bound inside of constraint.min() and constraint.max().
	void constrainTo(const MPlotAxisRange& constraint) {
		*this = constrainedTo(constraint);
	}

	/// Returns the minimum value for this axis range.
	qreal min() const { return min_; }
	/// Returns the maximum value for this axis range.
	qreal max() const { return max_; }
	/// Returns the separation between the minimum and maximum in this axis range.
	qreal length() const { return max_ - min_; }

	/// Sets the minimum value to \param min for this axis range.
	void setMin(qreal min) { min_ = min; }
	/// Sets the maximum value to \param max for this axis range.
	void setMax(qreal max) { max_ = max; }

	/// Sets the range (min and max) for this axis range to \param min and \param max.
	void setRange(qreal min, qreal max) { min_ = min; max_ = max; valid_ = true; }

	/// Operator for MPlotAxisRanges.  Takes the lowest minimum and largest maximum from the current axis range and \param other.
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
	/// Holds the minimum and maximum value for this axis range.
	qreal min_, max_;
	/// Holds whether or not this axis range is valid or not.
	bool valid_;
};

/// This class handles all the size aspects for a particular axis.  It manages the range of the axis, how big the axis should be, and some of the other specifics for the axis.  It is kind of like the model for MPlotAxis.  It holds all the relevent information and MPlotAxis paints the axis based on that information.
class MPLOTSHARED_EXPORT MPlotAxisScale : public QObject
{
	Q_OBJECT

public:
	/// Constructor.  Sets some default values to the \param dataRange, \param drawingSize, and \param axisPaddingPercent.  However, it must be specified whether or not the axis scale is veritcal or horizontal because that effects the calculation of some of the properties.
	MPlotAxisScale(Qt::Orientation orientation,
				   const QSizeF& drawingSize = QSizeF(100, 100),
				   const MPlotAxisRange& dataRange = MPlotAxisRange(0, 10),
				   qreal axisPaddingPercent = 5,
				   QObject* parent = 0);

	/// Returns the data value in scene coordinates (ie: is properly scaled within the confines of the minimum and maximum range that is displayed).
	qreal mapDataToDrawing(qreal dataValue) const {
		qreal min = dataRange_.min();
		qreal max = dataRange_.max();

		if(logScaleEnabled_ && min>0.0 && max>0.0) {
			if(dataValue <= 0.0)// When log scaling is active, ensure we bound datavalue within (min,max), since it can't be 0 or negative.  Note that min,max might not be in the order you think they are...: min might be > max.
				dataValue = log10(qBound(qMin(min,max), dataValue, qMax(min,max)));
			else
				dataValue = log10(dataValue);
			min = log10(min);
			max = log10(max);
		}

		if(orientation_ == Qt::Vertical)
			return drawingSize_.height() * (1 - (dataValue-min)/(max-min));
		else
			return drawingSize_.width() * (dataValue-min)/(max-min);
	}

	/// Returns the MPlotAxisRange of the axis scale but within the confines of the scene size.
	MPlotAxisRange mapDataToDrawing(const MPlotAxisRange& dataRange) const {
		return MPlotAxisRange(
					mapDataToDrawing(dataRange.min()),
					mapDataToDrawing(dataRange.max()));
	}

	/// The inverse operation of mapDataToDrawing.  It takes a value from scene coordinates and maps it to data coordinates.
	qreal mapDrawingToData(qreal drawingValue) const {
		qreal min = dataRange_.min();
		qreal max = dataRange_.max();

		bool logScaleOn = (logScaleEnabled_ && min>0.0 && max>0.0);
		if(logScaleOn) {
			min = log10(min);
			max = log10(max);
		}

		qreal rv;

		if(orientation_ == Qt::Vertical)
			rv = min + (1 - drawingValue/drawingSize_.height())*(max - min);
		else
			rv = min + drawingValue/drawingSize_.width()*(max-min);

		return logScaleOn ? pow(10, rv) : rv;
	}

	/// The inverse operation of the mapDataToDrawing.  It takes the MPlotAxisRange in scene coordinates and maps it back to data coordinates.
	MPlotAxisRange mapDrawingToData(const MPlotAxisRange& drawingRange) const {
		return MPlotAxisRange(
					mapDrawingToData(drawingRange.min()),
					mapDrawingToData(drawingRange.max()));
	}

	/// Returns the drawing size for this axis scale.  Defines how big can be in scene coordinates.
	QSizeF drawingSize() const { return drawingSize_; }
	/// Returns the size of the axis scale based on its orientation.
	qreal drawingLength() const { return orientation_ == Qt::Vertical ? drawingSize_.height() : drawingSize_.width(); }

	/// Returns the current data range used for this axis scale.
	MPlotAxisRange dataRange() const { return dataRange_; }
	/// Returns the minimum value in the data range for this axis scale.
	qreal min() const { return dataRange_.min(); }
	/// Returns the maximum value in the data range for this axis scale.
	qreal max() const { return dataRange_.max(); }

	/// Returns the orientation of the axis scale.
	Qt::Orientation orientation() const { return orientation_; }

	/// Enable or disable logarithmic scaling on this axis.  Note that log scaling is only applied when the data range's min() and max() are both > 0.
	void setLogScaleEnabled(bool logScaleEnabled = true);
	/// Indicates that logarithmic scaling should be applied on this axis.  Note that log scaling is only applied when the data range's min() and max() are both > 0, regardless of the value this returns.
	bool logScaleEnabled() const { return logScaleEnabled_; }

	/// Indicates that logarithmic scaling is enabled AND currently active. This means that logScaleEnabled() is true, and both min() and max() are > 0.
	bool logScaleInEffect() const { return logScaleEnabled_ && dataRange_.min() > 0 && dataRange_.max() > 0; }

	/// Indicates that this axis scale should be autoscaled.
	bool autoScaleEnabled() const { return autoScaleEnabled_; }
	/// Enable or disable autoscaling on this axis.  When autoscaling is enabled, plots will adjust the range of this axis to match the maximum range of all of the items which use it as their axis scale.
	void setAutoScaleEnabled(bool autoScaleEnabled = true);

	/// Used internally by MPlot to flag that a re-autoScale is pending for this axis scale
	bool autoScaleScheduled() const { return autoScaleScheduled_; }
	/// Used internally by MPlot to flag that a re-autoScale is pending for this axis scale
	void setAutoScaleScheduled(bool autoScaleScheduled = true) { autoScaleScheduled_ = autoScaleScheduled; }

	/// Returns the padding (as a percent) used for this axis scale.
	qreal padding() const { return axisPadding_*100.0; }
	/// Returns the data range constraint for the axis scale.  This defines the absolute minimum and maximum (in data coordinates) that the axis can show.
	MPlotAxisRange dataRangeConstraint() const { return dataRangeConstraint_; }


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
	/// Sets the orientation for the axis scale to \param orientation.
	void setOrientation(Qt::Orientation orientation);
	/// Sets the drawing size for the axis scale to \param newSize.
	void setDrawingSize(const QSizeF& newSize);
	/// Sets the data range for the axis scale to \param newDataRange.  It also automatically applies padding to the axis based on the current value of padding().
	void setDataRange(const MPlotAxisRange& newDataRange, bool applyPadding = true);
	/// Same as setDataRange but also disables autoscaling on the axis scale.
	void setDataRangeAndDisableAutoScaling(const MPlotAxisRange& newDataRange, bool applyPadding = true) {
		setDataRange(newDataRange,applyPadding);
		emit autoScaleEnabledChanged((autoScaleEnabled_ = false));
	}

	/// Sets the padding for the axis scale to \param percent.
	void setPadding(qreal percent);

	/// Applied as a constraint on setDataRange().  No matter what data range users attempt to set, it will not extend outside of this range.  To clear the constraint, use a null (invalid) MPlotAxisRange().  (You can use this, for example, to constrain a logScaleEnabled() axis from (1, 1.0/0.0), therefore ensuring that it isn't auto-scaled to include 0 even if the data includes 0.
	void setDataRangeConstraint(const MPlotAxisRange& constraintsOnDataRange);

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
	/// Holds the drawing size.  Represents the axis scale in scene coordinates.
	QSizeF drawingSize_;
	/// The MPlotAxisRanges.  Holds the range for data range and the unpadded data range (which obviously are different).
	MPlotAxisRange dataRange_, unpaddedDataRange_;
	/// Holds the orientation for the axis scale.
	Qt::Orientation orientation_;
	///< Amount to pad onto both the min and max of the data range provided; expressed as fraction of the total data range length.  This improves appearance by keeping data points from going right to the edges of the plots. Default is 5%.
	qreal axisPadding_;

	/// Used by MPlot to store whether this axis scale should be autoscaled.
	bool autoScaleEnabled_;
	/// Used by MPlot to flag that a re-autoScale is pending for this axis scale
	bool autoScaleScheduled_;

	/// True if logarithmic scaling should be applied on this axis.
	bool logScaleEnabled_;

	/// Applied as a constraint on setDataRange().  No matter what data range users attempt to set, it will not extend outside of this range.  (You can use this, for example, to constrain a logScaleEnabled() axis from (1, 1.0/0.0), therefore ensuring that it isn't auto-scaled to include 0 even if the data includes 0.
	MPlotAxisRange dataRangeConstraint_;
};

#endif // MPLOTAXISSCALE_H
