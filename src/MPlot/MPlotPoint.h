#ifndef MPLOTPOINT_H
#define MPLOTPOINT_H

#include "MPlotMarker.h"
#include "MPlotAxis.h"
#include "MPlotItem.h"

/// This Plot Item is useful for displaying a single data point on a plot.
class MPlotPoint : public MPlotItem {

	Q_OBJECT

public:

	/// Default constructor. Override setDefaults() for a custom look off the bat.
	MPlotPoint(const QPointF& value = QPointF(0,0))
		: MPlotItem(),
		point_(value)
	{
		marker_ = 0;

		setDefaults();
	}

	// Returns the current marker, which can be used to access it's pen, brush, and size.
		// If the plot has no marker (or MPlotMarkerShape::None), then this will be a null pointer. Must check before setting.
	virtual MPlotAbstractMarker* marker() const { return marker_; }

	virtual void setMarker(MPlotMarkerShape::Shape shape, double size = 6, const QPen& pen = QPen(QColor(Qt::red)), const QBrush& brush = QBrush(), MPlotMarkerSizeMode::Mode sizeMode = MPlotMarkerSizeMode::Pixels) {
		if(marker_)
			delete marker_;

		QPen realPen(pen);
		realPen.setCosmetic(true);
		marker_ = MPlotMarker::create(shape, size, realPen, brush, sizeMode);
		update();
	}

	/// returns the point location of this marker:
	QPointF value() const {
		return point_;
	}

public slots:
	/// set the point location of this marker:
	void setValue(const QPointF& point) {

		prepareGeometryChange();

		point_ = point;
		emit dataChanged(this);

		update();
	}




	// Required functions:
	//////////////////////////
	// Bounding rect: reported in our PlotSeries coordinates, which are just the actual data coordinates. This is used by the graphics view system to figure out how much we cover/need to redraw.  Subclasses that draw selection borders or markers need to add their size on top of this.
	virtual QRectF boundingRect() const {

		QRectF br = dataRect();

		// expand by the selection line width (in pixels...)
		QRectF hs = QRectF(0, 0, MPLOT_SELECTION_LINEWIDTH, MPLOT_SELECTION_LINEWIDTH);

		// expand by marker size (if expressed in pixels)
		if(marker() && marker()->sizeMode() == MPlotMarkerSizeMode::Pixels)
				hs |= QRectF(0,0, marker()->size(), marker()->size());


		// these sizes so far are in pixels (hopefully scene coordinates... trusting on an untransformed view.) Converting to local (data) coordinates.
		hs = mapRectFromScene(hs);

		// if the marker size mode is in "percent of plot", its size (converted from percentage) would be in the "parent" coordinates which go from (0, 0) to (1,1). (our plot's dataArea_)
		if(marker() && marker()->sizeMode() == MPlotMarkerSizeMode::PercentOfPlot) {
			QRectF fixedSize = mapRectFromParent(0,0, marker()->size()/100., marker()->size()/100.);
			hs |= QRectF(0,0, fixedSize.width(), fixedSize.height());
			qDebug() << "Marker size in local (data) coordinates:" << QRectF(0,0, fixedSize.width(), fixedSize.height());
		}


		// really we just need 1/2 the marker size and 1/2 the selection highlight width. But extra doesn't hurt.
		br.adjust(-hs.width(),-hs.height(),hs.width(), hs.height());

		return br;
	}

	// Data rect: also reported in our PlotSeries coordinates, which are the actual data coordinates. This is used by the auto-scaling to figure out the range of our data on an axis.
	virtual QRectF dataRect() const { return QRectF(point_, QSizeF(0,0)); }

	// Paint: must be implemented in subclass.
	virtual void paint(QPainter* painter,
					   const QStyleOptionGraphicsItem* option,
					   QWidget* widget)
	{
		Q_UNUSED(option)
		Q_UNUSED(widget)

		if(marker_) {
			QTransform wt; //our "world transform"
			// If in "percent of plot" coordinates, the transform is just up to our parent dataArea_ (0,0->1,1), and then corrected for percentage
			if(marker_->sizeMode() == MPlotMarkerSizeMode::PercentOfPlot)
				if(parentItem())
					wt = itemTransform(parentItem()).scale(100, 100);
			else
				wt = painter->deviceTransform();	// equivalent to worldTransform() and combinedTransform()


			QTransform wtInverse(QTransform::fromScale(1/wt.m11(), 1/wt.m22()));
			//wtInverse.scale(1/wt.m11(), 1/wt.m22());

			// Paint marker:
			painter->save();
			painter->setPen(marker_->pen());
			painter->setBrush(marker_->brush());

			painter->translate(point_.x(), point_.y());
			painter->setTransform(wtInverse, true);
			marker_->paint(painter);

			// repaint with selected pen, if selected...
			if(selected()) {
				painter->setPen(selectedPen_);
				marker_->paint(painter);
			}

			painter->restore();
		}
	}

protected:
	QPen selectedPen_;
	MPlotAbstractMarker* marker_;

	QPointF point_;

	virtual void setDefaults() {

		setYAxisTarget(MPlotAxis::Left);

		setMarker(MPlotMarkerShape::CrossCircle, 24, QPen(QColor(Qt::red), 0), QBrush());

		QColor selectionColor = MPLOT_SELECTION_COLOR;
		selectionColor.setAlphaF(MPLOT_SELECTION_OPACITY);
		selectedPen_ = QPen(QBrush(selectionColor), MPLOT_SELECTION_LINEWIDTH);
		selectedPen_.setCosmetic(true);

	}


};


#endif // MPLOTPOINT_H
