#ifndef MPLOTCOLORMAP_H
#define MPLOTCOLORMAP_H

#include <QPair>
#include <QColor>
#include <QDebug>

#include <cmath>

/// only required for MPlotInterval typedef. \todo: move somewhere more appropriate?
#include "MPlotImageData.h"
#include <QGradientStop>

#include <QSharedData>

/// This private class is used to implement implicit sharing for MPlotColorMap
class MPlotColorMapData : public QSharedData
{
  public:
        /// Default constructor. Can set the color resolution if desired.
	MPlotColorMapData(int resolution = 256);
        /// Constructor that builds the color map between \param color1 and \param color2.  Can set the color resolution if desired.
	MPlotColorMapData(const QColor& color1, const QColor& color2, int resolution = 256);
        /// Constructor that builds a color map with the given QGradientStops \param colorStops.  Must set the resolution.
	MPlotColorMapData(const QGradientStops& colorStops, int resolution);
        /// Constructor using one of the standard color maps.  Must set the resolution.
	MPlotColorMapData(int standardColorMap, int resolution);
        /// Constructor using another color map.
	MPlotColorMapData(const MPlotColorMapData &other);

	~MPlotColorMapData() {}


	/// Stores the pre-computed colors in the map.
	mutable QVector<QRgb> colorArray_;
	/// Stores the current color stops which define the map.
	QGradientStops colorStops_;
	/// Optimization: stores whether the colorArray_ has been already filled, or if recomputing the colors is required before asking for any rgbAt() or colorAt() values.
	mutable bool recomputeCachedColorsRequired_;

	/// This will be -1 if this is a custom colormap, and a StandardColorMap enum value if it is standard.
	int standardColorMapValue_;

	/// Whether to blend using RGB or HSV interpolation
	int blendMode_;

	/// Brightness, contrast, and gamma
	qreal brightness_, contrast_, gamma_;
	/// Optimization flag to indicate if brightness, contrast, and gamma corrections need to be applied
	qreal mustApplyBCG_;


	/// Helper function to recompute the cached color array when the color stops, resolution, or blend mode are changed.  It will be called automatically as required, but you can also call it prior to calling colorAt() or rgbAt() if you want to optimize the timing of when the cached color map is calculated.
	void recomputeCachedColors() const;

        /// Returns the resolution.
	int resolution() const { return colorArray_.size(); }
        /// Operator to determine if one color map is not the same as another.
	bool operator!=(const MPlotColorMapData& other) const;

	/// Returns the index for the color array if given a value within a range between 0 and 1.
	int colorIndex(QGradientStop stop) const {
		if (stop.first < 0) return 0;
		if (stop.first >= 1) return resolution()-1;
		return (int)(stop.first*(resolution()-1));
	}


	/// System-wide pre-computed values for default color maps: optimizes the creation of new default color maps by sharing the pre-computed color arrays.
	static QVector<QVector<QRgb>*> precomputedMaps_;

};



/// This class converts numerical values into colors, for use in 2D image maps.  (Its output is very similar to Matlab's \c colormap function.)  Like QGradient, it can produce an infinite variety of color mappings, by using a set of color stops at arbitrary values, and interpolating between them.
/*!
<b>Color Stops</b>

The color map is defined by a set of 'color stops' on a scale from 0.0 to 1.0.  In between each color stop, the color is determined by interpolating between the nearest stops on either side.  Before the first stop and after the last stop, the color stays constant.

\image http://www.mathworks.com/help/techdoc/ref/graphics_c30.gif

You can define the color scale by providing all the stops at once, with setColorStops().  You can also add a new stop into the existing scale with addStopAt().  For more information, see the documentation on QGradient, which uses the same concept.

The interpolation between colors is done using either RGB or HSV interpolation. You can change the interpolation method with setBlendMode().

<b>Performance and indexing</b>

For performance, colors are pre-computed and cached at a specific resolution(). (The default setting computes 256 color steps, but this can be changed using setResolution(), if you want finer color steps.

<b>Retrieving a color for a given value</b>

You can retrieve the color corresponding to a given value in three ways:

- colorAt(qreal value) returns the color for a \c value in the range from 0.0 to 1.0.
- colorAt(qreal value, MPlotInterval range) returns the color for a \c value within a specified range, from \c range.first to range.second

(For both of these functions, if the value is outside of the range, colorAt() returns the minimum or maximum color.)

- colorAtIndex(int index) returns the color corresponding to a bin in the cached color array. \c index must be from 0 to resolution()-1.  This function has the highest performance.

Equivalent versions of these functions are provided that return a QRgb instead of a  QColor, for maximum performance.

<b>Copy-on-write</b>
This class is intended to be passed value, in the same way that QColor and QGradient are passed by value.  It exploits the implicit sharing ("copy-on-write") strategy provided by all of Qt's container classes so that it can be copied very quickly.
*/


class MPlotColorMap {
public:

	/// Describes the interpolation mode used to interpolate between color stops.  RGB is fastest, while HSV preserves human-perception-based color relationships.
	enum BlendMode { RGB, HSV };
	/// Predefined (standard) color maps. These colormaps are pre-computed in memory, and can be copied very quickly.
	enum StandardColorMap { Autumn, Bone, Cool, Copper, Gray, Hot, Hsv, Jet, Pink, Spring, Summer, White, Winter };

	/// Constructs a default color map (Corresponding to MPlotColorMap::Jet)
	MPlotColorMap(int resolution = 256);
	/// Constructs a linear color map between \c color1 and \c color2.
	MPlotColorMap(const QColor& color1, const QColor& color2, int resolution = 256);
	/// Constructs a color map based on a set of initial \c colorStops
	MPlotColorMap(const QGradientStops& colorStops, int resolution = 256);
	/// Convenience constructor based on the pre-built color maps that are used in other applications, such as Matlab
	MPlotColorMap(StandardColorMap colorMap, int resolution = 256);

	/// See rgbAt(value).
	QColor colorAt(qreal value) const { return QColor::fromRgba(rgbAt(value)); }
	/// See rgbAt(value, range)
	QColor colorAt(qreal value, MPlotInterval range) const { return QColor::fromRgba(rgbAt(value, range)); }
	/// See rgbAtIndex()
	QColor colorAtIndex(int index) const { return QColor::fromRgba(rgbAtIndex(index)); }

	/// Returns a color for a \c value expressed with a given \c range.  [If value is outside this range, will return minimum or maximum color]
	QRgb rgbAt(qreal value, MPlotInterval range) const {
		if(range.first == range.second)	// don't blow up to infinite when the range is nothing.
			return rgbAtIndex(0);

		return rgbAt( (value-range.first)/(range.second-range.first) );	// map values in range to (0,1) and use rgbAt(value).
	}

	/// Returns a color for a \c value between (0,1).  [If value is outside this range, will return minimum or maximum color]
	QRgb rgbAt(qreal value) const
	{
		if(d->mustApplyBCG_) {
			if(d->gamma_ == 1.0)
				value = d->contrast_ * (value + d->brightness_);
			else
				value = d->contrast_*( pow(value,d->gamma_) + d->brightness_ );
		}
		return rgbAtIndex((int)round(value*(d->colorArray_.size()-1)));
	}

	/// Returns a color for an index between (0, resolution()-1) in the color map table.  [If index is outside this range, will return minimum or maximum color]
	QRgb rgbAtIndex(int index) const {
		if(d->recomputeCachedColorsRequired_)
			d->recomputeCachedColors();

		if (index < 0)
			return d->colorArray_.first();
		if(index >= d->colorArray_.size())
			return d->colorArray_.last();
		return d->colorArray_.at(index);
	}

	/// Returns the stop points for this gradient.
	/*! If no stop points have been specified, a gradient of black at 0 to white at 1 is used.*/
	QGradientStops stops() const { return d->colorStops_; }
	/// Replaces the current set of stop points with the given \c stopPoints. The positions of the points must be in the range 0 to 1, and must be sorted with the lowest point first.  As soon as you set the stops manually, if this color map was an optimized standardColorMap, it will cease to be.
	void setStops(const QGradientStops& stopPoints);
	/// Adds a stop the given \c position with the color \c color.
	void addStopAt(qreal position, const QColor& color);

	/// Returns the resolution (number of color steps) in the pre-computed color map.
	int resolution() const { return d->colorArray_.size(); }
	/// Set the resolution (number of color steps) in the pre-computed color map.  The default is 256.  Higher resolution could produce a smoother image, but will require more memory.  (For comparison, Matlab's default resolution is 64.)
	void setResolution(int newResolution) {
		if(newResolution == resolution())
			return;

		d.detach();
		d->colorArray_.resize(newResolution);
		d->recomputeCachedColorsRequired_ = true;
	}


	/// Returns the interpolation mode used to interpolate between color stops.  RGB is fastest, while HSV preserves human-perception-based color relationships.
	BlendMode blendMode() const { return (BlendMode)d->blendMode_; }
	/// Set the interpolation mode used to interpolate between color stops.
	void setBlendMode(BlendMode newBlendMode) {
		if(d->blendMode_ == newBlendMode)
			return;

		d.detach();
		d->blendMode_ = newBlendMode;
		d->recomputeCachedColorsRequired_ = true;
	}



	/// Comparison operator to see if a color map matches \c other. [implies same: resolution, standardColorMap, and if not a standard color map, same colorStops()]
	bool operator==(const MPlotColorMap& other) const { return !(*this != other); }
	/// Comparison operator to see if a color map is different than \c other. [implies: either a different resolution, different standardColorMap, or if not a standard color map, different colorStops()]
	bool operator!=(const MPlotColorMap& other) const;

	/// If this map is one of the standard color maps, returns the StandardColorMap value of that map. Otherwise returns -1.
	int standardColorMapValue() const { return d->standardColorMapValue_; }


	/// Set a brightness correction for the color map. \brightness is a floating point number between 0 and 1.  Positive values make the image "lighter"; negative values make it "darker".  (A value of 1 for brightness would result in the maximum color being returned for any input value.)
	void setBrightness(qreal brightness);
	/// Set a brightness correction for the color map. \contrast is a floating point number greater than 0 which provides a multiplicative scaling factor; values larger than 1 increase the contrast, values less than 1 decrease the contrast.
	void setContrast(qreal contrast);
	/// Set a gamma correction for the color map.  \c gamma is a floating point number greater than 0 which provides an exponential scaling factor; values larger than 1 emphasize detail near the maximum values in the image; values less than 1 emphasize detail near the minimum values in the image.
	void setGamma(qreal gamma);

	/// Return the current brightness correction
	qreal brightness() const { return d->brightness_; }
	/// Return the current contrast correction
	qreal contrast() const { return d->contrast_; }
	/// Return the current gamma correction
	qreal gamma() const { return d->gamma_; }


protected:

private:

	/// To implement implicit sharing:
	QExplicitlySharedDataPointer<MPlotColorMapData> d;


};

#include <QMetaType>

Q_DECLARE_METATYPE(MPlotColorMap)


#endif // MPLOTCOLORMAP_H
