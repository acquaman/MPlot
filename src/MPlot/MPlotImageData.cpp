#ifndef MPLOTIMAGEDATA_CPP
#define MPLOTIMAGEDATA_CPP

#include "MPlot/MPlotImageData.h"

MPlotImageDataSignalSource::MPlotImageDataSignalSource(MPlotAbstractImageData *parent)
	: QObject(0)
{
	data_ = parent;
}

MPlotAbstractImageData::MPlotAbstractImageData()
{
	signalSource_ = new MPlotImageDataSignalSource(this);
	range_ = MPlotRange();
}

MPlotAbstractImageData::~MPlotAbstractImageData()
{
	delete signalSource_;
	signalSource_ = 0;
}

MPlotRange MPlotAbstractImageData::range() const
{
	return range_;
}

// MPlotSimpleImageData
// /////////////////////////////////////////////

MPlotSimpleImageData::MPlotSimpleImageData(int xSize, int ySize)
	: MPlotAbstractImageData()
{
	x_ = QVector<qreal>(xSize);
	y_ = QVector<qreal>(ySize);
	z_ = QVector<qreal>(xSize*ySize);
}

qreal MPlotSimpleImageData::x(int indexX) const
{
	return x_.at(indexX);
}

qreal MPlotSimpleImageData::y(int indexY) const
{
	return y_.at(indexY);
}

qreal MPlotSimpleImageData::z(int indexX, int indexY) const
{
	return z_.at(indexX + indexY*x_.size());
}

QRectF MPlotSimpleImageData::boundingRect() const
{
	return boundingRect_;
}

QPoint MPlotSimpleImageData::count() const
{
	return QPoint(x_.size(), y_.size());
}

void MPlotSimpleImageData::setZ(int indexX, int indexY, qreal z)
{
	if (range_.isNull()){

		range_.setX(z);
		range_.setY(z);
	}

	else {

		if (z < range_.x())
			range_.setX(z);

		if (z > range_.y())
			range_.setY(z);
	}

	z_[indexX + indexY*x_.size()] = z;
	emitDataChanged();
}

void MPlotSimpleImageData::recomputeBoundingRect()
{
	double minimumX = x_.first();
	double maximumX = x_.last();
	double minimumY = y_.first();
	double maximumY = y_.last();

	if(maximumX < minimumX)
		qSwap(minimumX, maximumX);

	if(maximumY < minimumY)
		qSwap(minimumY, maximumY);

	boundingRect_ = QRectF(minimumX, minimumY, maximumX-minimumX, maximumY-minimumY);
}

void MPlotSimpleImageData::zValues(int xStart, int yStart, int xEnd, int yEnd, qreal *outputValues) const
{
	if ((xEnd-xStart+1) == x_.size() && (yEnd-yStart+1) == y_.size())
		memcpy(outputValues, z_.constData(), z_.size()*sizeof(qreal));

	else{

		int ySize = y_.size();

		for (int j = 0, jSize = yEnd-yStart+1; j < jSize; j++)
			for (int i = 0, iSize = xEnd-xStart+1; i < iSize; i++)
				outputValues[i*jSize+j] = z_.at((i+xStart)*ySize + (j+yStart));
	}
}

void MPlotSimpleImageData::setXValues(int start, int end, qreal *newValues)
{
	memcpy(x_.data()+start, newValues, (end-start+1)*sizeof(qreal));
	recomputeBoundingRect();
	MPlotAbstractImageData::emitBoundsChanged();
}

void MPlotSimpleImageData::setYValues(int start, int end, qreal *newValues)
{
	memcpy(y_.data()+start, newValues, (end-start+1)*sizeof(qreal));
	recomputeBoundingRect();
	MPlotAbstractImageData::emitBoundsChanged();
}

void MPlotSimpleImageData::setZValues(int xStart, int yStart, int xEnd, int yEnd, qreal *newValues)
{
	int ySize = y_.size();
	int xOffset = xStart*y_.size();
	int yOffset = yStart;
	double rangeMinimum = newValues[0];
	double rangeMaximum = newValues[0];

	for (int j = 0, jSize = yEnd-yStart+1; j < jSize; j++){

		for (int i = 0, iSize = xEnd-xStart+1; i < iSize; i++){

			double newValue = newValues[i*jSize+j];

			if (newValue > rangeMaximum)
				rangeMaximum = newValue;

			if (newValue < rangeMinimum)
				rangeMinimum = newValue;

			z_[i*ySize+xOffset + j + yOffset] = newValue;
		}
	}

	range_ = MPlotRange(rangeMinimum, rangeMaximum);

	MPlotAbstractImageData::emitDataChanged();
}

// MPlotSimpleImageDatawDefault
// ////////////////////////////////////////////

MPlotSimpleImageDatawDefault::MPlotSimpleImageDatawDefault(int xSize, int ySize, qreal defaultValue)
	: MPlotSimpleImageData(xSize, ySize)
{
	defaultValue_ = defaultValue;
}

void MPlotSimpleImageDatawDefault::setZValues(int xStart, int yStart, int xEnd, int yEnd, qreal *newValues)
{
	int ySize = y_.size();
	int xOffset = xStart*y_.size();
	int yOffset = yStart;
	double rangeMinimum = newValues[0];
	double rangeMaximum = newValues[0];

	for (int j = 0, jSize = yEnd-yStart+1; j < jSize; j++){

		for (int i = 0, iSize = xEnd-xStart+1; i < iSize; i++){

			double newValue = newValues[i*jSize+j];

			if (newValue > rangeMaximum)
				rangeMaximum = newValue;

			if (newValue < rangeMinimum && newValue != defaultValue_)
				rangeMinimum = newValue;

			z_[i*ySize+xOffset + j + yOffset] = newValue;
		}
	}

	range_ = MPlotRange(rangeMinimum, rangeMaximum);

	MPlotAbstractImageData::emitDataChanged();
}

void MPlotSimpleImageDatawDefault::setZ(int indexX, int indexY, qreal z)
{
	if (range_.isNull()){

		range_.setX(z);
		range_.setY(z);
	}

	else {

		if (z < range_.x() && z != defaultValue_)
			range_.setX(z);

		if (z > range_.y())
			range_.setY(z);
	}

	z_[indexX*y_.size() + indexY] = z;
	emitDataChanged();
}

#endif // MPLOTIMAGEDATA_H
