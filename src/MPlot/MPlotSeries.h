#ifndef __MPlotSeries_H__
#define __MPlotSeries_H__

#include "MPlotAbstractSeries.h"
#include <QGraphicsLineItem>
#include <QQueue>

#include <QPainterPath>
#include <QPainter>


// TODO:
// Put markers on top of plot lines...
// Optimize placeAllMarkers to not set Brush

class MPlotSeries : public MPlotAbstractSeries {
	
	Q_OBJECT

public:
	
	MPlotSeries(const MPlotSeriesData* data = 0, QGraphicsItem* parent = 0) : MPlotAbstractSeries(data, parent) {
		
		// unique setup for MPlotSeries?
		
		setFlag(QGraphicsItem::ItemHasNoContents, true);// all painting done by children
		
	}
	
	
	
	// Properties:	
	virtual void setLinePen(const QPen& pen) { 
		MPlotAbstractSeries::setLinePen(pen);
		
		foreach(QGraphicsLineItem* line, lines_) {
			line->setPen(linePen_);
		}
	}
	virtual void setMarkerPen(const QPen& pen) { 
		MPlotAbstractSeries::setMarkerPen(pen);
		
		foreach(MPlotAbstractMarker* marker, markers_) {
			marker->setPen(markerPen_);
		}
	}
		
	virtual void setMarkerBrush(const QBrush& brush) { 
		MPlotAbstractSeries::setMarkerBrush(brush);
		
		foreach(MPlotAbstractMarker* marker, markers_) {
			marker->setBrush(markerBrush_);
		}
	}
	
	virtual void setMarkerShape(MPlotMarkerShape::Shape shape) {
		MPlotAbstractSeries::setMarkerShape(shape);
		
		createMarkers(0);
		if(data_) {
			createMarkers(data_->count());
			placeAllMarkers();
		}
	}
	
	virtual void setMarkerSize(double size) {
		
		MPlotAbstractSeries::setMarkerSize(size);
		
		foreach(MPlotAbstractMarker* marker, markers_) {
			marker->setSize(size);
		}		
	}

	
	// Sets this series to view the model in 'data';
	virtual void setModel(const MPlotSeriesData* data) {
		
		MPlotAbstractSeries::setModel(data);
		
		// If there's a new valid model:
		if(data_) {
			// add initial data points (lines and markers)	
			int numLines = (data_->count() > 1) ? data_->count() - 1 : 0;
			createLines(numLines);
			placeAllLines();

			createMarkers(data_->count());
			placeAllMarkers();
			
			// Connect model signals to slots: rowsInserted(), rowsRemoved(), dataChanged()
			connect(data_, SIGNAL(rowsInserted(const QModelIndex &, int, int)), this, SLOT(onRowsInserted( const QModelIndex &, int, int )));
			connect(data_, SIGNAL(rowsRemoved(const QModelIndex &, int, int)), this, SLOT(onRowsRemoved( const QModelIndex &, int, int )));
			connect(data_, SIGNAL(dataChanged( const QModelIndex &, const QModelIndex & )), this, SLOT(onDataChanged( const QModelIndex &, const QModelIndex & )));
		}
		
		// Otherwise, (data == 0) we're just clearing the model:
		else {
			createLines(0);
			createMarkers(0);
		}
	}
	
	const MPlotSeriesData* model() const { return data_; }
	
	
	// Required functions:
	//////////////////////////
	// boundingRect: reported in our PlotSeries coordinates, which are just the actual data coordinates.
		// using parent implementation
	
	// Paint:
	virtual void paint(QPainter* /*painter*/,
					   const QStyleOptionGraphicsItem* /*option*/,
					   QWidget* /*widget*/) {
		// Do nothing... drawn with children
	}
	
	
	// Overriding parent class with higher-performance. We already know our shape.
	/*
	virtual QPainterPath shape() const {
		
		return plotPathClosed_;
	}*/
	

	
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

	QQueue<MPlotAbstractMarker*> markers_;
	QQueue<QGraphicsLineItem*> lines_;

	// Customize this if needed for MPlotSeries. For now we use the parent class implementation
	/*
	virtual void setDefaults() {
		
	}*/
	
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
