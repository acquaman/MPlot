
#ifndef MPLOTCOLORMAP_CPP
#define MPLOTCOLORMAP_CPP

#include "MPlotColorMap.h"


/// Constructs a default color map (Corresponding to MPlotColorMap::Jet)
MPlotColorMap::MPlotColorMap(int resolution = 256)
{
	// Jet colorStops_.
	colorArray_.resize(resolution);
	recomputeCachedColors();
}

/// Constructs a linear color map between \c color1 and \c color2.
MPlotColorMap::MPlotColorMap(const QColor& color1, const QColor& color2, int resolution = 256)
{
	colorStops_ << QGradientStop(0, color1) << QGradientStop(1, color2);
	colorArray_.resize(resolution);
	recomputeCachedColors();
}

/// Constructs a color map based on a set of initial \c colorStops
MPlotColorMap::MPlotColorMap(const QGradientStops& colorStops, int resolution = 256)
{
	colorStops_ << colorStops;
	colorArray_.resize(resolution);
	recomputeCachedColors();
}

/// Convenience constructor based on the pre-built color maps that are used in other applications.
MPlotColorMap::MPlotColorMap(StandardColorMap colorMap, int resolution = 256)
{
	switch(colorMap){
		// stuff.
	}

	colorArray_.resize(resolution);
	recomputeCachedColors();
}

/// Assumes range is 0 to 1.
QRgb MPlotColorMap::rgbAt(double value) const
{
	rgbAt(value, MPlotInterval(0.0, 1.0));
}

/// Assumes a MPlotInterval of (smallest, largest).
QRgb MPlotColorMap::rgbAt(double value, MPlotInterval range) const
{
	if (value < range.first)
		return colorArray_.first();

	if (value > range.second)
		return colorArray_.last();

	return colorArray_.at((int)round((value/range.second)*resolution()));
}

QRgb MPlotColorMap::rgbAtIndex(int index) const
{
	return colorArray_.at(index);
}

/// Returns the stop points for this gradient.
/*! If no stop points have been specified, a gradient of black at 0 to white at 1 is used.*/
QGradientStops MPlotColorMap::stops() const
{
	return colorStops_;
}

/// Replaces the current set of stop points with the given \c stopPoints. The positions of the points must be in the range 0 to 1, and must be sorted with the lowest point first.
void MPlotColorMap::setStops(const QGradientStops& stopPoints)
{
	colorStops_.clear();
	colorStops_ << stopPoints;
	recomputeCachedColors();
}

/// Adds a stop the given \c position with the color \c color.
void MPlotColorMap::addStopAt(double position, const QColor& color)
{
	colorStops_.insert(floor(position*resolution()), color);
	recomputeCachedColors();
}

/// Returns the resolution (number of color steps) in the pre-computed color map.
int MPlotColorMap::resolution() const
{
	return colorArray_.size();
}

/// Set the resolution (number of color steps) in the pre-computed color map.  The default is 256.  Higher resolution could produce a smoother image, but will require more memory.  (For comparison, Matlab's default resolution is 64.)
void MPlotColorMap::setResolution(int newResolution)
{
	colorArray_.resize(newResolution);
	recomputeCachedColors();
}


/// Returns the interpolation mode used to interpolate between color stops.  RGB is fastest, while HSV preserves human-perception-based color relationships.
BlendMode MPlotColorMap::blendMode() const
{
	return blendMode_;
}

/// Set the interpolation mode used to interpolate between color stops.
void MPlotColorMap::setBlendMode(BlendMode newBlendMode)
{
	blendMode_ = newBlendMode;
}

/// Helper function to recompute the cached color array when the color stops, resolution, or blend mode are changed.
void MPlotColorMap::recomputeCachedColors()
{
	colorArray_.clear();
	colorArray_.resize(resolution());

	if (colorStops_.isEmpty()){

		QColor color;
		for (int i = 0; i < resolution(); i++){

			if (blendMode() == RGB)
				color = QColor::fromHsv(0, 0, i/resolution());
			else
				color = QColor::fromRgb(i/resolution(), i/resolution(), i/resolution());
			colorArray_.insert(i, color.rgb());
		}
	}

	else if (colorStops_.size() == 1)
		colorArray_.fill(colorStops_.first().second.rgb());

	else {

		QGradientStop start;
		QGradientStop end;
		int startIndex;
		int endIndex;

		if (colorStops_.first().first != 0.0)
			colorArray_.insert(0, colorIndex(colorStops_.first()), colorStops_.first().second.rgb());
// Need to check that my interpolation works correctly.  Pretty sure it's incorrect at the moment.
		for (int i = 0; i < colorStops_.size()-1; i++){

			start = colorStops_.at(i);
			end = colorStops_.at(i+1);
			startIndex = colorIndex(start);
			endIndex = colorIndex(end);

			for (int i = startIndex; i < endIndex; i++){

				colorArray_.insert(i, QColor::fromRgb((i-start.second.red())/(end.second.red()-start.second.red()),
													  (i-start.second.blue())/(end.second.blue()-start.second.blue()),
													  (i-start.second.green())/(end.second.green()-start.second.green()),
													  (i-start.second.alpha())/(end.second.alpha()-start.second.alpha())));
			}
		}

		if (colorStops_.last().first != 1.0)
			colorArray_.insert(colorIndex(colorStops_.last()), resolution()-1, colorStops_.last().second);
	}
}

int MPlotColorMap::colorIndex(QGradientStop stop)
{
	return (int)floor(stop.first*resolution());
}

/*
/// Constructs a color map with \c start and \c end values.
MPlotLinearColorMap::MPlotLinearColorMap(const QColor& start, const QColor& finish)
	: start_(start), finish_(finish)
{
}

///  return a QRgb (unsigned int) representing the color for a given \c value within a \c range. (Faster than returning a full QColor)
QRgb MPlotLinearColorMap::rgb(double value, MPlotInterval range) const {

	double Ratio = (value - range.first)/(range.second - range.first);
	QRgb Col1 = start_.rgba();
	QRgb Col2 = finish_.rgba();

	int A1=qAlpha(Col1), R1=qRed(Col1), G1=qGreen(Col1), B1=qBlue(Col1);
	int A2=qAlpha(Col2), R2=qRed(Col2), G2=qGreen(Col2), B2=qBlue(Col2);
	//int A2=(Col2>>24)&0xFF, R2=(Col2>>16)&0xFF, G2=(Col2>>8)&0xFF, B2=Col2&0xFF;
	int Color;
	if (Ratio<=0) return Col1;	// Ratio parameter must be between 0 and 1
	else if (Ratio>=1) return Col2;
	Color=	(int)(A1+(A2-A1)*Ratio+0.5)<<24 |
			(int)(R1+(R2-R1)*Ratio+0.5)<<16 |		// rounding
			(int)(G1+(G2-G1)*Ratio+0.5)<<8  |
			(int)(B1+(B2-B1)*Ratio+0.5);
	return Color;

}
/// return the color representing a given \c value within a \c range.
QColor MPlotLinearColorMap::color(double value, MPlotInterval range) const {
	return QColor::fromRgba(rgb(value, range));
}
*/



#endif // MPLOTCOLORMAP_H
