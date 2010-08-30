#ifndef MPLOTIMAGEDATA_H
#define MPLOTIMAGEDATA_H

#include <QObject>
#include <QPoint>
#include <QRectF>
#include <QPair>
#include <QVector>


/// An MPlotInterval is just a typedef for a pair of doubles
typedef QPair<double,double> MPlotInterval;

class MPlotAbstractImageData;


/// This class acts as a proxy to emit signals for MPlotAbstractImageData. You can receive the dataChanged() signal by hooking up to MPlotAbstractImage::signalSource().
/*! To allow classes that implement MPlotAbstractImageData to also inherit QObject, MPlotAbstractImageData does NOT inherit QObject.  However, it still needs a way to emit signals notifying of changes to the data, which is the role of this class.
  */
class MPlotImageDataSignalSource : public QObject {
	Q_OBJECT
public:
	MPlotAbstractImageData* imageData() const { return data_; }
protected:
	MPlotImageDataSignalSource(MPlotAbstractImageData* parent);
	void emitDataChanged() { emit dataChanged(); }
	void emitBoundsChanged() { emit boundsChanged(); }

	MPlotAbstractImageData* data_;
	friend class MPlotAbstractImageData;

signals:
	void dataChanged();	/// < the z = f(x,y) data has changed
	void boundsChanged();/// < The limits / bounds of the x-y grid have changed
};


/// this class defines the interface to represent 3D data z = f(x,y), used by image plots and contour plots.
  // copied=pasted from MPlotSeriesData. Fix.
  // todo: figure out resolution question. Data sets resolution? plot sets resoution>?
class MPlotAbstractImageData {

public:
	MPlotAbstractImageData();

	~MPlotAbstractImageData();

	/// Use this proxy object to receive dataChanged() and boundsChanged() signals from the data
	MPlotImageDataSignalSource* signalSource() const { return signalSource_; }

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

protected:
	/// Proxy object for emitting signals:
	MPlotImageDataSignalSource* signalSource_;
	friend class MPlotImageDataSignalSource;

	/// Implementing classes should call this when their z- data changes in value
	void emitDataChanged() { signalSource_->emitDataChanged(); }
	/// Implementing classes should call this when their x- y- data changes in extent
	void emitBoundsChanged() { signalSource_->emitBoundsChanged(); }



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
