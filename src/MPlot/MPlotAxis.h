#ifndef __MPlotAxis_H__
#define __MPlotAxis_H__

#include <QGraphicsItem>
#include <QGraphicsLineItem>
#include <QList>
#include <QFont>
#include <QPen>

#include <math.h>

// Coordinate system: draws self in the (0,0) to (1,1) coordinate space of MPlot's plotArea_.
class MPlotAxis : public QGraphicsItem {

public:
	// Types:
	enum AxisID { Left = 1, Bottom = 2, Right = 4, Top = 8 };
	enum TickStyle { Outside, Inside, Middle };
	
	// Axis ID:
	MPlotAxis(AxisID type, QString name = QString(""), QGraphicsItem* parent = 0) : QGraphicsItem(parent) {
		
		setFlags(QGraphicsItem::ItemHasNoContents);

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
		extraGrid_ = new QGraphicsLineItem(this);
		extraGrid_->setVisible(false);
		
		
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
		delete extraGrid_;
	}
	
	// Set the range (in data-value coordinates)
	void setRange(double min, double max) {
		
		min_ = min; max_ = max;		
		
		placeAxis();
		
	}
		
	// Set the number and style of ticks:
		// tStyle can be Inside, Outside, or Middle.  tickLength is in logical coordinates ("percent of plot") coordinates.
	void setTicks(int num, TickStyle tstyle = Outside, double tickLength = 2) {
		
		if(num != ticks_.count()) {
			createTicks(num);
		}
		tickStyle_ = tstyle;
		tickLength_ = tickLength/100;
		placeAxis();
	}
	
	
	// Access properties:
	double min() const { return min_; }
	double max() const { return max_; }
	int numTicks() const { return ticks_.count(); }
	const QString& name() const { return name_; }
	// TODO: access font, labelOffset, etc.
		
	void showTickLabels(bool valuesOn) { tickLabelsVisible_ = valuesOn;  placeAxis(); }
	void showGrid(bool gridOn) { gridVisible_ = gridOn; placeAxis(); }
	
	// Set the pen for the axis line:
	void setAxisPen(const QPen& pen) { axisPen_ = pen; axisPen_.setCosmetic(true); mainLine_->setPen(axisPen_); }
	void setTickPen(const QPen& pen) { tickPen_ = pen; tickPen_.setCosmetic(true); foreach(QGraphicsLineItem* tick, ticks_) tick->setPen(tickPen_); extraTick_->setPen(tickPen_); }
	void setGridPen(const QPen& pen) { gridPen_ = pen; gridPen_.setCosmetic(true); foreach(QGraphicsLineItem* grid, grids_) grid->setPen(gridPen_); extraGrid_->setPen(gridPen_); }
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
	
	// tick lengths, in logical units (fraction of plot width)
	double tickLength_;
	double tickLabelOffset_;

	bool tickLabelsVisible_, gridVisible_;
	
	QString name_;
	
	QPen axisPen_, tickPen_, gridPen_;
	QFont tickLabelFont_, labelFont_;
	
	// Actual tick values:
	double minTickVal_, tickIncVal_;
	
	// Objects:
	QGraphicsLineItem* mainLine_, *extraTick_, *extraGrid_;
	QList<QGraphicsLineItem*> ticks_, grids_;
	QList<QGraphicsSimpleTextItem*> labels_;
	QGraphicsSimpleTextItem* extraLabel_;
	
	
	//Helper Functions:
	
	// Adjust the length of the main line, and re-place the correct number of ticks.
	void placeAxis() {
		
		// Determine "nice" values for the axis labels. (Sets minTickVal_ and tickIncVal_)
		intelliScale();
		
		// The ratio of scene coordinates to data coordinates:
		double scPerDC = 1.0 / (max_ - min_);
		double scIncrement = tickIncVal_ * scPerDC;
		double scStart = (minTickVal_ - min_) * scPerDC;		
		
		QLineF tickLine, gridLine;
		QPointF scIncP, scStartP;
		
		// Handle differences between vertical and horizontal axes:
		if(type_ == Left || type_ == Right) {
			scIncP = QPointF(0, scIncrement);
			// Left axis:
			if(type_ == Left) {
				mainLine_->setLine(0, 0, 0, 1);
				scStartP = QPointF(0, scStart);
				gridLine.setLine(0, 0, 1, 0);
				switch(tickStyle_) {
					case Outside: tickLine.setLine(-tickLength_, 0, 0, 0); break;
					case Inside: tickLine.setLine(tickLength_, 0, 0, 0); break;
					case Middle: tickLine.setLine(-tickLength_/2, 0, tickLength_/2, 0); break;
				}
			}
			// Right axis:
			else {
				mainLine_->setLine(1, 0, 1, 1);
				scStartP = QPointF(1, scStart);
				gridLine.setLine(1, 0, 0, 0);
				switch(tickStyle_) {
					case Outside: tickLine.setLine(tickLength_, 0, 0, 0); break;
					case Inside: tickLine.setLine(-tickLength_, 0, 0, 0); break;
					case Middle: tickLine.setLine(-tickLength_/2, 0, tickLength_/2, 0); break;
				}
			}
		}
		
		// Same for horizontal axes (increment is horizontal, tickline is vertical)
		else{
			scIncP = QPointF(scIncrement, 0);			
			if(type_ == Bottom) {
				mainLine_->setLine(0, 0, 1, 0);
				scStartP = QPointF(scStart, 0);
				gridLine.setLine(0, 0, 0, 1);
				switch(tickStyle_) {
					case Inside: default: tickLine.setLine(0, tickLength_, 0, 0); break;
					case Outside: tickLine.setLine(0, -tickLength_, 0, 0); break;
					case Middle: tickLine.setLine(0, tickLength_/2, 0, -tickLength_/2); break;
				}
			}
			else {
				mainLine_->setLine(0, 1, 1, 1);
				scStartP = QPointF(scStart, 1);
				gridLine.setLine(0, 1, 0, 0);
				switch(tickStyle_) {
					case Inside: default: tickLine.setLine(0, -tickLength_, 0, 0); break;
					case Outside: tickLine.setLine(0, +tickLength_, 0, 0); break;
					case Middle: tickLine.setLine(0, tickLength_/2, 0, -tickLength_/2); break;
				}
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
				labels_[i]->setVisible(false);	// todo: necessary?
			}
			
			// Place grids:
			if(gridVisible_) {
				grids_[i]->setPen(gridPen_); 
				grids_[i]->setLine(gridLine);
				grids_[i]->setPos(scStartP + i*scIncP);
				grids_[i]->setVisible(true);
			}
			else {
				grids_[i]->setVisible(false);	// todo: necessary?
			}
		}
		
		// Is there room left for the extra tick on the axis?
		if(ticks_.count() > 1 && scStart + ticks_.count()*scIncrement < 1) {
			extraTick_->setPen(tickPen_);
			extraTick_->setLine(tickLine);
			extraTick_->setPos(scStartP + ticks_.count()*scIncP);
			extraTick_->setVisible(true);
			
			if(tickLabelsVisible_)
				placeLabel(extraLabel_, minTickVal_ + ticks_.count()*tickIncVal_, scStartP + ticks_.count()*scIncP);
			else
				extraLabel_->setVisible(false);
			
			if(gridVisible_) {
				extraGrid_->setPen(gridPen_);
				extraGrid_->setLine(gridLine);
				extraGrid_->setPos(scStartP + ticks_.count()*scIncP);
				extraGrid_->setVisible(true);
			}
			else
				extraGrid_->setVisible(false);
		}
		
		// no room for extra tick
		else {
			extraTick_->setVisible(false);
			extraLabel_->setVisible(false);
			extraGrid_->setVisible(false);
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
			delete grids_.takeFirst();
		}
		
		// create additional ticks:
		while(ticks_.count() < num) {
			ticks_ << new QGraphicsLineItem(this);
			labels_ << new QGraphicsSimpleTextItem(this);
			grids_ << new QGraphicsLineItem(this);
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

			
			// Hit Zero if possible: (while passing through origin)
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
	}
	
	void setDefaults() {
		
		min_ = 0;
		max_ = 10;
		
		tickStyle_ = Outside;
		tickLength_ = .02;
		
		tickLabelOffset_ = 5;
		tickLabelFont_.setPixelSize(12);
		
		if(type_ == Top || type_ == Right)
			tickLabelsVisible_ = false;
		else
			tickLabelsVisible_ = true;
		
		gridPen_ = QPen(QBrush(QColor(Qt::blue)), 1, Qt::DotLine);
		gridPen_.setCosmetic(true);
		QVector<qreal> dashes;
		dashes << 4 << 4;
		gridPen_.setDashPattern(dashes);
		
		if(type_ == Left)
			gridVisible_ = true;
		else
			gridVisible_ = false;
	}

};

#endif
