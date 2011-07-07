#ifndef MPLOTCOLORMAP_H
#define MPLOTCOLORMAP_H

#include <QPair>
#include <QColor>
#include <QDebug>

#include <cmath>

/// only required for MPlotInterval typedef. \todo: move somewhere more appropriate?
#include "MPlotImageData.h"
#include <QGradientStop>


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

	/// This function assumes that value is between 0-1.
	QColor colorAt(qreal value) const { return QColor::fromRgba(rgbAt(value)); }
	QColor colorAt(qreal value, MPlotInterval range) const { return QColor::fromRgba(rgbAt(value, range)); }
	QColor colorAtIndex(int index) const { return QColor::fromRgba(rgbAtIndex(index)); }

	QRgb rgbAt(qreal value) const { return rgbAt(value, MPlotInterval(0.0, 1.0)); }
	QRgb rgbAt(qreal value, MPlotInterval range) const
	{
		if(recomputeCachedColorsRequired_)
			recomputeCachedColors();

		if(range.first == range.second)	// don't blow up to infinite when the range is nothing.
			return rgbAtIndex(0);

		return rgbAtIndex((int)round(((value-range.first)/(range.second-range.first))*(resolution()-1)));
	}
	QRgb rgbAtIndex(int index) const { if (index < 0 || index >= resolution()) return QRgb(); return colorArray_.at(index); }

	/// Returns the stop points for this gradient.
	/*! If no stop points have been specified, a gradient of black at 0 to white at 1 is used.*/
	QGradientStops stops() const { return colorStops_; }
	/// Replaces the current set of stop points with the given \c stopPoints. The positions of the points must be in the range 0 to 1, and must be sorted with the lowest point first.  As soon as you set the stops manually, if this color map was an optimized standardColorMap, it will cease to be.
	void setStops(const QGradientStops& stopPoints);
	/// Adds a stop the given \c position with the color \c color.
	void addStopAt(qreal position, const QColor& color);

	/// Returns the resolution (number of color steps) in the pre-computed color map.
	int resolution() const { return colorArray_.size(); }
	/// Set the resolution (number of color steps) in the pre-computed color map.  The default is 256.  Higher resolution could produce a smoother image, but will require more memory.  (For comparison, Matlab's default resolution is 64.)
	void setResolution(int newResolution) { colorArray_.resize(newResolution); recomputeCachedColorsRequired_ = true; }


	/// Returns the interpolation mode used to interpolate between color stops.  RGB is fastest, while HSV preserves human-perception-based color relationships.
	BlendMode blendMode() const { return blendMode_; }
	/// Set the interpolation mode used to interpolate between color stops.
	void setBlendMode(BlendMode newBlendMode) { blendMode_ = newBlendMode; recomputeCachedColorsRequired_ = true; }


	/// Helper function to recompute the cached color array when the color stops, resolution, or blend mode are changed.  It will be called automatically as required, but you can also call it prior to calling colorAt() or rgbAt() if you want to optimize the timing of when the cached color map is calculated.
	void recomputeCachedColors() const;


	/// Comparison operator to see if a color map matches \c other. [implies same: resolution, standardColorMap, and if not a standard color map, same colorStops()]
	bool operator==(const MPlotColorMap& other) { return !(*this != other); }
	/// Comparison operator to see if a color map is different than \c other. [implies: either a different resolution, different standardColorMap, or if not a standard color map, different colorStops()]
	bool operator!=(const MPlotColorMap& other);


protected:


	/// Stores the pre-computed colors in the map.
	mutable QVector<QRgb> colorArray_;
	/// Stores the current color stops which define the map.
	QGradientStops colorStops_;
	/// Optimization: stores whether the colorArray_ has been already filled, or if recomputing the colors is required before asking for any rgbAt() or colorAt() values.
	mutable bool recomputeCachedColorsRequired_;

	/// This will be -1 if this is a custom colormap, and a StandardColorMap enum value if it is standard.
	int standardColorMapValue_;

	/// System-wide pre-computed values for default color maps: optimizes the creation of new default color maps by sharing the pre-computed color arrays.
	static QVector<QVector<QRgb>*> precomputedMaps_;

private:
	/// Returns the index for the color array if given a value within a range between 0 and 1.
	int colorIndex(QGradientStop stop) const { if (stop.first < 0) return 0; if (stop.first >= 1) return resolution()-1; return (int)(stop.first*(resolution()-1)); }

	BlendMode blendMode_;

};


#endif // MPLOTCOLORMAP_H
