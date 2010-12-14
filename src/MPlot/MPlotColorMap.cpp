
#ifndef MPLOTCOLORMAP_CPP
#define MPLOTCOLORMAP_CPP

#include "MPlotColorMap.h"


/// Constructs a default color map (Corresponding to MPlotColorMap::Jet)
MPlotColorMap::MPlotColorMap(int resolution)
	: colorArray_(resolution)
{
	blendMode_ = RGB;
	colorStops_ << QGradientStop(0.0, QColor(0, 0, 131))
			<< QGradientStop(0.121569, QColor(0, 0, 255))
			<< QGradientStop(0.372549, QColor(0, 255, 255))
			<< QGradientStop(0.623529, QColor(255, 255, 0))
			<< QGradientStop(0.874510, QColor(255, 0, 0))
			<< QGradientStop(1.0, QColor(128, 0, 0));
	recomputeCachedColorsRequired_ = true;
}

/// Constructs a linear color map between \c color1 and \c color2.
MPlotColorMap::MPlotColorMap(const QColor& color1, const QColor& color2, int resolution )
	: colorArray_(resolution)
{
	blendMode_ = RGB;
	colorStops_ << QGradientStop(0.0, color1) << QGradientStop(1.0, color2);
	recomputeCachedColorsRequired_ = true;
}

/// Constructs a color map based on a set of initial \c colorStops
MPlotColorMap::MPlotColorMap(const QGradientStops& colorStops, int resolution)
	: colorArray_(resolution), colorStops_(colorStops)
{
	blendMode_ = RGB;
	recomputeCachedColorsRequired_ = true;
}

/// Convenience constructor based on the pre-built color maps that are used in other applications.  Since the positions come from indices on a 256 resolution scale, to compute the position I've used (x-1)/(resolution-1).  This gives a range between 0 and 1.
MPlotColorMap::MPlotColorMap(StandardColorMap colorMap, int resolution)
	: colorArray_(resolution)
{
	blendMode_ = RGB;

	switch(colorMap){

	case Autumn:
		colorStops_ << QGradientStop(0, QColor(255, 0, 0))
				<< QGradientStop(1, QColor(255, 255, 0));
		break;
	case Bone:
		colorStops_ << QGradientStop(0, QColor(0, 0, 0))
				<< QGradientStop(0.372549, QColor(83, 83, 115))
				<< QGradientStop(0.749020, QColor(167, 199, 199))
				<< QGradientStop(1, QColor(255, 255, 255));
		break;
	case Cool:
		colorStops_ << QGradientStop(0, QColor(0, 255, 255))
				<< QGradientStop(1, QColor(255, 0, 255));
		break;
	case Copper:
		colorStops_ << QGradientStop(0, QColor(0, 0, 0))
				<< QGradientStop(1, QColor(255, 199, 127));
		break;
	case Gray:
		colorStops_ << QGradientStop(0, QColor(0, 0, 0))
				<< QGradientStop(1, QColor(255, 255, 255));
		break;
	case Hot:
		colorStops_ << QGradientStop(0, QColor(3, 0, 0))
				<< QGradientStop(0.372549, QColor(255, 0, 0))
				<< QGradientStop(0.749020, QColor(255, 255, 0))
				<< QGradientStop(1, QColor(255, 255, 255));
		break;
	case Hsv:
		colorStops_ << QGradientStop(0, QColor(255, 0, 0))
				<< QGradientStop(0.4, QColor(0, 255, 99))
				<< QGradientStop(0.8, QColor(199, 0, 255))
				<< QGradientStop(1, QColor(255, 0, 6));
		setBlendMode(HSV);
		break;
	case Jet:
		colorStops_ << QGradientStop(0.0, QColor(0, 0, 131))
				<< QGradientStop(0.121569, QColor(0, 0, 255))
				<< QGradientStop(0.372549, QColor(0, 255, 255))
				<< QGradientStop(0.623529, QColor(255, 255, 0))
				<< QGradientStop(0.874510, QColor(255, 0, 0))
				<< QGradientStop(1.0, QColor(128, 0, 0));
		break;
	case Pink:// This is a linear interpolation of a non-linear calculation.
		colorStops_ << QGradientStop(0, QColor(15, 0, 0))
				<< QGradientStop(0.372549, QColor(195, 128, 128))
				<< QGradientStop(0.749020, QColor(234, 234, 181))
				<< QGradientStop(1, QColor(255, 255, 255));
		break;
	case Spring:
		colorStops_ << QGradientStop(0, QColor(255, 0, 255))
				<< QGradientStop(1, QColor(255, 255, 0));
		break;
	case Summer:
		colorStops_ << QGradientStop(0, QColor(0, 128, 102))
				<< QGradientStop(1, QColor(255, 255, 102));
		break;
	case White:
		colorStops_ << QGradientStop(0, QColor(255, 255, 255))
				<< QGradientStop(1, QColor(255, 255, 255));
		break;
	case Winter:
		colorStops_ << QGradientStop(0, QColor(0, 0, 255))
				<< QGradientStop(1, QColor(0, 255, 128));
		break;
	}

	recomputeCachedColorsRequired_ = true;
}

/// Replaces the current set of stop points with the given \c stopPoints. The positions of the points must be in the range 0 to 1, and must be sorted with the lowest point first.
void MPlotColorMap::setStops(const QGradientStops& stopPoints)
{
	colorStops_ = stopPoints;
	recomputeCachedColorsRequired_ = true;
}

/// Adds a stop the given \c position with the color \c color.  Note that position must be between 0 and 1.
void MPlotColorMap::addStopAt(double position, const QColor& color)
{
	for (int i = 0; i < colorStops_.size(); i++){

		// The first time position is smaller than the current position, put it in that place.  Exit loop.
		if (position < colorStops_.at(i).first){

			colorStops_.insert(i, QGradientStop(position, color));
			break;
		}
	}

	recomputeCachedColorsRequired_ = true;
}

/// Helper function to recompute the cached color array when the color stops, resolution, or blend mode are changed.
void MPlotColorMap::recomputeCachedColors() const
{

	// If no stops were given, produce a generic grayscale colour map.
	if (colorStops_.isEmpty()){

		double maxSize = (double)resolution();

		if (blendMode() == HSV)
			for (int i = 0; i < maxSize; i++)
				colorArray_[i] = QColor::fromHsvF(0, 0, i/resolution()).rgb();
		else if (blendMode() == RGB)
			for (int i = 0; i < maxSize; i++)
				colorArray_[i] = QColor::fromRgbF(i/maxSize, i/maxSize, i/maxSize).rgb();
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
		double endMinusStart;

		// If the first stop isn't at 0 then fill with the first colour up to the index of the first stop.
		if (colorStops_.first().first > 0.0)
			for (int i = 0; i < colorIndex(colorStops_.first()); i++)
				colorArray_[i] = colorStops_.first().second.rgb();

		// General fill algorithm based on two stops.
		for (int j = 0; j < colorStops_.size()-1; j++){

			start = colorStops_.at(j);
			end = colorStops_.at(j+1);
			startIndex = colorIndex(start);
			endIndex = colorIndex(end);
			endMinusStart = endIndex-startIndex;

			if (blendMode() == HSV)
				for (int i = 0; i <= endMinusStart; i++)
					colorArray_[startIndex+i] = QColor::fromHsv(start.second.hue()+(end.second.hue()-start.second.hue())*i/endMinusStart,
													  start.second.saturation()+(end.second.saturation()-start.second.saturation())*i/endMinusStart,
													  start.second.value()+(end.second.value()-start.second.value())*i/endMinusStart,
													  start.second.alpha()+(end.second.alpha()-start.second.alpha())*i/endMinusStart)
													  .rgb();
			else if (blendMode() == RGB)
				for (int i = 0; i <= endMinusStart; i++)
					colorArray_[startIndex+i] = QColor::fromRgb(start.second.red()+(end.second.red()-start.second.red())*i/endMinusStart,
													  start.second.green()+(end.second.green()-start.second.green())*i/endMinusStart,
													  start.second.blue()+(end.second.blue()-start.second.blue())*i/endMinusStart,
													  start.second.alpha()+(end.second.alpha()-start.second.alpha())*i/endMinusStart)
													  .rgb();
		}

		// If the last stop isn't at 1 then fill the rest of the colour map with the last stop's colour.
		if (colorStops_.last().first < 1.0)
			for (int i = colorIndex(colorStops_.last()); i < resolution(); i++)
				colorArray_[i] = colorStops_.last().second.rgb();
	}

	// we're done recomputing the colorArray_, so reset this flag.
	recomputeCachedColorsRequired_ = false;
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
