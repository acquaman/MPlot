#ifndef __MPlotSeries_H__
#define __MPlotSeries_H__

#include <QGraphicsObject>
#include <QGraphicsLineItem>
#include <QQueue>
#include <QPen>
#include <QBrush>
#include "MPlotMarker.h"
#include "MPlotAxis.h"
#include "MPlotSeriesData.h"


// TODO:
// Put markers on top of plot lines...
// Optimize placeAllMarkers to not set Brush

class MPlotSeries : public QGraphicsObject {
	
	Q_OBJECT

public:
	
	MPlotSeries(const MPlotSeriesData* data = 0, QGraphicsItem* parent = 0) : QGraphicsObject(parent) {
		
		data_ = 0;
		// setFlag(QGraphicsItem::ItemIsSelectable, true);	// TODO: figure this out
		
		// Set style defaults:
		setDefaults();
		
		// Set model (will check that data != 0)
		setModel(data);
		
	}
	
	
	
	// Properties:
	void setLinePen(const QPen& pen) { linePen_ = pen; linePen_.setCosmetic(true); placeAllLines(); }
	void setMarkerPen(const QPen& pen) { markerPen_ = pen; markerPen_.setCosmetic(true); placeAllMarkers(); }
	void setMarkerBrush(const QBrush& brush) { markerBrush_ = brush; placeAllMarkers(); }
	void setMarkerShape(MPlotMarkerShape::Shape shape) {
		markerShape_ = shape; 
		if(data_) {
			createMarkers(0);
			createMarkers(data_->count());
			placeAllMarkers();
		}
	}
	
	void setMarkerSize(double size) {
		
		markerSize_ = size;
		foreach(MPlotAbstractMarker* marker, markers_) {
			marker->setSize(size);
		}

		// Long way:
		/*
		if(data_) {
			createMarkers(0);
			createMarkers(data_->count());
			placeAllMarkers();
		}
		*/
		
	}
	
	MPlotAxis::AxisID yAxisTarget() { return yAxisTarget_;}
	void setYAxisTarget(MPlotAxis::AxisID axis) { yAxisTarget_ = axis; }
	
	// Sets this series to view the model in 'data';
	void setModel(const MPlotSeriesData* data) {
		
		// Clearing a model:
		if(data == 0) {
			if(data_)
				disconnect(data_, 0, this, 0);
			data_ = 0;
			createLines(0);
			createMarkers(0);
			return;
		}
		
		// Setting a new model:
		// If there was an old model, disconnect old signals:
		if(data_)
			disconnect(data_, 0, this, 0);
		
		// New model from here:
		data_ = data;
		
		// add initial data point (lines and markers)	
		int numLines = (data_->count() > 1) ? data_->count() - 1 : 0;
		createLines(numLines);
		placeAllLines();
		
		createMarkers(data_->count());
		placeAllMarkers();
		
		// Connect model signals to slots: rowsInserted(), rowsRemoved(), dataChanged()
		connect(data_, SIGNAL(rowsInserted(const QModelIndex &, int, int)), this, SLOT(handleRowsInserted( const QModelIndex &, int, int )));
		connect(data_, SIGNAL(rowsRemoved(const QModelIndex &, int, int)), this, SLOT(handleRowsRemoved( const QModelIndex &, int, int )));
		connect(data_, SIGNAL(dataChanged( const QModelIndex &, const QModelIndex & )), this, SLOT(handleDataChanged( const QModelIndex &, const QModelIndex & )));
		
		emit dataChanged(this);
	}
	
	
	// Required functions:
	//////////////////////////
	// Bounding rect: reported in our PlotSeries coordinates, which are just the actual data coordinates.
	virtual QRectF boundingRect() const { if(data_) return data_->boundingRect(); else return QRectF(); }
	// Paint:
	virtual void paint(QPainter* /*painter*/,
					   const QStyleOptionGraphicsItem* /*option*/,
					   QWidget* /*widget*/) {
		// Do nothing... drawn with children
	}
	

signals:
	
	void dataChanged(MPlotSeries* series);	// listen to this if you want to auto-scale on changes.
	
protected slots:
	void handleRowsInserted( const QModelIndex & /*parent*/, int start, int end ) {
		
		// This signal should only be delivered by a valid data_ model:
		
		// Handle single insert at beginning: (this is fast)
		if(start == 0 && end == 0) {
			
			if(data_->count() > 1) {
				QGraphicsLineItem* newline = new QGraphicsLineItem(this);
				lines_.prepend(newline);
				newline->setLine(data_->x(0), data_->y(0), data_->x(1), data_->y(1));
				newline->setPen(linePen_);
			}
			
			MPlotAbstractMarker* newmarker = MPlotMarker::create(markerShape_, this, markerSize_);
			markers_.prepend( newmarker );
			newmarker->setPos(data_->x(0), data_->y(0));
			newmarker->setPen(markerPen_);
			newmarker->setBrush(markerBrush_);			
		}
		
		// Handle single insert at end: (this is also pretty fast)
		// Note: if this is the first item, we've already handled it above. This won't run.
		else if(start == data_->count()-1 && end == data_->count()-1) {
			int insertIndex = data_->count()-1;
			
			QGraphicsLineItem* newline = new QGraphicsLineItem(this);
			lines_.append(newline);
			newline->setLine(data_->x(insertIndex-1), data_->y(insertIndex-1), data_->x(insertIndex), data_->y(insertIndex));
			newline->setPen(linePen_);
			
			MPlotAbstractMarker* newmarker = MPlotMarker::create(markerShape_, this, markerSize_);
			markers_.append( newmarker );
			newmarker->setPos(data_->x(insertIndex), data_->y(insertIndex));
			newmarker->setPen(markerPen_);
			newmarker->setBrush(markerBrush_);
		}
		
		// Otherwise (insert in the middle, etc.) do a complete re-do of all the lines and markers:
		else {
			createLines(data_->count()-1);
			placeAllLines();
			createMarkers(data_->count());
			placeAllMarkers();
		}
		
		emit dataChanged(this);

	}
	
	void handleRowsRemoved ( const QModelIndex & /*parent*/, int start, int end ) {
		
		// This signal should only be delivered by a valid data_ model:
		
		// Handle single removal at beginning: (this is fast)
		if(start == 0 && end == 0) {
			if(!markers_.isEmpty())
				delete markers_.takeFirst();
			if(!lines_.isEmpty())
				delete lines_.takeFirst();			
		}
		
		// Handle single removal at end: (this is also pretty fast)
		else if(start == data_->count() && end == data_->count()) {
			if(!markers_.isEmpty())
				delete markers_.takeLast();
			if(!lines_.isEmpty())
				delete lines_.takeLast();
		}
		
		// Otherwise (removal in the middle, etc.) do a complete re-do of all the lines and markers:
		else {
			createLines(data_->count()-1);
			placeAllLines();
			createMarkers(data_->count());
			placeAllMarkers();
		}
		
		emit dataChanged(this);
	}
	
	void handleDataChanged( const QModelIndex & topLeft, const QModelIndex & bottomRight ) {
		
		// This signal should only be delivered by a valid data_ model:
		for(int i = topLeft.row(); i <= bottomRight.row(); i++) {
			markers_[i]->setPos(data_->x(i), data_->y(i));
			if(i>0)
				lines_[i-1]->setLine(QLineF(data_->x(i-1), data_->y(i-1), data_->x(i), data_->y(i)));
			if(i<lines_.count())
				lines_[i]->setLine(QLineF(data_->x(i), data_->y(i), data_->x(i+1), data_->y(i+1)));
		}
		emit dataChanged(this);
	}
	
protected:
	QPen linePen_, markerPen_;
	QBrush markerBrush_;
	MPlotMarkerShape::Shape markerShape_;
	double markerSize_;
	
	const MPlotSeriesData* data_;
	QQueue<MPlotAbstractMarker*> markers_;
	QQueue<QGraphicsLineItem*> lines_;
	
	MPlotAxis::AxisID yAxisTarget_;
	
	void setDefaults() {
		
		yAxisTarget_ = MPlotAxis::Left;
		
		setLinePen(QPen(QColor(Qt::red)));	// Red solid lines on plot
		setMarkerPen(QPen(QColor(Qt::blue))); // Blue outlines on markers
		setMarkerBrush(QBrush());	// default: NoBrush
		
		markerShape_ = MPlotMarkerShape::Square;
		markerSize_ = 6.0;
		
	}
	
	void createMarkers(int num) {
		
		// remove extra markers
		while(markers_.count() > num) {
			delete markers_.takeFirst();
		}
		
		// create additional markers:
		while(markers_.count() < num) {
			markers_ << MPlotMarker::create(markerShape_, this, markerSize_);
		}
	}
	
	// Important: need to make sure first that number of markers matches data_->count()!
	void placeAllMarkers() {
		
		if(!data_)
			return;
		
		for(int i=0; i<data_->count(); i++) {
			markers_[i]->setPos(data_->x(i), data_->y(i));
			// TODO: optimize this. Need to do this every time we place?
			markers_[i]->setPen(markerPen_);
			markers_[i]->setBrush(markerBrush_);

		}
		
	}
	
	void createLines(int num) {
		
		// remove extra lines
		while(lines_.count() > num) {
			delete lines_.takeFirst();
		}
		
		// create additional lines:
		while(lines_.count() < num) {
			lines_ << new QGraphicsLineItem(this);
		}
	}
	
	void placeAllLines() {
		
		if(!data_)
			return;
		
		for(int i=1; i<data_->count(); i++) {
			lines_[i-1]->setLine(data_->x(i-1), data_->y(i-1), data_->x(i), data_->y(i));
			lines_[i-1]->setPen(linePen_);
		}
	}

};

#endif
