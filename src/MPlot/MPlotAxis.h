#ifndef __MPlotAxis_H__
#define __MPlotAxis_H__

#include <QGraphicsItem>
#include <QGraphicsLineItem>
#include <QList>
#include <QFont>

#include <math.h>

// Coordinate system: draws self from 0 to 1, either horizontally (if type is Bottom or Top), or vertically (if type is Left or Right)
class MPlotAxis : public QGraphicsItem {

public:
	// Types:
	enum AxisID { Left = 1, Bottom = 2, Right = 4, Top = 8 };
	enum TickStyle { Outside, Inside, Middle };
	
	// Axis ID:
	MPlotAxis(AxisID type, QString name = QString(""), QGraphicsItem* parent = 0) : QGraphicsItem(parent) {
		
		type_ = type;		
		name_ = name;
		
		setDefaults();	
				
		// Create main axis border:
		mainLine_ = new QGraphicsLineItem(this);
		
		// Normal ticks/tick labels are created in createTicks().
		// These extra guys hang around, and we enable them if there's room to fit an additional tick on the axis scale.
		extraTick_ = new QGraphicsLineItem(this);
		extraTick_->setVisible(false);
		extraLabel_ = new QGraphicsSimpleTextItem(this);
		extraLabel_->setVisible(false);
		
		
		// Create the tick marks:
		createTicks(5);
		
		// Put everything in the right spot:
		placeAxis();		
	}
	
	virtual ~MPlotAxis() {
		// Destroy all ticks:
		createTicks(0);
		
		// Delete the main axis
		delete mainLine_;
		delete extraTick_;
		delete extraLabel_;
	}
	
	// Set the range (in data values)
	void setRange(double min, double max) {
		
		min_ = min; max_ = max;		
		
		placeAxis();
		
	}
		
	// Set the number and style of ticks:
	void setTicks(int num, TickStyle tstyle = Outside) {
		
		if(num != ticks_.count()) {
			createTicks(num);
		}
		tickStyle_ = tstyle;
		placeAxis();
	}
	
	
	// Access properties:
	double min() const { return min_; }
	double max() const { return max_; }
	int numTicks() const { return ticks_.count(); }
	const QString& name() const { return name_; }
	// TODO: access font, labelOffset, etc.
	
	// Set the scene-coordinate-system length of the axis (needed to redraw when the size of the scene changes )
	void setExtent(double sceneCoordLength) { scLength_ = sceneCoordLength; placeAxis(); }
	void showTickLabels(bool valuesOn) { tickLabelsVisible_ = valuesOn;  placeAxis(); }
	
	// Set the pen for the axis line:
	void setAxisPen(const QPen& pen) { axisPen_ = pen; axisPen_.setCosmetic(true); mainLine_->setPen(axisPen_); }
	void setTickPen(const QPen& pen) { tickPen_ = pen; tickPen_.setCosmetic(true); foreach(QGraphicsLineItem* tick, ticks_) tick->setPen(tickPen_); }
	// TODO: minor ticks
	
	// Required functions:
	//////////////////////////
	// Bounding rect:
	virtual QRectF boundingRect() const { return childrenBoundingRect(); }
	// Paint:
	virtual void paint(QPainter* /*painter*/,
		  const QStyleOptionGraphicsItem* /*option*/,
		  QWidget* /*widget*/) {
		// Do nothing... drawn with children
	}
	// TODO: finer shape?
	/*
	QPainterPath shape() {
		return 
	}*/
	
protected:
	
	// Properties:
	double min_, max_;
	
	AxisID type_;
	TickStyle tickStyle_;
	double tickLength_;
	double tickLabelOffset_;
	bool tickLabelsVisible_;
	
	QString name_;
		
	double scLength_;
	
	QPen axisPen_, tickPen_;
	QFont tickLabelFont_, labelFont_;
	
	// Actual tick values:
	double minTickVal_, tickIncVal_;
	
	// Objects:
	QGraphicsLineItem* mainLine_, *extraTick_;
	QList<QGraphicsLineItem*> ticks_;
	QList<QGraphicsSimpleTextItem*> labels_;
	QGraphicsSimpleTextItem* extraLabel_;
	
	
	//Helper Functions:
	
	// Adjust the length of the main line, and re-place the correct number of ticks.
	void placeAxis() {
		
		// Determine "nice" values for the axis labels. (Sets minTickVal_ and tickIncVal_)
		intelliScale();
		
		// The ratio of scene coordinates to data coordinates:
		double scPerDC = scLength_ / (max_ - min_);
		double scIncrement = tickIncVal_ * scPerDC;
		double scStart = (minTickVal_ - min_) * scPerDC;		
		
		QLineF tickLine;
		QPointF scIncP, scStartP;
		
		// Handle differences between vertical and horizontal axes:
		if(type_ == Left || type_ == Right) {
			mainLine_->setLine(0, 0, 0, -scLength_);
			scIncP = QPointF(0, -scIncrement);
			scStartP = QPointF(0, -scStart);
			
			if(type_ == Left)
				switch(tickStyle_) {
					case Outside: tickLine.setLine(-tickLength_, 0, 0, 0); break;
					case Inside: tickLine.setLine(tickLength_, 0, 0, 0); break;
					case Middle: tickLine.setLine(-tickLength_/2, 0, tickLength_/2, 0); break;
				}
			else
				switch(tickStyle_) {
					case Outside: tickLine.setLine(tickLength_, 0, 0, 0); break;
					case Inside: tickLine.setLine(-tickLength_, 0, 0, 0); break;
					case Middle: tickLine.setLine(-tickLength_/2, 0, tickLength_/2, 0); break;
				}
		}
		
		// Same for horizontal axes (increment is horizontal, tickline is vertical)
		else{
			mainLine_->setLine(0, 0, scLength_, 0);
			scIncP = QPointF(scIncrement, 0);
			scStartP = QPointF(scStart, 0);
			
			if(type_ == Bottom)
				switch(tickStyle_) {
					case Outside: default: tickLine.setLine(0, tickLength_, 0, 0); break;
					case Inside: tickLine.setLine(0, -tickLength_, 0, 0); break;
					case Middle: tickLine.setLine(0, tickLength_/2, 0, -tickLength_/2); break;
				}
			else
				switch(tickStyle_) {
					case Outside: default: tickLine.setLine(0, -tickLength_, 0, 0); break;
					case Inside: tickLine.setLine(0, +tickLength_, 0, 0); break;
					case Middle: tickLine.setLine(0, tickLength_/2, 0, -tickLength_/2); break;
				}
		}
		
		// Place all the ticks along the axis:
		for(int i=0; i<ticks_.count(); i++) {
			// Place Ticks:
			ticks_[i]->setPen(tickPen_);
			ticks_[i]->setLine(tickLine);
			ticks_[i]->setPos(scStartP + i*scIncP);
			
			// Place Tick Labels:
			if(tickLabelsVisible_) {
				placeLabel(labels_[i], minTickVal_ + i*tickIncVal_, (scStartP + i*scIncP) );
			}
			else {
				labels_[i]->setVisible(false);
			}
		}
		
		// Is there room left for the extra tick on the axis?
		if(ticks_.count() > 1 && scStart + ticks_.count()*scIncrement < scLength_) {
			extraTick_->setPen(tickPen_);
			extraTick_->setLine(tickLine);
			extraTick_->setPos(scStartP + ticks_.count()*scIncP);
			extraTick_->setVisible(true);
			
			if(tickLabelsVisible_)
				placeLabel(extraLabel_, minTickVal_ + ticks_.count()*tickIncVal_, scStartP + ticks_.count()*scIncP);
			else
				extraLabel_->setVisible(false);
		}
		else {
			extraTick_->setVisible(false);
			extraLabel_->setVisible(false);
		}
	}
	
	// Place a single label onto the axis position corresponding to 'value' at axis position tickPos
	// (Accounts for size/shape of label and shifting into centered position automatically)
	void placeLabel(QGraphicsSimpleTextItem* label, double value, QPointF tickPos) {
		label->setVisible(true);
		label->setScale(0.4);
		label->setText(QString("%1").arg(value));
		label->setFont(tickLabelFont_);
		
		// Label Alignment: depends on size of label
		QSizeF labelSize = label->boundingRect().size()*0.4;
		
		QPointF shift;
		switch(type_) {
			case Bottom: shift.setX(-labelSize.width()/2); shift.setY( tickLabelOffset_ ); break;
			case Top: shift.setX(-labelSize.width()/2); shift.setY( -tickLabelOffset_ - labelSize.height() ); break;
			case Left: shift.setX(-labelSize.width() - tickLabelOffset_); shift.setY( -labelSize.height()/2 ); break;
			case Right: shift.setX( tickLabelOffset_); shift.setY( -labelSize.height()/2 ); break;
		}
		
		label->setPos(tickPos+shift);
	}
	
	void createTicks(int num) {
		
		// remove extra ticks
		while(ticks_.count() > num) {
				delete ticks_.takeFirst();
				delete labels_.takeFirst();
		}
		
		// create additional ticks:
		while(ticks_.count() < num) {
			ticks_ << new QGraphicsLineItem(this);
			labels_ << new QGraphicsSimpleTextItem(this);
		}
	}
	
	// IntelliScale: Calculate "nice" values for starting tick and tick increment.
	// Sets minTickVal_ and tickIncVal_ for nice values of axis ticks.
	// Prior to calling, numTicks() and max_ and min_ must be correct.
	void intelliScale() {
		if(numTicks() > 1) {
			
			// normalize range so difference between max and min goes to 10(max):
			double norm = pow(10, trunc(log10(max_ - min_)) - 1);
			// Round off:
			minTickVal_ = trunc(min_/norm);
			tickIncVal_  = trunc( (max_/norm-minTickVal_)/(numTicks()-1) );

			
			// Hit Zero if possible:
			if(min_ < 0 && max_ > 0) {
				double potentialminTickVal = minTickVal_ + ( (int)(-minTickVal_) % (int)tickIncVal_ );
				if( (potentialminTickVal + tickIncVal_*(numTicks()-1))*norm < max_)	// Just making sure we don't go past the end of the axis with this tweak
					minTickVal_ = potentialminTickVal;
			}
			
			//Rescale:
			minTickVal_ *= norm;
			tickIncVal_ *= norm;
		}
		
		else {	// 1 or zero ticks.
			minTickVal_ = (min_ + max_) / 2;
			tickIncVal_ = 1;
		}
	} // TODO: Sometimes you can fit in an extra tick above (for ex: data from -.3 to +0.8), or it might make more sense to remove an additional tick
		// (ie: tickIncVal_ goes from min to max, instead of minTickVal to max, and just don't add the ticks that overflow the edge)
	// Handle changing the approximate number of ticks.
	
	
	
	void setDefaults() {
		
		min_ = 0;
		max_ = 10;
		
		tickStyle_ = Outside;
		tickLength_ = 2;
		
		tickLabelOffset_ = 5;
		tickLabelFont_.setPixelSize(12);
		
		if(type_ == Top || type_ == Right)
			tickLabelsVisible_ = false;
		else
			tickLabelsVisible_ = true;
	}

};

#endif
