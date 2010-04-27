#ifndef __MPlotAbstractSeries_H__
#define __MPlotAbstractSeries_H__




#include "MPlotMarker.h"
#include "MPlotItem.h"
#include "MPlotSeriesData.h"

#include <QPen>
#include <QBrush>
#include <QDebug>

// This is the color of the selection highlight
#define MPLOT_SELECTION_COLOR QColor(255, 210, 129)
#define MPLOT_SELECTION_LINEWIDTH 10

// When the number of points exceeds this, we simply return the bounding box instead of the exact shape of the plot.  Makes selection less precise, but faster.
#define MPLOT_EXACTSHAPE_POINT_LIMIT 2000

// TODO:
// Put markers on top of plot lines...
// Optimize placeAllMarkers to not set Brush

class MPlotAbstractSeries : public MPlotItem {
	
	Q_OBJECT
	Q_PROPERTY(bool selected READ selected WRITE setSelected NOTIFY selectedChanged);
	
public:
	
	MPlotAbstractSeries(const MPlotAbstractSeriesData* data = 0) : MPlotItem() {
		
		data_ = 0;
		marker_ = 0;
		
		setFlag(QGraphicsItem::ItemIsSelectable, false);	// We're implementing our own selection mechanism... ignoring QGraphicsView's selection system.
		
		// Set style defaults:
		setDefaults();	// override in subclasses
		
		// Set model (will check that data != 0)
		setModel(data);
		
		isSelected_ = false;
		
	}
	
	
	
	// Properties:	
	virtual void setLinePen(const QPen& pen) { 
		linePen_ = pen; 
		linePen_.setCosmetic(true); 
	}
	
	// Returns the current marker, which can be used to access it's pen, brush, and size.
		// If the plot has no marker (or MPlotMarkerShape::None), then this will be a null pointer. Must check before setting.
	virtual MPlotAbstractMarker* marker() const { return marker_; }
	
	virtual void setMarkerShape(MPlotMarkerShape::Shape shape) {
		double oldsize = 6;
		QPen oldpen;
		QBrush oldbrush;
		if(marker_) {
			oldsize = marker_->size();
			oldpen = marker_->pen();
			oldbrush = marker_->brush();
			delete marker_;
		}
		marker_ = MPlotMarker::create(shape, oldsize, oldpen, oldbrush);
		update();
	}
	
	// Note: must use setMarkerShape to something besides MPlotMarkerShape::None before using these, otherwise changes will be lost.
	virtual void setMarkerPen(const QPen& pen) { 
		if(marker_) {
			marker_->setPen(pen);
			update();
		}
	}
	
	virtual void setMarkerBrush(const QBrush& brush) { 
		if(marker_) {
			marker_->setBrush(brush);
			update();
		}
	}

	
	virtual void setMarkerSize(double size) {
		if(marker_) {
			marker_->setSize(size);
			update();
		}
	}
	
	
	// Sets this series to view the model in 'data';
	virtual void setModel(const MPlotAbstractSeriesData* data) {
		
		// If there was an old model, disconnect old signals:
		if(data_)
			disconnect(data_, 0, this, 0);
		
		// new data from here:
		data_ = data;
		
		// If there's a new valid model:
		if(data_) {
			
			// Connect model signals to slots: dataChanged(unsigned fromIndex, unsigned toIndex);
			connect(data_, SIGNAL(dataChanged(unsigned, unsigned)), this, SLOT(onDataChanged(unsigned, unsigned)));
		}
		
		emit dataChanged(this);
		
	}
	
	virtual const MPlotAbstractSeriesData* model() const { return data_; }

	
	// Required functions:
	//////////////////////////
	// Bounding rect: reported in our PlotSeries coordinates, which are just the actual data coordinates. This is used by the graphics view system to figure out how much we cover/need to redraw.  Subclasses that draw selection borders or markers need to add their size on top of this.
	virtual QRectF boundingRect() const { if(data_) return data_->boundingRect(); else return QRectF(); }

	// Data rect: also reported in our PlotSeries coordinates, which are the actual data coordinates. This is used by the auto-scaling to figure out the range of our data on an axis.
	virtual QRectF dataRect() const { if(data_) return data_->boundingRect(); else return QRectF(); }
	
	// Paint: must be implemented in subclass.
	virtual void paint(QPainter* painter,
					   const QStyleOptionGraphicsItem* option,
					   QWidget* widget) = 0;	
	
	virtual QPainterPath shape() const {
		
		QPainterPath shape;
		
		// If there's under 1000 points, we can return a detailed shape with ok performance.
		// Above 1000 points, let's just return the bounding box.
		if(data_ && data_->count() > MPLOT_EXACTSHAPE_POINT_LIMIT)
			shape.addRect(boundingRect());
		
		
		else if(data_ && data_->count() > 0) {
			shape.moveTo(data_->x(0), data_->y(0));
			for(unsigned i=0; i<data_->count(); i++)
				shape.lineTo(data_->x(i), data_->y(i));
			
			for(int i=data_->count()-2; i>=0; i--)
				shape.lineTo(data_->x(i), data_->y(i));
			shape.lineTo(data_->x(0), data_->y(0));
		}
		
		return shape;
	}
	
protected slots:
	virtual void onDataChanged(unsigned fromIndex, unsigned toIndex) {
		Q_UNUSED(fromIndex)
		Q_UNUSED(toIndex)
		emit dataChanged(this);
	}
	
signals:
	
	// in base class: void dataChanged(MPlotAbstractSeries* series);	// listen to this if you want to auto-scale on changes.
	// in base class: void selectedChanged(bool isSelected);
		
	
protected:
	QPen linePen_, selectedPen_;
	MPlotAbstractMarker* marker_;
	
	const MPlotAbstractSeriesData* data_;
	
	virtual void setDefaults() {
		
		yAxisTarget_ = MPlotAxis::Left;
		
		setLinePen(QPen(QColor(Qt::red)));	// Red solid lines on plot
		
		setMarkerShape(MPlotMarkerShape::Square);
		setMarkerPen(QPen(QColor(Qt::blue), 0)); // Blue outlines on markers
		setMarkerBrush(QBrush());	// default: NoBrush

		
		selectedPen_ = QPen(QBrush(MPLOT_SELECTION_COLOR), MPLOT_SELECTION_LINEWIDTH);
		selectedPen_.setCosmetic(true);
	}
};
	
#endif
