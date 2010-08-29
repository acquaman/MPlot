#ifndef __MPlotSeriesData_H__
#define __MPlotSeriesData_H__

#include <QAbstractTableModel>
#include <QQueue>
#include <QList>
#include <QRectF>
#include "MPlotObservable.h"

// This defines the interface for classes which may be used for Series (XY) plot data.
// Unfortunately, because QObject doesn't support multiple inheritance, if you want your data object to inherit from another QObject-derived class, wrapper classes are needed as shown in the example below.
class MPlotAbstractSeriesData : public MPlotObservable {

public:
	MPlotAbstractSeriesData();
	~MPlotAbstractSeriesData();

	// Return the x-value (y-value) at index:
	virtual double x(unsigned index) const = 0;
	virtual double y(unsigned index) const = 0;

	// Return the number of elements
	virtual unsigned count() const = 0;


	// Return the bounds of the data (the rectangle containing the max/min x- and y-values)
	virtual QRectF boundingRect() const = 0;

	// Implements MPlotObservable and will Emit(0, "dataChanged") when the data changes and a re-scale + re-plot is required.


	// todo: to support multi-threading, consider a
	// void pauseUpdates();	// to tell nothing to redraw using the plot because the data is currently invalid; a dataChanged will be emitted when it is valid again.
							// This would need to be deterministic, so maybe we need to use function calls instead of signals.
};




/// This class provides a Qt TableModel implementation of XY data.  It is optimized for fast storage of real-time data.
/*! It provides fast (usually constant-time) lookups of the min and max values for each axis, which is important for plotting so that
	// boundingRect() and autoscaling calls run quickly.
// When using for real-time data, calling insertPointFront and insertPointBack is very fast.
// This class implements all the functions of the MPlotAbstractSeriesData interface.  However, if we told the compiler that, it would
// imply multiple inheritance of QObject, which is not allowed. To pass this class to plot->setModel(), you must use wrapper-class MPlotRealtimeModelSeriesData
  */
class MPlotRealtimeModel : public QAbstractTableModel, public MPlotAbstractSeriesData {

	Q_OBJECT

public:
	MPlotRealtimeModel(QObject *parent = 0);

	int rowCount(const QModelIndex & /*parent*/) const;
	virtual unsigned count() const;
	int columnCount(const QModelIndex & /*parent*/) const;

	virtual double x(unsigned index) const;
	virtual double y(unsigned index) const;


	QVariant data(const QModelIndex &index, int role) const;

	QVariant headerData(int section, Qt::Orientation orientation, int role) const;

	bool setData(const QModelIndex &index, const QVariant &value, int role);

	// This allows editing of values within range (for ex: in a QTableView)
	Qt::ItemFlags flags(const QModelIndex &index) const;

	// This allows you to add data points at the beginning:
	void insertPointFront(double x, double y);

	// This allows you to add data points at the end:
	void insertPointBack(double x, double y);

	// Remove a point at the front (Returns true if successful).
	bool removePointFront();

	// Remove a point at the back (returns true if successful)
	bool removePointBack();

	virtual QRectF boundingRect() const;

	// TODO: add properties: set and read axis names

	// implements MPlotObservable, and will Emit(0, "dataChanged") when x- or y- data changes.

protected:

	// Members: Data arrays:
	QQueue<double> xval_;
	QQueue<double> yval_;

	// Max/min index tracking:
	int minYIndex_, maxYIndex_, minXIndex_, maxXIndex_;

	//
	QString xName_, yName_;


	// Helper functions:
	// Check if an added point @ index is the new min. or max record holder:
	// Must call this AFTER adding both x and y to the xval_ and y_val lists.
	void minMaxAddCheck(double x, double y, int index);

	// Check if a point modified at index causes us to lose our record holders, or is a new record holder.
	// Inserts the point (modifies the data array).
	void minMaxChangeCheckX(double newVal, int index);
	void minMaxChangeCheckY(double newVal, int index);

	int searchMaxIndex(const QList<double>& list);

	int searchMinIndex(const QList<double>& list);

	// Warning: only call these if the list is not empty:
	double minY() const;
	double maxY() const;
	double minX() const;
	double maxX() const;

};





#endif
