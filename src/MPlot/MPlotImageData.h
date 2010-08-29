#ifndef MPLOTIMAGEDATA_H
#define MPLOTIMAGEDATA_H

#include <QObject>
#include <QPoint>
#include <QRectF>
#include <QPair>
#include <QVector>
#include "MPlotObservable.h"

/// An MPlotInterval is just a typedef for a pair of doubles
typedef QPair<double,double> MPlotInterval;

/// this class defines the interface to represent 3D data z = f(x,y), used by image plots and contour plots.


  // copied=pasted from MPlotSeriesData. Fix.
  // todo: figure out resolution question. Data sets resolution? plot sets resoution>?
class MPlotAbstractImageData : public MPlotObservable {

public:
	MPlotAbstractImageData() : MPlotObservable() {}

	~MPlotAbstractImageData() {}

	/// Return the x (data value) corresponding an (x,y) \c index:
	virtual double x(unsigned indexX) const = 0;
	/// Return the y (data value) corresponding an (x,y) \c index:
	virtual double y(unsigned indexY) const = 0;
	/// Return the z = f(x,y) value corresponding an (x,y) \c index:
	virtual double z(unsigned xIndex, unsigned yIndex) const = 0;

	/// Convenience function overloads:
	virtual double x(const QPoint& index) const { return x(index.x()); }
	virtual double y(const QPoint& index) const { return y(index.y()); }
	virtual double z(const QPoint& index) const { return z(index.x(), index.y()); }
	virtual double value(const QPoint& index) const { return z(index); }
	virtual double value(unsigned xIndex, unsigned yIndex) const { return z(xIndex, yIndex); }


	/// Set the z value at \c index
	virtual void setZ(double value, unsigned xIndex, unsigned yIndex) = 0;

	/// Convenience function overloads:
	virtual void setZ(double value, const QPoint& index) { setZ(value, index.x(), index.y() ); }
	virtual void setValue(double value, const QPoint& index) { setZ(value, index); }


	/// Return the number of elements in x and y
	virtual QPoint count() const = 0;
	/// Identical to count(), but returning the information as a QSize instead of QPoint
	virtual QSize size() const { return QSize(count().x(), count().y()); }


	/// Return the bounds of the data (the rectangle containing the max/min x- and y-values)
	virtual QRectF boundingRect() const = 0;
	/// Return the minimum and maximum z values:
	virtual MPlotInterval range() const = 0;

/// Signals: Implements MPlotObservable. Will Emit(0, "dataChanged") when the z-data has been changed.  Will Emit(1, "boundsChanged") when the x- or y-limits have been changed and the plot scaling might need to be recalculated.


	// todo: to support multi-threading, consider a
	// void pauseUpdates();	// to tell nothing to redraw using the plot because the data is currently invalid; a dataChanged will be emitted when it is valid again.
	// This would need to be deterministic, so maybe we need to use function calls instead of signals.
};

/// This class is a very basic 2D array which implements the MPlotAbstractImageData interface
class MPlotSimpleImageData : public MPlotAbstractImageData {

public:
	/// Constructor: represent image data with physical coordinate boundaries \c dataBounds, and a resolution (number of "pixels") \c resolution.  Data values are initialized to 0.
	MPlotSimpleImageData(const QRectF& dataBounds, const QSize& resolution)
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
	virtual double x(unsigned indexX) const  {

		return bounds_.left() + bounds_.width()*indexX/num_.x();

	}
	/// Return the y (data value) corresponding an (x,y) \c index:
	virtual double y(unsigned indexY) const {

		return bounds_.top() + bounds_.height()*indexY/num_.y();
	}

	/// Return the z = f(x,y) value corresponding an (x,y) \c index:
	virtual double z(unsigned indexX, unsigned indexY) const {

		if((int)indexX>=num_.x() || (int)indexY>=num_.y())
			return -1.;

		return d_[indexY][indexX];
	}

	/// Return the number of elements in x and y
	virtual QPoint count() const {
		return num_;
	}


	/// Return the bounds of the data (the rectangle containing the max/min x- and y-values)
	/*! Use the top left corner for the (minX,minY) values, ie: boundingRect() == QRectF(minX, minY, maxX-minX, maxY-minY)... so that boundingRect().width() == maxX-minX, and boundingRect().height() == maxY - minY.
	  */
	virtual QRectF boundingRect() const {
		return bounds_;
	}
	/// Return the minimum and maximum z values:
	virtual MPlotInterval range() const {

		if(minIndex_.x() < 0)
			minSearch();
		if(maxIndex_.x() < 0)
			maxSearch();

		return MPlotInterval(d_[minIndex_.y()][minIndex_.x()], d_[maxIndex_.y()][maxIndex_.x()]);
	}


	/// set the z value at \c index:
	virtual void setZ(double value, unsigned indexX, unsigned indexY) {

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
		Emit(0, "dataChanged");

	}





protected:
	/// resolution: number of values in x and y
	QPoint num_;
	/// Stores raw data. (row-major: d_.count() == numRows)
	QVector<QVector< double > > d_;
	/// the (min/max) (x/y) values, in physical(data) coordinates. bounds_.upperLeft is == (minX, minY)
	QRectF bounds_;

	/// minimum and maximum z data values:
	mutable QPoint minIndex_, maxIndex_;


	/// manually search for minimum value
	void minSearch() const {
		minIndex_ = QPoint(0,0);

		for(int yy=0; yy<num_.y(); yy++)
			for(int xx=0; xx<num_.x(); xx++)
				if(d_[yy][xx] < d_[minIndex_.y()][minIndex_.x()])
					minIndex_ = QPoint(xx, yy);
	}

	/// manually search for maximum value
	void maxSearch() const {
		maxIndex_ = QPoint(0,0);

		for(int yy=0; yy<num_.y(); yy++)
			for(int xx=0; xx<num_.x(); xx++)
				if(d_[yy][xx] > d_[maxIndex_.y()][maxIndex_.x()])
					maxIndex_ = QPoint(xx, yy);
	}

};

#endif // MPLOTIMAGEDATA_H
