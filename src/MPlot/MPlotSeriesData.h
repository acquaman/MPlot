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
	MPlotAbstractSeriesData() : MPlotObservable() {}
	
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
	MPlotRealtimeModel(QObject *parent = 0) : QAbstractTableModel(parent), MPlotAbstractSeriesData(), xName_("x"), yName_("y") {
		
		// min/max tracking indices are invalid at start (empty model)
		minYIndex_ = maxYIndex_ = minXIndex_ = maxXIndex_ = -1;
		
		// Axis names: initialized on first line to "x", "y" (Real original... I know.)
		
	}
	
	int rowCount(const QModelIndex & /*parent*/) const { return xval_.count(); }
	virtual unsigned count() const { return xval_.count(); }
	int columnCount(const QModelIndex & /*parent*/) const { return 2; }
	
	virtual double x(unsigned index) const { if(index<(unsigned)xval_.count()) return xval_.at(index); else return 0.0; }
	virtual double y(unsigned index) const { if(index<(unsigned)yval_.count()) return yval_.at(index); else return 0.0; }
	
	
	QVariant data(const QModelIndex &index, int role) const {
		
		// Invalid index:
		if(!index.isValid())
			return QVariant();
		
		// We only answer to Qt::DisplayRole right now
		if(role != Qt::DisplayRole)
			return QVariant();
		
		// Out of range: (Just checking for too big.  isValid() checked for < 0)
		if(index.row() >= xval_.count())
			return QVariant();
		
		// Return x-val:
		if(index.column() == 0)
			return xval_.at(index.row());
		// Return y-val:
		if(index.column() == 1)
			return yval_.at(index.row());
		
		// Anything else:
		return QVariant();		
	}
	
	QVariant headerData(int section, Qt::Orientation orientation, int role) const {
		
		if (role != Qt::DisplayRole) return QVariant();
		
		// Vertical headers:
		if(orientation == Qt::Vertical) {
			return section;
		}
		
		// Horizontal Headers: (Column labels)		
		else {
			if(section == 0)
				return xName_;
			if(section == 1)
				return yName_;
		}
		return QVariant();
	}
	
	bool setData(const QModelIndex &index, const QVariant &value, int role) {
		
		if (index.isValid()  && index.row() < xval_.count() && role == Qt::EditRole) {
			
			bool conversionOK;
			double dval = value.toDouble(&conversionOK);
			if(!conversionOK)
				return false;
			
			// Setting an x value?
			if(index.column() == 0) {
				minMaxChangeCheckX(dval, index.row());
				emit QAbstractItemModel::dataChanged(index, index);
				Emit(0, "dataChanged");
				return true;
			}
			// Setting a y value?
			if(index.column() == 1) {
				minMaxChangeCheckY(dval, index.row());
				emit QAbstractItemModel::dataChanged(index, index);
				Emit(0, "dataChanged");
				return true;
			}
		}
		return false;	// no value set
	}
	
	// This allows editing of values within range (for ex: in a QTableView)
	Qt::ItemFlags flags(const QModelIndex &index) const {
		
		Qt::ItemFlags flags;
		if (index.isValid() && index.row() < xval_.count() && index.column()<2)
			flags = Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled;
		return flags;
	}
	
	// This allows you to add data points at the beginning:
	void insertPointFront(double x, double y) {
		beginInsertRows(QModelIndex(), 0, 0);
		
		xval_.prepend(x);
		yval_.prepend(y);
		
		// Update the max/min index trackers:
		minXIndex_++;
		minYIndex_++;
		maxXIndex_++;
		maxYIndex_++;
		
		// Check if this guy is a new min or max:
		minMaxAddCheck(x, y, 0);
		
		endInsertRows();
		
		// Signal a full-plot update
		Emit(0, "dataChanged");
	}
	
	// This allows you to add data points at the end:
	void insertPointBack(double x, double y) {
		beginInsertRows(QModelIndex(), xval_.count(), xval_.count());
		
		xval_.append(x);
		yval_.append(y);
		
		minMaxAddCheck(x, y, xval_.count()-1);		
		
		endInsertRows();
		// Signal a full-plot update
		Emit(0, "dataChanged");
	}
	
	// Remove a point at the front (Returns true if successful).
	bool removePointFront() {
		if(xval_.isEmpty())
			return false;
		
		beginRemoveRows(QModelIndex(), 0, 0);
		
		xval_.takeFirst();
		yval_.takeFirst();
		
		// Update the max/min index trackers:
		minXIndex_--;
		minYIndex_--;
		maxXIndex_--;
		maxYIndex_--;
		
		// Did we remove the current max/min?
		if(minYIndex_ == -1)
			minYIndex_ = searchMinIndex(yval_);
		if(maxYIndex_ == -1)
			maxYIndex_ = searchMaxIndex(yval_);
		if(minXIndex_ == -1)
			minXIndex_ = searchMinIndex(xval_);
		if(maxXIndex_ == -1)
			maxXIndex_ = searchMaxIndex(xval_);
		
		
		endRemoveRows();
		
		// Signal a full-plot update
		Emit(0, "dataChanged");
		return true;
	}
	
	// Remove a point at the back (returns true if successful)
	bool removePointBack() {
		if(xval_.isEmpty())
			return false;
		
		beginRemoveRows(QModelIndex(), xval_.count()-1, xval_.count()-1);
		
		xval_.takeLast();
		yval_.takeLast();
		
		// Did we remove the current max/min?
		int oldIndexToCheck = xval_.count();
		if(minYIndex_ == oldIndexToCheck)
			minYIndex_ = searchMinIndex(yval_);
		if(maxYIndex_ == oldIndexToCheck)
			maxYIndex_ = searchMaxIndex(yval_);
		if(minXIndex_ == oldIndexToCheck)
			minXIndex_ = searchMinIndex(xval_);
		if(maxXIndex_ == oldIndexToCheck)
			maxXIndex_ = searchMaxIndex(xval_);
		
		endRemoveRows();
		
		// Signal a full-plot update
		Emit(0, "dataChanged");
		return true;
	}
	
	virtual QRectF boundingRect() const {
		if(xval_.isEmpty() || yval_.isEmpty())
			return QRectF();	// No data... return an invalid QRectF
		
		return QRectF(minX(), minY(), maxX()-minX(), maxY()-minY());
		
	}
	
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
	void minMaxAddCheck(double x, double y, int index) {
		
		// in case of previously-empty list (min_Index_ initialized to -1)
		if(xval_.count() == 1) {
			minYIndex_ = index;
			maxYIndex_ = index;
			minXIndex_ = index;
			maxXIndex_ = index;
			return;
		}
		
		// New x max found:		
		if(x > xval_.at(maxXIndex_))
			maxXIndex_ = index;
		// New x min found:
		if(x < xval_.at(minXIndex_))
			minXIndex_ = index;
		// New y max found:
		if(y > yval_.at(maxYIndex_))
			maxYIndex_ = index;
		if(y < yval_.at(minYIndex_))
			minYIndex_ = index;
		
	}
	
	// Check if a point modified at index causes us to lose our record holders, or is a new record holder.
	// Inserts the point (modifies the data array).
	void minMaxChangeCheckX(double newVal, int index) {
		
		double oldVal = xval_.at(index);
		xval_[index] = newVal;
		
		// Maybe not the max anymore... Need to find the new one:
		if(index == maxXIndex_ && newVal < oldVal)
			maxXIndex_ = searchMaxIndex(xval_);
		
		// Maybe not the min anymore... Need to find the new one:
		if(index == minXIndex_ && newVal > oldVal)
			minXIndex_ = searchMinIndex(xval_);
		
		if(newVal > xval_.at(maxXIndex_))
			maxXIndex_ = index;
		
		if(newVal < xval_.at(minXIndex_))
			minXIndex_ = index;
		
		
	}
	void minMaxChangeCheckY(double newVal, int index) {
		
		double oldVal = yval_.at(index);
		yval_[index] = newVal;
		
		// Maybe not the max anymore... Need to find the new one:
		if(index == maxYIndex_ && newVal < oldVal)
			maxYIndex_ = searchMaxIndex(yval_);
		
		// Maybe not the min anymore... Need to find the new one:
		if(index == minYIndex_ && newVal > oldVal)
			minYIndex_ = searchMinIndex(yval_);
		
		if(newVal > yval_.at(maxYIndex_))
			maxYIndex_ = index;
		
		if(newVal < yval_.at(minYIndex_))
			minYIndex_ = index;
		
		
	}
	
	int searchMaxIndex(const QList<double>& list) {
		if(list.isEmpty())
			return -1;
		
		int mi = 0;	// max index
		for(int i=1; i<list.count(); i++)
			if(list.at(i) > list.at(mi))
				mi = i;
		
		return mi;
	}
	
	int searchMinIndex(const QList<double>& list) {
		if(list.isEmpty())
			return -1;
		
		int mi = 0;	// min index
		for(int i=1; i<list.count(); i++)
			if(list.at(i) < list.at(mi))
				mi = i;
		
		return mi;
	}
	
	// Warning: only call these if the list is not empty:
	double minY() const { return yval_.at(minYIndex_); }
	double maxY() const { return yval_.at(maxYIndex_); }
	double minX() const { return xval_.at(minXIndex_); }
	double maxX() const { return xval_.at(maxXIndex_); }
	
};





#endif
