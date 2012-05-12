#ifndef MPLOTIMAGEDATA_CPP
#define MPLOTIMAGEDATA_CPP

#include "MPlotImageData.h"

MPlotImageDataSignalSource::MPlotImageDataSignalSource(MPlotAbstractImageData *parent)
	: QObject(0) {
	data_ = parent;
}

MPlotAbstractImageData::MPlotAbstractImageData()
{
	signalSource_ = new MPlotImageDataSignalSource(this);
	minMaxCacheUpdateRequired_ = true;
}

MPlotAbstractImageData::~MPlotAbstractImageData()
{
	delete signalSource_;
	signalSource_ = 0;
}

void MPlotAbstractImageData::minMaxSearch() const {
	QPoint c = count();
	int sizeX = c.x();
	int sizeY = c.y();

	if(sizeX == 0 || sizeY == 0)
		return;

	// performance optimization.  If total points is less than 500, just call z() repeatedly.  If total points is over that, usually faster to allocate a vector and use the block zValues().  However, to limit memory usage, don't allocate blocks over 1MB (125000 doubles).
	if(sizeX*sizeY < 500) {
		qreal minZ, maxZ, d;
		minZ = maxZ = z(0,0);
		for(int xx=0; xx<sizeX; ++xx)
			for(int yy=0; yy<sizeY; ++yy) {
				d = z(xx,yy);
				if(d<minZ) minZ=d;
				if(d>maxZ) maxZ=d;
			}
		minMaxCache_.first = minZ;
		minMaxCache_.second = maxZ;
		minMaxCacheUpdateRequired_ = false;
	}
	else if(sizeX*sizeY < 125000) {	// less than 1MB buffer: do in one shot
		QVector<qreal> dataBuffer(sizeX*sizeY);
		zValues(0,0, sizeX-1, sizeY-1, dataBuffer.data());
		qreal minZ, maxZ;
		minZ = maxZ = dataBuffer.at(0);
		foreach(qreal d, dataBuffer) {
			if(d<minZ) minZ=d;
			if(d>maxZ) maxZ=d;
		}
		minMaxCache_.first = minZ;
		minMaxCache_.second = maxZ;
		minMaxCacheUpdateRequired_ = false;
	}
	else {	// large array; don't want to allocate more than 1MB buffer. Do in sections of approximately 1MB.
		int rowsAtOnce = 125000 / sizeY;
		if(rowsAtOnce == 0) rowsAtOnce = 1;
		qreal minZ, maxZ;
		minZ = maxZ = z(0,0);
		QVector<qreal> dataBuffer(rowsAtOnce*sizeY);

		for(int xrow=0; xrow<sizeX; xrow+=rowsAtOnce) {
			int maxRow = qMin(sizeX-1, xrow+rowsAtOnce-1);
			dataBuffer.resize((maxRow-xrow+1)*sizeY);	// for all blocks except the last block, will do nothing. Resizing on the last (partial) block allows us to use foreach, which is faster than a for-loop over the space we know we have.
			zValues(xrow, 0, maxRow, sizeY-1, dataBuffer.data());

			foreach(qreal d, dataBuffer) {
				if(d<minZ) minZ=d;
				if(d>maxZ) maxZ=d;
			}
		}
		minMaxCache_.first = minZ;
		minMaxCache_.second = maxZ;
		minMaxCacheUpdateRequired_ = false;
	}
}

MPlotInterval MPlotAbstractImageData::range() const {

	// empty data set? Return default interval of (0,1)
	QPoint c = count();
	if(c.x() == 0 || c.y() == 0)
		return MPlotInterval(0,1);

	if(minMaxCacheUpdateRequired_) {
		minMaxSearch();
	}

	return minMaxCache_;
}

// This class is a very basic 2D array which implements the MPlotAbstractImageData interface

// Constructor: represent image data with physical coordinate boundaries \c dataBounds, and a resolution (number of "pixels") \c resolution.  Data values are initialized to 0.
MPlotSimpleImageData::MPlotSimpleImageData(const QRectF& dataBounds, const QSize& resolution)
	: MPlotAbstractImageData(),
	num_(resolution.expandedTo(QSize(1,1)).width(), resolution.expandedTo(QSize(1,1)).height()),
	d_(num_.y(), QVector<qreal>(num_.x(), 0)),
	bounds_(dataBounds)
{
	// max and min trackers are valid from the beginning; every data value is 0, so we might as well use z(0,0) = max = min.
	minIndex_ = QPoint(0,0);
	maxIndex_ = QPoint(0,0);

}

// Return the x (data value) corresponding an (x,y) \c index:
qreal MPlotSimpleImageData::x(int indexX) const  {

	return bounds_.left() + bounds_.width()*indexX/num_.x();

}

// Return the y (data value) corresponding an (x,y) \c index:
qreal MPlotSimpleImageData::y(int indexY) const {

	return bounds_.top() + bounds_.height()*indexY/num_.y();
}

// Return the z = f(x,y) value corresponding an (x,y) \c index:
qreal MPlotSimpleImageData::z(int indexX, int indexY) const {

	return d_[indexY][indexX];
}

// Return the number of elements in x and y
QPoint MPlotSimpleImageData::count() const {
	return num_;
}

// Return the bounds of the data (the rectangle containing the max/min x- and y-values)
/* Use the top left corner for the (minX,minY) values, ie: boundingRect() == QRectF(minX, minY, maxX-minX, maxY-minY)... so that boundingRect().width() == maxX-minX, and boundingRect().height() == maxY - minY.
	  */
QRectF MPlotSimpleImageData::boundingRect() const {
	return bounds_;
}

// Return the minimum and maximum z values:
MPlotInterval MPlotSimpleImageData::range() const {

	if(minIndex_.x() < 0)
		minSearch();
	if(maxIndex_.x() < 0)
		maxSearch();

	return MPlotInterval(d_[minIndex_.y()][minIndex_.x()], d_[maxIndex_.y()][maxIndex_.x()]);
}

// set the z value at \c index:
void MPlotSimpleImageData::setZ(qreal value, int indexX, int indexY) {

	// if we're modifying what used to be the maximum value, and this new one is smaller, we've lost our max tracking. Don't know anymore.
	if((int)indexX == maxIndex_.x() && (int)indexY == maxIndex_.y() && value < d_[maxIndex_.y()][maxIndex_.x()])
		maxIndex_ = QPoint(-1,-1);

	// if we're modifying what used to be the minimum value, and this new one is larger, we've lost our min tracking. Don't know anymore.
	if((int)indexX == minIndex_.x() && (int)indexY == minIndex_.y() && value > d_[minIndex_.y()][minIndex_.x()])
		minIndex_ = QPoint(-1, -1);

	// if we're tracking the min index, and this new value is smaller, it becomes the new min.
	if(minIndex_.x()>=0 && value < d_[minIndex_.y()][minIndex_.x()])
		minIndex_ = QPoint(indexX, indexY);

	// if we're tracking the max index, and this new value is larger, it becomes the new max.
	if(maxIndex_.x()>=0 && value > d_[maxIndex_.y()][maxIndex_.x()])
		maxIndex_ = QPoint(indexX, indexY);

	// store value:
	d_[indexY][indexX] = value;
	emitDataChanged();

}

// manually search for minimum value
void MPlotSimpleImageData::minSearch() const {
	minIndex_ = QPoint(0,0);

	for(int yy=0; yy<num_.y(); yy++)
		for(int xx=0; xx<num_.x(); xx++)
			if(d_[yy][xx] < d_[minIndex_.y()][minIndex_.x()])
				minIndex_ = QPoint(xx, yy);
}

// manually search for maximum value
void MPlotSimpleImageData::maxSearch() const {
	maxIndex_ = QPoint(0,0);

	for(int yy=0; yy<num_.y(); yy++)
		for(int xx=0; xx<num_.x(); xx++)
			if(d_[yy][xx] > d_[maxIndex_.y()][maxIndex_.x()])
				maxIndex_ = QPoint(xx, yy);
}

void MPlotSimpleImageData::zValues(int xStart, int yStart, int xEnd, int yEnd, qreal *outputValues) const
{
	for(int xx=xStart; xx<=xEnd; ++xx)
		for(int yy=yStart; yy<=yEnd; ++yy)
			*(outputValues++) = d_[yy][xx];
}

#endif // MPLOTIMAGEDATA_H
