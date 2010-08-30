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
}

MPlotAbstractImageData::~MPlotAbstractImageData()
{
	delete signalSource_;
	signalSource_ = 0;
}

/// Convenience function overloads:
double MPlotAbstractImageData::x(const QPoint& index) const {
	return x(index.x());
}

double MPlotAbstractImageData::y(const QPoint& index) const {
	return y(index.y());
}

double MPlotAbstractImageData::z(const QPoint& index) const {
	return z(index.x(), index.y());
}

double MPlotAbstractImageData::value(const QPoint& index) const {
	return z(index);
}

double MPlotAbstractImageData::value(unsigned xIndex, unsigned yIndex) const {
	return z(xIndex, yIndex);
}

/// Convenience function overloads:
void MPlotAbstractImageData::setZ(double value, const QPoint& index) {
	setZ(value, index.x(), index.y() );
}

void MPlotAbstractImageData::setValue(double value, const QPoint& index) {
	setZ(value, index);
}

/// Identical to count(), but returning the information as a QSize instead of QPoint
QSize MPlotAbstractImageData::size() const { return QSize(count().x(), count().y()); }


/// This class is a very basic 2D array which implements the MPlotAbstractImageData interface

/// Constructor: represent image data with physical coordinate boundaries \c dataBounds, and a resolution (number of "pixels") \c resolution.  Data values are initialized to 0.
MPlotSimpleImageData::MPlotSimpleImageData(const QRectF& dataBounds, const QSize& resolution)
	: MPlotAbstractImageData(),
	num_(resolution.expandedTo(QSize(1,1)).width(), resolution.expandedTo(QSize(1,1)).height()),
	d_(num_.y(), QVector<double>(num_.x(), 0)),
	bounds_(dataBounds)
{
	// max and min trackers are valid from the beginning; every data value is 0, so we might as well use z(0,0) = max = min.
	minIndex_ = QPoint(0,0);
	maxIndex_ = QPoint(0,0);

}


/// Return the x (data value) corresponding an (x,y) \c index:
double MPlotSimpleImageData::x(unsigned indexX) const  {

	return bounds_.left() + bounds_.width()*indexX/num_.x();

}
/// Return the y (data value) corresponding an (x,y) \c index:
double MPlotSimpleImageData::y(unsigned indexY) const {

	return bounds_.top() + bounds_.height()*indexY/num_.y();
}

/// Return the z = f(x,y) value corresponding an (x,y) \c index:
double MPlotSimpleImageData::z(unsigned indexX, unsigned indexY) const {

	if((int)indexX>=num_.x() || (int)indexY>=num_.y())
		return -1.;

	return d_[indexY][indexX];
}

/// Return the number of elements in x and y
QPoint MPlotSimpleImageData::count() const {
	return num_;
}


/// Return the bounds of the data (the rectangle containing the max/min x- and y-values)
/*! Use the top left corner for the (minX,minY) values, ie: boundingRect() == QRectF(minX, minY, maxX-minX, maxY-minY)... so that boundingRect().width() == maxX-minX, and boundingRect().height() == maxY - minY.
	  */
QRectF MPlotSimpleImageData::boundingRect() const {
	return bounds_;
}
/// Return the minimum and maximum z values:
MPlotInterval MPlotSimpleImageData::range() const {

	if(minIndex_.x() < 0)
		minSearch();
	if(maxIndex_.x() < 0)
		maxSearch();

	return MPlotInterval(d_[minIndex_.y()][minIndex_.x()], d_[maxIndex_.y()][maxIndex_.x()]);
}


/// set the z value at \c index:
void MPlotSimpleImageData::setZ(double value, unsigned indexX, unsigned indexY) {

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



/// manually search for minimum value
void MPlotSimpleImageData::minSearch() const {
	minIndex_ = QPoint(0,0);

	for(int yy=0; yy<num_.y(); yy++)
		for(int xx=0; xx<num_.x(); xx++)
			if(d_[yy][xx] < d_[minIndex_.y()][minIndex_.x()])
				minIndex_ = QPoint(xx, yy);
}

/// manually search for maximum value
void MPlotSimpleImageData::maxSearch() const {
	maxIndex_ = QPoint(0,0);

	for(int yy=0; yy<num_.y(); yy++)
		for(int xx=0; xx<num_.x(); xx++)
			if(d_[yy][xx] > d_[maxIndex_.y()][maxIndex_.x()])
				maxIndex_ = QPoint(xx, yy);
}


#endif // MPLOTIMAGEDATA_H

