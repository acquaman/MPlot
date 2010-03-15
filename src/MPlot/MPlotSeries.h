#ifndef __MPlotSeries_H__
#define __MPlotSeries_H__

#include <QGraphicsObject>
#include <QGraphicsLineItem>
#include <QQueue>
#include <QPen>
#include <QBrush>
#include <QPainterPath>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QGraphicsScene>
#include "MPlotMarker.h"
#include "MPlotAxis.h"
#include "MPlotSeriesData.h"

#include <QDebug>

#define MPLOT_DESELECTED_OPACITY 0.25
#define MPLOT_SELECTED_OPACITY 1.0

// When selecting lines on plots with the mouse, this is how wide the selection region is (split equally on either side of the line)
#define MPLOT_SELECTION_WIDTH 10

#define MPLOT_SELECTION_COLOR QColor(255, 210, 129)

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
		//setFlag(QGraphicsItem::ItemHasNoContents, true);// all painting done by children
		
		// Set style defaults:
		setDefaults();
		
		// Set model (will check that data != 0)
		setModel(data);
		
		wasSelected_ = false;
		
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
		connect(data_, SIGNAL(rowsInserted(const QModelIndex &, int, int)), this, SLOT(onRowsInserted( const QModelIndex &, int, int )));
		connect(data_, SIGNAL(rowsRemoved(const QModelIndex &, int, int)), this, SLOT(onRowsRemoved( const QModelIndex &, int, int )));
		connect(data_, SIGNAL(dataChanged( const QModelIndex &, const QModelIndex & )), this, SLOT(onDataChanged( const QModelIndex &, const QModelIndex & )));
		
		emit dataChanged(this);
	}
	
	
	// Required functions:
	//////////////////////////
	// Bounding rect: reported in our PlotSeries coordinates, which are just the actual data coordinates.
	virtual QRectF boundingRect() const { if(data_) return data_->boundingRect(); else return QRectF(); }
	// Paint:
	virtual void paint(QPainter* painter,
					   const QStyleOptionGraphicsItem* /*option*/,
					   QWidget* /*widget*/) {
		// Do nothing... drawn with children
		
		// Highlight if selected:
		
		if(isSelected()) {
			QPen pen = QPen(QBrush(MPLOT_SELECTION_COLOR), MPLOT_SELECTION_WIDTH);
			pen.setCosmetic(true);
			painter->setPen(pen);
			painter->drawPath(shape());
		}
	}
	
	
	QTransform firstDeviceTransform() const {
		if(scene()->views().count() > 0)
			return deviceTransform(scene()->views().at(0)->viewportTransform());
		else
			return QTransform();
	}
	
	virtual bool isSelected() { return wasSelected_; }
	
	virtual void setSelected(bool isSelected = true) {
		if(wasSelected_ = isSelected)
			emit selected();
		else
			emit unselected();
		
		// schedule a redraw, to draw/not-draw the highlight:
		update();
	}
	
	virtual bool contains ( const QPointF & point ) const {
		QPainterPath circleAroundPoint;
		QTransform dt = firstDeviceTransform();
		
		circleAroundPoint.addEllipse(point, MPLOT_SELECTION_WIDTH/dt.m11(), MPLOT_SELECTION_WIDTH/dt.m22());
		
		return circleAroundPoint.intersects(shape());
	}
	
	
	virtual QPainterPath shape() const {
		// Returns a shape consisting of the shapes of the markers, and all the lines plus 5 pixels on either side (for easy selection)
		// todo: should this be optimized? Change when adding points, not every time it's called?
		
		// TODO: selection only works on one side of some lines.
		// TODO: bounding box doesn't include selection highlight.
		
		
		// Add the lines:
		QPainterPath shape;
		
		if(data_ && data_->count() > 0) {
			shape.moveTo(data_->x(0), data_->y(0));
			for(int i=0; i<data_->count(); i++)
				shape.lineTo(data_->x(i), data_->y(i));
						
			for(int i=data_->count()-2; i>=0; i--)
				shape.lineTo(data_->x(i), data_->y(i));
			shape.lineTo(data_->x(0), data_->y(0));
		}

		// Add the markers:
		// This doesn't work because the ItemIgnoresTranformations is set, so the dimensions are too big/small to add directly.
		//foreach(MPlotAbstractMarker* marker, markers_) {
		//	shape.addPath(marker->shape());
		//}
		
		return shape;
		
	}
	

signals:
	
	void dataChanged(MPlotSeries* series);	// listen to this if you want to auto-scale on changes.
	void selected();	// emitted when the plot series is selected by mouse.
	void unselected();  // emitted when the plot series is unselected (ie: setSelected(false); )
	
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
	
	void onRowsRemoved ( const QModelIndex & /*parent*/, int start, int end ) {
		
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
	
	void onDataChanged( const QModelIndex & topLeft, const QModelIndex & bottomRight ) {
		
		// todo: necessary?
		prepareGeometryChange();
		
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
	
	//mutable QPainterPath shape_;
	
	bool wasSelected_;	// required to detect transitions from selected to unselected and vice versa
	
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
			MPlotAbstractMarker* marker = MPlotMarker::create(markerShape_, this, markerSize_);
			marker->setBrush(markerBrush_);
			marker->setPen(markerPen_);
			markers_ << marker;
		}
	}
	
	// Important: need to make sure first that number of markers matches data_->count()!
	void placeAllMarkers() {
		
		if(!data_)
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
	
	// This is used to detect selection/unselection
	virtual void mousePressEvent ( QGraphicsSceneMouseEvent * event ) {
		qDebug() << objectName() << "press event... in bounding box";
		
		// If it's "close" to us:
		if( contains(event->pos()) ) {
		   qDebug() << objectName() << "   inside shape";
			if(!isSelected())
				this->setSelected(true);	// set selected
			// absorb this event here... don't let it keep travelling.
		}
		else	// propagate the event like normal.
			QGraphicsItem::mousePressEvent(event);
	}

};

#endif
