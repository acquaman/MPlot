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

- colorAt(double value) returns the color for a \c value in the range from 0.0 to 1.0.
- colorAt(double value, MPlotInterval range) returns the color for a \c value within a specified range, from \c range.first to range.second

(For both of these functions, if the value is outside of the range, colorAt() returns the minimum or maximum color.)

- colorAtIndex(int index) returns the color corresponding to a bin in the cached color array. \c index must be from 0 to resolution()-1.  This function has the highest performance.

Equivalent versions of these functions are provided that return a QRgb instead of a  QColor, for maximum performance.

<b>Copy-on-write</b>
This class is intended to be passed value, in the same way that QColor and QGradient are passed by value.  It exploits the implicit sharing ("copy-on-write") strategy provided by all of Qt's container classes so that it can be copied very quickly.
*/


class MPlotColorMap {
public:

	/// Describes the interpolation mode used to interpolate between color stops.  RGB is fastest, while HSV preserves human-perception-based color relationships.
	enum BlendMode { RGB, HSB };

	/// Constructs a default color map (Corresponding to MPlotColorMap::Jet)
	MPlotColorMap(int resolution = 256);
	/// Constructs a linear color map between \c color1 and \c color2.
	MPlotColorMap(const QColor& color1, const QColor& color2, int resolution = 256);
	/// Constructs a color map based on a set of initial \c colorStops
	MPlotColorMap(const QGradientStops& colorStops, int resolution = 256);


	QColor colorAt(double value) const { return QColor(rgbAt(value)); }
	QColor colorAt(double value, MPlotInterval range) const { return QColor(rgbAt(value, range)); }
	QColor colorAtIndex(int index) const { return QColor(rgbAtIndex(index)); }

	QRgb rgbAt(double value) const;
	QRgb rgbAt(double value, MPlotInterval range) const;
	QRgb rgbAtIndex(int index) const;

	/// Returns the stop points for this gradient.
	/*! If no stop points have been specified, a gradient of black at 0 to white at 1 is used.*/
	QGradientStops stops() const;
	/// Replaces the current set of stop points with the given \c stopPoints. The positions of the points must be in the range 0 to 1, and must be sorted with the lowest point first.
	void setStops(const QGradientStops& stopPoints);

	/// Returns the resolution (number of color steps) in the pre-computed color map.
	int resolution() const;
	/// Set the resolution (number of color steps) in the pre-computed color map.  The default is 256.  Higher resolution could produce a smoother image, but will require more memory.  (For comparison, Matlab's default resolution is 64.)
	void setResolution(int newResolution);


	/// Returns the interpolation mode used to interpolate between color stops.  RGB is fastest, while HSV preserves human-perception-based color relationships.
	BlendMode blendMode() const;
	/// Set the interpolation mode used to interpolate between color stops.
	void setBlendMode(BlendMode newBlendMode);



	/// Predefined (standard) color maps. These colormaps are pre-computed in memory, and can be copied very quickly.
	static MPlotColorMap& Autumn;
	static MPlotColorMap& Bone;
	static MPlotColorMap& ColorCube;
	static MPlotColorMap& Cool;
	static MPlotColorMap& Copper;
	static MPlotColorMap& Flag;
	static MPlotColorMap& Gray;
	static MPlotColorMap& Hot;
	static MPlotColorMap& Hsv;
	static MPlotColorMap& Jet;
	static MPlotColorMap& Pink;
	static MPlotColorMap& Prism;
	static MPlotColorMap& Spring;
	static MPlotColorMap& Summer;
	static MPlotColorMap& White;
	static MPlotColorMap& Winter;

protected:
	/// Helper function to recompute the cached color array when the color stops, resolution, or blend mode are changed.
	void recomputeCachedColors();

	/// Stores the pre-computed colors in the map.
	QVector<QRgb> colorArray_;
	/// Stores the current color stops which define the map.
	QGradientStops colorStops_;

};

/*
/// This class defines the interface for color maps, which are used by image plots to turn a z-value into a color.
class MPlotAbstractColorMap {
public:
	///  return a QRgb (unsigned int) representing the color for a given \c value within a \c range. (Faster than returning a full QColor)
	virtual QRgb rgb(double value, MPlotInterval range = MPlotInterval(0.0, 1.0) ) const = 0;
	/// return the color representing a given \c value within a \c range.
	virtual QColor color(double value, MPlotInterval range = MPlotInterval(0, 1) ) const = 0;

};


/// This implementation of a color map linearly interpolates between two colors.
class MPlotLinearColorMap : public MPlotAbstractColorMap {
public:

	/// Constructs a color map with \c start and \c end values.
	MPlotLinearColorMap(const QColor& start = QColor("black"), const QColor& finish = QColor("black"));

	///  return a QRgb (unsigned int) representing the color for a given \c value within a \c range. (Faster than returning a full QColor)
	virtual QRgb rgb(double value, MPlotInterval range = MPlotInterval(0.0, 1.0) ) const;

	/// return the color representing a given \c value within a \c range.
	virtual QColor color(double value, MPlotInterval range = MPlotInterval(0, 1) ) const;


protected:
	QColor start_, finish_;
};
*/


#endif // MPLOTCOLORMAP_H
