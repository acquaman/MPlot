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
	MPlotAbstractImageData();

	~MPlotAbstractImageData();

	/// Return the x (data value) corresponding an (x,y) \c index:
	virtual double x(unsigned indexX) const = 0;
	/// Return the y (data value) corresponding an (x,y) \c index:
	virtual double y(unsigned indexY) const = 0;
	/// Return the z = f(x,y) value corresponding an (x,y) \c index:
	virtual double z(unsigned xIndex, unsigned yIndex) const = 0;

	/// Convenience function overloads:
	virtual double x(const QPoint& index) const;
	virtual double y(const QPoint& index) const;
	virtual double z(const QPoint& index) const;
	virtual double value(const QPoint& index) const;
	virtual double value(unsigned xIndex, unsigned yIndex) const;


	/// Set the z value at \c index
	virtual void setZ(double value, unsigned xIndex, unsigned yIndex) = 0;

	/// Convenience function overloads:
	virtual void setZ(double value, const QPoint& index);
	virtual void setValue(double value, const QPoint& index);


	/// Return the number of elements in x and y
	virtual QPoint count() const = 0;
	/// Identical to count(), but returning the information as a QSize instead of QPoint
	virtual QSize size() const;


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
	MPlotSimpleImageData(const QRectF& dataBounds, const QSize& resolution);


	/// Return the x (data value) corresponding an (x,y) \c index:
	virtual double x(unsigned indexX) const;
	/// Return the y (data value) corresponding an (x,y) \c index:
	virtual double y(unsigned indexY) const;

	/// Return the z = f(x,y) value corresponding an (x,y) \c index:
	virtual double z(unsigned indexX, unsigned indexY) const;

	/// Return the number of elements in x and y
	virtual QPoint count() const;

	/// Return the bounds of the data (the rectangle containing the max/min x- and y-values)
	/*! Use the top left corner for the (minX,minY) values, ie: boundingRect() == QRectF(minX, minY, maxX-minX, maxY-minY)... so that boundingRect().width() == maxX-minX, and boundingRect().height() == maxY - minY.
	  */
	virtual QRectF boundingRect() const;
	/// Return the minimum and maximum z values:
	virtual MPlotInterval range() const;


	/// set the z value at \c index:
	virtual void setZ(double value, unsigned indexX, unsigned indexY);

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
	void minSearch() const;

	/// manually search for maximum value
	void maxSearch() const;

};

#endif // MPLOTIMAGEDATA_H
