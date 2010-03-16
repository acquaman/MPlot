#ifndef __MPlotSeries_H__
#define __MPlotSeries_H__

#include <QGraphicsObject>
#include <QGraphicsLineItem>
#include <QQueue>
#include <QPen>
#include <QBrush>
#include <QPainterPath>
#include <QPainter>
//#include <QGraphicsSceneMouseEvent>
//#include <QGraphicsView>
//#include <QGraphicsScene>
#include "MPlotMarker.h"
#include "MPlotAxis.h"
#include "MPlotSeriesData.h"

#include <QDebug>


// TODO:
// Put markers on top of plot lines...
// Optimize placeAllMarkers to not set Brush

class MPlotSeries : public QGraphicsObject {
	
	Q_OBJECT
	
	Q_PROPERTY(bool selected READ isSelected WRITE setSelected)

public:
	
	MPlotSeries(const MPlotSeriesData* data = 0, QGraphicsItem* parent = 0) : QGraphicsObject(parent) {
		
		data_ = 0;
		setFlag(QGraphicsItem::ItemIsSelectable, false);	// We're implementing our own selection mechanism... ignoring QGraphicsView's selection system.
		setFlag(QGraphicsItem::ItemHasNoContents, true);// all painting done by children
		
		// Set style defaults:
		setDefaults();
		
		// Set model (will check that data != 0)
		setModel(data);
		
	}
	
	
	
	// Properties:	
	void setLinePen(const QPen& pen) { 
		linePen_ = pen; 
		linePen_.setCosmetic(true); 
		foreach(QGraphicsLineItem* line, lines_) {
			line->setPen(linePen_);
		}
	}
	void setMarkerPen(const QPen& pen) { 
		markerPen_ = pen; 
		markerPen_.setCosmetic(true);
		foreach(MPlotAbstractMarker* marker, markers_) {
			marker->setPen(markerPen_);
		}
	}
		
	void setMarkerBrush(const QBrush& brush) { 
		markerBrush_ = brush; 
		foreach(MPlotAbstractMarker* marker, markers_) {
			marker->setBrush(markerBrush_);
		}
	}
	void setMarkerShape(MPlotMarkerShape::Shape shape) {
		markerShape_ = shape;
		createMarkers(0);
		if(data_) {
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
		connect(data_, SIGNAL(rowsInserted(const QModelIndex &, int, int)), this, SLOT(onRowsInserted( const QModelIndex &, int, int )));
		connect(data_, SIGNAL(rowsRemoved(const QModelIndex &, int, int)), this, SLOT(onRowsRemoved( const QModelIndex &, int, int )));
		connect(data_, SIGNAL(dataChanged( const QModelIndex &, const QModelIndex & )), this, SLOT(onDataChanged( const QModelIndex &, const QModelIndex & )));
		
		emit dataChanged(this);
	}
	
	const MPlotSeriesData* model() const { return data_; }
	
	
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
	
	
	virtual QPainterPath shape() const {
		
		QPainterPath shape;
		
		// If there's under 1000 points, we can return a detailed shape with ok performance.
		// Above 1000 points, let's just return the bounding box.
		if(data_ && data_->count() > 1000)
			shape.addRect(boundingRect());
		
		
		else if(data_ && data_->count() > 0) {
			shape.moveTo(data_->x(0), data_->y(0));
			for(int i=0; i<data_->count(); i++)
				shape.lineTo(data_->x(i), data_->y(i));
						
			for(int i=data_->count()-2; i>=0; i--)
				shape.lineTo(data_->x(i), data_->y(i));
			shape.lineTo(data_->x(0), data_->y(0));
		}
		
		return shape;
	}
	

signals:
	
	void dataChanged(MPlotSeries* series);	// listen to this if you want to auto-scale on changes.
	
protected slots:
	void onRowsInserted( const QModelIndex & /*parent*/, int start, int end ) {
		
		// This signal should only be delivered by a valid data_ model:
		
		// Handle single insert at beginning: (this is fast)
		if(start == 0 && end == 0) {
			
			if(data_->count() > 1) {
				QGraphicsLineItem* newline = new QGraphicsLineItem(this);
				lines_.prepend(newline);
				newline->setLine(data_->x(0), data_->y(0), data_->x(1), data_->y(1));
				newline->setPen(linePen_);
			}
			
			if(markerShape_) {
				MPlotAbstractMarker* newmarker = MPlotMarker::create(markerShape_, this, markerSize_);
				markers_.prepend( newmarker );
				newmarker->setPos(data_->x(0), data_->y(0));
				newmarker->setPen(markerPen_);
				newmarker->setBrush(markerBrush_);
			}
		}
		
		// Handle single insert at end: (this is also pretty fast)
		// Note: if this is the first item, we've already handled it above. This won't run.
		else if(start == data_->count()-1 && end == data_->count()-1) {
			int insertIndex = data_->count()-1;
			
			QGraphicsLineItem* newline = new QGraphicsLineItem(this);
			lines_.append(newline);
			newline->setLine(data_->x(insertIndex-1), data_->y(insertIndex-1), data_->x(insertIndex), data_->y(insertIndex));
			newline->setPen(linePen_);
			
			if(markerShape_) {
				MPlotAbstractMarker* newmarker = MPlotMarker::create(markerShape_, this, markerSize_);
				markers_.append( newmarker );
				newmarker->setPos(data_->x(insertIndex), data_->y(insertIndex));
				newmarker->setPen(markerPen_);
				newmarker->setBrush(markerBrush_);
			}
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
	
	void onRowsRemoved ( const QModelIndex & /*parent*/, int start, int end ) {
		
		// This signal should only be delivered by a valid data_ model:
		
		// Handle single removal at beginning: (this is fast)
		if(start == 0 && end == 0) {
			if(markerShape_)
				if(!markers_.isEmpty())
					delete markers_.takeFirst();
			if(!lines_.isEmpty())
				delete lines_.takeFirst();			
		}
		
		// Handle single removal at end: (this is also pretty fast)
		else if(start == data_->count() && end == data_->count()) {
			if(markerShape_)
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
	
	void onDataChanged( const QModelIndex & topLeft, const QModelIndex & bottomRight ) {

		
		// This signal should only be delivered by a valid data_ model:
		for(int i = topLeft.row(); i <= bottomRight.row(); i++) {
			
			// Adjust markers (if we have 'em.)
			if(markerShape_)
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
	
	//mutable QPainterPath shape_;
	
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
		
		// If the marker shape is MPlotMarkerNone, don't create. Delete all if they exist.
		if(markerShape_ == MPlotMarkerShape::None)
			num = 0;
		
		// remove extra markers
		while(markers_.count() > num) {
			delete markers_.takeFirst();
		}
		
		// create additional markers:
		while(markers_.count() < num) {
			MPlotAbstractMarker* marker = MPlotMarker::create(markerShape_, this, markerSize_);
			marker->setBrush(markerBrush_);
			marker->setPen(markerPen_);
			markers_ << marker;
		}
	}
	
	// Important: whenever markerShape_ is not MPlotMarkerShape::None, need to make sure first that number of markers matches data_->count()!
	void placeAllMarkers() {
		
		if(!data_)
			return;
		
		if(markerShape_ == MPlotMarkerShape::None)
			return;
		
		for(int i=0; i<data_->count(); i++) {
			markers_[i]->setPos(data_->x(i), data_->y(i));
		}
		
	}
	
	void createLines(int num) {
		
		// remove extra lines
		while(lines_.count() > num) {
			delete lines_.takeFirst();
		}
		
		// create additional lines:
		while(lines_.count() < num) {
			QGraphicsLineItem* line = new QGraphicsLineItem(this);
			line->setPen(linePen_);
			lines_ << line;
		}
	}
	
	void placeAllLines() {
		
		if(!data_)
			return;
		
		for(int i=1; i<data_->count(); i++) {
			lines_[i-1]->setLine(data_->x(i-1), data_->y(i-1), data_->x(i), data_->y(i));
		}
	}
	
};

#endif
