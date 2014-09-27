
#ifndef MPLOTCOLORMAP_CPP
#define MPLOTCOLORMAP_CPP

#include "MPlot/MPlotColorMap.h"

// System-wide pre-computed values for default color maps: optimizes the creation of new default color maps. These all have a standard resolution of 256.
QVector<QVector<QRgb>*> MPlotColorMapData::precomputedMaps_ = QVector<QVector<QRgb>*>(13,0);


MPlotColorMapData::MPlotColorMapData(const MPlotColorMapData &other)
	: QSharedData(other),
	  colorArray_(other.colorArray_),
	  colorStops_(other.colorStops_),
	  recomputeCachedColorsRequired_(other.recomputeCachedColorsRequired_),
	  standardColorMapValue_(other.standardColorMapValue_),
	  blendMode_(other.blendMode_),
	  brightness_(other.brightness_),
	  contrast_(other.contrast_),
	  gamma_(other.gamma_),
	  mustApplyBCG_(other.mustApplyBCG_)
{
}

MPlotColorMapData::MPlotColorMapData(int resolution) : colorArray_(resolution) {
	blendMode_ = MPlotColorMap::RGB;
	standardColorMapValue_ = MPlotColorMap::Jet;
	colorStops_ << QGradientStop(0.0, QColor(0, 0, 131))
				<< QGradientStop(0.121569, QColor(0, 0, 255))
				<< QGradientStop(0.372549, QColor(0, 255, 255))
				<< QGradientStop(0.623529, QColor(255, 255, 0))
				<< QGradientStop(0.874510, QColor(255, 0, 0))
				<< QGradientStop(1.0, QColor(128, 0, 0));

	recomputeCachedColorsRequired_ = true;

	mustApplyBCG_ = false;
	brightness_ = 0.;
	contrast_ = gamma_ = 1.;
}

// Constructs a default color map (Corresponding to MPlotColorMap::Jet)
MPlotColorMap::MPlotColorMap(int resolution)
	: d(new MPlotColorMapData(resolution))
{
}

MPlotColorMapData::MPlotColorMapData(const QColor& color1, const QColor& color2, int resolution) : colorArray_(resolution) {
	blendMode_ = MPlotColorMap::RGB;
	standardColorMapValue_ = -1;
	colorStops_ << QGradientStop(0.0, color1) << QGradientStop(1.0, color2);
	recomputeCachedColorsRequired_ = true;

	mustApplyBCG_ = false;
	brightness_ = 0.;
	contrast_ = gamma_ = 1.;
}

// Constructs a linear color map between \c color1 and \c color2.
MPlotColorMap::MPlotColorMap(const QColor& color1, const QColor& color2, int resolution )
	: d(new MPlotColorMapData(color1, color2, resolution))
{
}

// Constructs a color map based on a set of initial \c colorStops
MPlotColorMapData::MPlotColorMapData(const QGradientStops& colorStops, int resolution)
	: colorArray_(resolution), colorStops_(colorStops)
{
	blendMode_ = MPlotColorMap::RGB;
	standardColorMapValue_ = -1;
	recomputeCachedColorsRequired_ = true;

	mustApplyBCG_ = false;
	brightness_ = 0.;
	contrast_ = gamma_ = 1.;
}

// Constructs a color map based on a set of initial \c colorStops
MPlotColorMap::MPlotColorMap(const QGradientStops& colorStops, int resolution)
	: d(new MPlotColorMapData(colorStops, resolution))
{
}

// Convenience constructor based on the pre-built color maps that are used in other applications.  Since the positions come from indices on a 256 resolution scale, to compute the position I've used (x-1)/(resolution-1).  This gives a range between 0 and 1.
MPlotColorMapData::MPlotColorMapData(int colorMap, int resolution)
	: colorArray_(resolution)
{
	blendMode_ = MPlotColorMap::RGB;
	standardColorMapValue_ = colorMap;

	switch(colorMap){

	case MPlotColorMap::Autumn:
		colorStops_ << QGradientStop(0, QColor(255, 0, 0))
					<< QGradientStop(1, QColor(255, 255, 0));
		break;
	case MPlotColorMap::Bone:
		colorStops_ << QGradientStop(0, QColor(0, 0, 0))
					<< QGradientStop(0.372549, QColor(83, 83, 115))
					<< QGradientStop(0.749020, QColor(167, 199, 199))
					<< QGradientStop(1, QColor(255, 255, 255));
		break;
	case MPlotColorMap::Cool:
		colorStops_ << QGradientStop(0, QColor(0, 255, 255))
					<< QGradientStop(1, QColor(255, 0, 255));
		break;
	case MPlotColorMap::Copper:
		colorStops_ << QGradientStop(0, QColor(0, 0, 0))
					<< QGradientStop(1, QColor(255, 199, 127));
		break;
	case MPlotColorMap::Gray:
		colorStops_ << QGradientStop(0, QColor(0, 0, 0))
					<< QGradientStop(1, QColor(255, 255, 255));
		break;
	case MPlotColorMap::Hot:
		colorStops_ << QGradientStop(0, QColor(3, 0, 0))
					<< QGradientStop(0.372549, QColor(255, 0, 0))
					<< QGradientStop(0.749020, QColor(255, 255, 0))
					<< QGradientStop(1, QColor(255, 255, 255));
		break;
	case MPlotColorMap::Hsv:
		colorStops_ << QGradientStop(0, QColor(255, 0, 0))
					<< QGradientStop(0.4, QColor(0, 255, 99))
					<< QGradientStop(0.8, QColor(199, 0, 255))
					<< QGradientStop(1, QColor(255, 0, 6));
		blendMode_ = MPlotColorMap::HSV;
		break;

	case MPlotColorMap::Pink:// This is a linear interpolation of a non-linear calculation.
		colorStops_ << QGradientStop(0, QColor(15, 0, 0))
					<< QGradientStop(0.372549, QColor(195, 128, 128))
					<< QGradientStop(0.749020, QColor(234, 234, 181))
					<< QGradientStop(1, QColor(255, 255, 255));
		break;

	case MPlotColorMap::Spring:
		colorStops_ << QGradientStop(0, QColor(255, 0, 255))
					<< QGradientStop(1, QColor(255, 255, 0));
		break;

	case MPlotColorMap::Summer:
		colorStops_ << QGradientStop(0, QColor(0, 128, 102))
					<< QGradientStop(1, QColor(255, 255, 102));
		break;

	case MPlotColorMap::White:
		colorStops_ << QGradientStop(0, QColor(255, 255, 255))
					<< QGradientStop(1, QColor(255, 255, 255));
		break;

	case MPlotColorMap::Winter:
		colorStops_ << QGradientStop(0, QColor(0, 0, 255))
					<< QGradientStop(1, QColor(0, 255, 128));
		break;

	default:
		standardColorMapValue_ = MPlotColorMap::Jet;
	case MPlotColorMap::Jet:
		colorStops_ << QGradientStop(0.0, QColor(0, 0, 131))
					<< QGradientStop(0.121569, QColor(0, 0, 255))
					<< QGradientStop(0.372549, QColor(0, 255, 255))
					<< QGradientStop(0.623529, QColor(255, 255, 0))
					<< QGradientStop(0.874510, QColor(255, 0, 0))
					<< QGradientStop(1.0, QColor(128, 0, 0));
		break;
	}

	recomputeCachedColorsRequired_ = true;

	mustApplyBCG_ = false;
	brightness_ = 0.;
	contrast_ = gamma_ = 1.;
}

MPlotColorMap::MPlotColorMap(StandardColorMap colorMap, int resolution)
	: d(new MPlotColorMapData(colorMap, resolution))
{
}

// Replaces the current set of stop points with the given \c stopPoints. The positions of the points must be in the range 0 to 1, and must be sorted with the lowest point first.
void MPlotColorMap::setStops(const QGradientStops& stopPoints)
{
	d.detach();
	d->standardColorMapValue_ = -1;
	d->colorStops_ = stopPoints;
	d->recomputeCachedColorsRequired_ = true;
}

// Adds a stop the given \c position with the color \c color.  Note that position must be between 0 and 1.
void MPlotColorMap::addStopAt(qreal position, const QColor& color)
{
	d.detach();
	d->standardColorMapValue_ = -1;
	for (int i = 0; i < d->colorStops_.size(); i++){

		// The first time position is smaller than the current position, put it in that place.  Exit loop.
		if (position < d->colorStops_.at(i).first){

			d->colorStops_.insert(i, QGradientStop(position, color));
			break;
		}
	}

	d->recomputeCachedColorsRequired_ = true;
}

// Helper function to recompute the cached color array when the color stops, resolution, or blend mode are changed.
void MPlotColorMapData::recomputeCachedColors() const
{
	// we're going to be recomputing the colorArray_, so reset this flag.
	recomputeCachedColorsRequired_ = false;

	if(resolution() == 256 &&
			standardColorMapValue_ != -1 &&
			precomputedMaps_.at(standardColorMapValue_) ) {
		colorArray_ = *(precomputedMaps_.at(standardColorMapValue_));
		return;
	}

	// If no stops were given, produce a generic grayscale colour map.
	if (colorStops_.isEmpty()){

		qreal maxSize = (qreal)resolution();

		if (blendMode_ == MPlotColorMap::HSV)
			for (int i = 0; i < maxSize; i++)
				colorArray_[i] = QColor::fromHsvF(0, 0, i/resolution()).rgb();
		else if (blendMode_ == MPlotColorMap::RGB)
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
		qreal endMinusStart;

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

			if (blendMode_ == MPlotColorMap::HSV)
				for (int i = 0; i <= endMinusStart; i++)
					colorArray_[startIndex+i] = QColor::fromHsv(int(start.second.hue()+(end.second.hue()-start.second.hue())*i/endMinusStart),
																int(start.second.saturation()+(end.second.saturation()-start.second.saturation())*i/endMinusStart),
																int(start.second.value()+(end.second.value()-start.second.value())*i/endMinusStart),
																int(start.second.alpha()+(end.second.alpha()-start.second.alpha())*i/endMinusStart))
							.rgb();
			else if (blendMode_ == MPlotColorMap::RGB)
				for (int i = 0; i <= endMinusStart; i++)
					colorArray_[startIndex+i] = QColor::fromRgb(int(start.second.red()+(end.second.red()-start.second.red())*i/endMinusStart),
																int(start.second.green()+(end.second.green()-start.second.green())*i/endMinusStart),
																int(start.second.blue()+(end.second.blue()-start.second.blue())*i/endMinusStart),
																int(start.second.alpha()+(end.second.alpha()-start.second.alpha())*i/endMinusStart))
							.rgb();
		}

		// If the last stop isn't at 1 then fill the rest of the colour map with the last stop's colour.
		if (colorStops_.last().first < 1.0)
			for (int i = colorIndex(colorStops_.last()); i < resolution(); i++)
				colorArray_[i] = colorStops_.last().second.rgb();
	}



	// system-wide optimization: sharing standard color maps. (Valid only for default resolution of 256)
	// Here we share the map we've just computed, if it hasn't been shared already.
	if(resolution() == 256 &&
			standardColorMapValue_ != -1 &&
			precomputedMaps_.at(standardColorMapValue_) == 0) {

		precomputedMaps_[standardColorMapValue_] = new QVector<QRgb>();
		*(precomputedMaps_[standardColorMapValue_]) = colorArray_;
	}
}

bool MPlotColorMapData::operator !=(const MPlotColorMapData &other) const
{
	if(brightness_ != other.brightness_ || contrast_ != other.contrast_ || gamma_ != other.gamma_)
		return true;
	if(blendMode_ != other.blendMode_)
		return true;
	if(resolution() != other.resolution())
		return true;
	if(standardColorMapValue_ != other.standardColorMapValue_)
		return true;
	// if standardColorMapValue_s are both -1, we have custom maps. Need to compare stops.
	if(standardColorMapValue_ == -1 && colorStops_ != other.colorStops_)
		return true;

	return false;	// they're the same!
}

bool MPlotColorMap::rgbValues(const QVector<qreal> &values, MPlotRange range, QRgb *output)
{
	if (d->recomputeCachedColorsRequired_)
		d->recomputeCachedColors();

	if (range.x() == range.y()){

		int size = values.size();
		QVector<QRgb> defaultValues = QVector<QRgb>(size, rgbAtIndex(0));
		memcpy(output, defaultValues.constData(), size*sizeof(QRgb));
	}

	else {

		qreal rangeMinimum = range.x();
		qreal rangeDifference = range.y() - rangeMinimum;
		int lastColorArrayIndex = d->colorArray_.size() - 1;
		qreal contrast = d->contrast_;
		qreal brightness = d->brightness_;
		qreal gamma = d->gamma_;
		QVector<QRgb> colorArray = d->colorArray_;
		int colorArraySize = colorArray.size();

		if (d->mustApplyBCG_){

			if (gamma == 1.0){

				for (int i = 0, size = values.size(); i < size; i++){

					int index = (int)qRound((contrast*((values.at(i)-rangeMinimum)/rangeDifference+brightness))*lastColorArrayIndex);

					if (index < 0)
						index = 0;

					else if (index >= colorArraySize)
						index = lastColorArrayIndex;

					output[i] = colorArray.at(index);
				}
			}

			else{

				for(int i = 0, size = values.size(); i < size; i++){

					int index = (int)qRound((contrast*(pow((values.at(i)-rangeMinimum)/rangeDifference, gamma)+brightness))*lastColorArrayIndex);

					if (index < 0)
						index = 0;

					else if (index >= colorArraySize)
						index = lastColorArrayIndex;

					output[i] = colorArray.at(index);
				}
			}
		}

		else{

			for(int i = 0, size = values.size(); i < size; i++){

				int index = (int)qRound(((values.at(i)-rangeMinimum)/rangeDifference*lastColorArrayIndex));

				if (index < 0)
					index = 0;

				else if (index >= colorArraySize)
					index = lastColorArrayIndex;

				output[i] = colorArray.at(index);
			}
		}
	}

	return true;
}

bool MPlotColorMap::rgbValues(const QVector<qreal> &values, QRgb *output)
{
	if (d->recomputeCachedColorsRequired_)
		d->recomputeCachedColors();

	int lastColorArrayIndex = d->colorArray_.size() - 1;
	qreal contrast = d->contrast_;
	qreal brightness = d->brightness_;
	qreal gamma = d->gamma_;
	QVector<QRgb> colorArray = d->colorArray_;
	int colorArraySize = colorArray.size();

	if (d->mustApplyBCG_){

		if (d->gamma_ == 1.0){

			for (int i = 0, size = values.size(); i < size; i++){

				int index = (int)qRound((contrast*(values.at(i)+brightness))*colorArraySize);

				if (index < 0)
					index = 0;

				else if (index >= colorArraySize)
					index = lastColorArrayIndex;

				output[i] = colorArray.at(index);
			}
		}

		else{

			for(int i = 0, size = values.size(); i < size; i++){

				int index = (int)qRound((contrast*(pow(values.at(i), gamma)+brightness))*colorArraySize);

				if (index < 0)
					index = 0;

				else if (index >= colorArraySize)
					index = lastColorArrayIndex;

				output[i] = colorArray.at(index);
			}
		}
	}

	else{

		for(int i = 0, size = values.size(); i < size; i++){

			int index = (int)qRound((values.at(i)*colorArraySize));

			if (index < 0)
				index = 0;

			else if (index >= colorArraySize)
				index = lastColorArrayIndex;

			output[i] = colorArray.at(index);
		}
	}

	return true;
}

bool MPlotColorMap::rgbValues(const QVector<int> &values, QRgb *output)
{
	if (d->recomputeCachedColorsRequired_)
		d->recomputeCachedColors();

	int lastColorArrayIndex = d->colorArray_.size() - 1;
	QVector<QRgb> colorArray = d->colorArray_;
	int colorArraySize = colorArray.size();

	for (int i = 0, size = values.size(); i < size; i++){

		int index = values.at(i);

		if (index < 0)
			index = 0;

		else if (index >= colorArraySize)
			index = lastColorArrayIndex;

		output[i] = colorArray.at(index);
	}

	return true;
}

void MPlotColorMap::setBrightness(qreal brightness)
{
	d.detach();
	d->brightness_ = brightness;
	d->mustApplyBCG_ = !(d->brightness_ == 0.0 && d->contrast_ == 1.0 && d->gamma_ == 1.0);
}

void MPlotColorMap::setContrast(qreal contrast)
{
	d.detach();
	d->contrast_ = contrast;
	d->mustApplyBCG_ = !(d->brightness_ == 0.0 && d->contrast_ == 1.0 && d->gamma_ == 1.0);
}

void MPlotColorMap::setGamma(qreal gamma)
{
	d.detach();
	d->gamma_ = gamma;
	d->mustApplyBCG_ = !(d->brightness_ == 0.0 && d->contrast_ == 1.0 && d->gamma_ == 1.0);
}

bool MPlotColorMap::operator !=(const MPlotColorMap &other) const
{
	 return *d != *(other.d);
}

#endif // MPLOTCOLORMAP_H
