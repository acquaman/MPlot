
#ifndef MPLOTCOLORMAP_CPP
#define MPLOTCOLORMAP_CPP

#include "MPlotColorMap.h"


/// Constructs a default color map (Corresponding to MPlotColorMap::Jet)
MPlotColorMap::MPlotColorMap(int resolution)
{
	// Jet colorStops_.
	colorArray_.resize(resolution);
	recomputeCachedColors();
}

/// Constructs a linear color map between \c color1 and \c color2.
MPlotColorMap::MPlotColorMap(const QColor& color1, const QColor& color2, int resolution )
{
	colorStops_ << QGradientStop(0, color1) << QGradientStop(1, color2);
	colorArray_.resize(resolution);
	recomputeCachedColors();
}

/// Constructs a color map based on a set of initial \c colorStops
MPlotColorMap::MPlotColorMap(const QGradientStops& colorStops, int resolution)
{
	colorStops_ << colorStops;
	colorArray_.resize(resolution);
	recomputeCachedColors();
}

/// Convenience constructor based on the pre-built color maps that are used in other applications.
MPlotColorMap::MPlotColorMap(StandardColorMap colorMap, int resolution)
{
	switch(colorMap){

	case Autumn:
		break;
	case Bone:
		break;
	case Cool:
		break;
	case Copper:
		break;
	case Flag:
		break;
	case Gray:
		break;
	case Hot:
		break;
	case Jet:
		break;
	case Pink:
		break;
	case Spring:
		break;
	case Summer:
		break;
	case White:
		break;
	case Winter:
		break;
	}

	colorArray_.resize(resolution);
	recomputeCachedColors();
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
	for (int i = 0; i < colorStops_.size(); i++){

		// The first time position is smaller than the current position, put it in that place.  Exit loop.
		if (position < colorStops_.at(i).first){

			colorStops_.insert(i, QGradientStop(floor(position*resolution()), color));
			break;
		}
	}

	recomputeCachedColors();
}

/// Helper function to recompute the cached color array when the color stops, resolution, or blend mode are changed.
void MPlotColorMap::recomputeCachedColors()
{
	// Resize the QVector if required.
	colorArray_.resize(resolution());

	// If no stops were given, produce a generic grayscale colour map.
	if (colorStops_.isEmpty()){

		for (int i = 0; i < resolution(); i++){

			if (blendMode() == HSV)
				colorArray_.insert(i, QColor::fromHsv(0, 0, i/resolution()).rgb());
			else
				colorArray_.insert(i, QColor::fromRgb(i/resolution(), i/resolution(), i/resolution()).rgb());
		}
	}

	// If a single stop is given, the color map is a single colour.
	else if (colorStops_.size() == 1)
		colorArray_.fill(colorStops_.first().second.rgb());

	// Otherwise, interpolate a colour map based on the number of stops given.
	else {

		QGradientStop start;
		QGradientStop end;
		int startIndex;
		int endIndex;

		// If the first stop isn't at 0 then fill with the first colour up to the index of the first stop.
		if (colorStops_.first().first != 0.0)
			colorArray_.insert(0, colorIndex(colorStops_.first()), colorStops_.first().second.rgb());

		// General fill algorithm based on two stops.
		for (int i = 0; i < colorStops_.size()-1; i++){

			start = colorStops_.at(i);
			end = colorStops_.at(i+1);
			startIndex = colorIndex(start);
			endIndex = colorIndex(end);

			for (int i = startIndex; i < endIndex; i++){

				if (blendMode() == HSV)

					colorArray_.insert(i, QColor::fromHsv(start.second.hue()+(end.second.hue()-start.second.hue())*i/(endIndex-startIndex),
													  start.second.saturation()+(end.second.saturation()-start.second.saturation())*i/(endIndex-startIndex),
													  start.second.value()+(end.second.value()-start.second.value())*i/(endIndex-startIndex),
													  start.second.alpha()+(end.second.alpha()-start.second.alpha())*i/(endIndex-startIndex))
													  .rgb());
				else
					colorArray_.insert(i, QColor::fromRgb(start.second.red()+(end.second.red()-start.second.red())*i/(endIndex-startIndex),
													  start.second.green()+(end.second.green()-start.second.green())*i/(endIndex-startIndex),
													  start.second.blue()+(end.second.blue()-start.second.blue())*i/(endIndex-startIndex),
													  start.second.alpha()+(end.second.alpha()-start.second.alpha())*i/(endIndex-startIndex))
													  .rgb());
			}
		}

		// If the last stop isn't at 1 then fill the rest of the colour map with the last stop's colour.
		if (colorStops_.last().first != 1.0)
			colorArray_.insert(colorIndex(colorStops_.last()), resolution()-1, colorStops_.last().second.rgb());
	}
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
