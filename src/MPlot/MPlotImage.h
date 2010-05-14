#ifndef MPLOTIMAGE_H
#define MPLOTIMAGE_H

#include "MPlotImageData.h"
#include "MPlotColorMap.h"
#include "MPlotItem.h"
#include "MPlotObserver.h"

class MPlotAbstractImage : public MPlotItem, public MPlotObserver {

public:

	MPlotAbstractImage(const MPlotAbstractImageData* data = 0)
		: MPlotItem(), MPlotObserver(),
		defaultColorMap_()
	{

		map_ = &defaultColorMap_;
		data_ = 0;

		// Set style defaults:
		setDefaults();	// override in subclasses for custom appearance

		// Set model (will check that data != 0)
		setModel(data);

	}



	// Properties:
	/// Set the color map, used to convert numeric values into pixel colors. \c map must be a reference to a color map that exists elsewhere, and must exist as long as it is set. (We don't make a copy of the map).
	virtual void setColorMap(MPlotAbstractColorMap* map) {
		if(map) {
			map_ = map;
			onDataChanged();
		}
	}

	/// Returns a reference to the active color map.
	virtual MPlotAbstractColorMap* colorMap() {
		return map_;
	}




	// Sets this series to view the model in 'data';
	virtual void setModel(const MPlotAbstractImageData* data) {

		// If there was an old model, disconnect old signals:
		if(data_)
			data_->removeObserver(this);

		// new data from here:
		data_ = data;

		// If there's a new valid model:
		if(data_) {

			// Connect model signals to slots: Emit(0, dataChanged) and Emit(1, boundsChanged).
			data_->addObserver(this);
		}

		Emit(0, "dataChanged");

	}

	virtual const MPlotAbstractImageData* model() const { return data_; }


	// Required functions:
	//////////////////////////
	/// Bounding rect: reported in PlotItem coordinates, which are just the actual data coordinates. This is used by the graphics view system to figure out how much we cover/need to redraw.  Subclasses that draw selection borders or markers need to add their size on top of this.
	virtual QRectF boundingRect() const { if(data_) return data_->boundingRect(); else return QRectF(); }

	/// Data rect: also reported in PlotItem coordinates, which are the actual data coordinates. This is used by the auto-scaling to figure out the range of our data on an axis.
	virtual QRectF dataRect() const { if(data_) return data_->boundingRect(); else return QRectF(); }



public: // "slots"

	virtual void onObservableChanged(MPlotObservable* source, int code, const char* msg, int payload) {

		Q_UNUSED(source)
		Q_UNUSED(msg)
		Q_UNUSED(payload)

		switch(code) {

		case 0: // dataChanged:
			onDataChanged();
			break;

		case 1: // boundsChanged.  This could require re-scaling on the 2D plot, so we MPlotItem::Emit(0, "dataChanged").  Sorry for the confusing names.
			if(data_)
				onBoundsChanged(data_->boundingRect());
			Emit(0, "dataChanged");
		}
	}


protected:

	const MPlotAbstractImageData* data_;

	MPlotLinearColorMap defaultColorMap_;
	MPlotAbstractColorMap* map_;

	virtual void setDefaults() {

		defaultColorMap_ = MPlotLinearColorMap(QColor(Qt::white), QColor(Qt::darkBlue));
		map_ = &defaultColorMap_;
	}

	/// When the z-data changes, this is called to allow an update:
	virtual void onDataChanged() = 0;
	/// When the bounds change, this is called to allow whatever needs to happen for computing a new raster grid, etc.
	virtual void onBoundsChanged(const QRectF& newBounds) = 0;

};


/// This class implements an image (2d intensity plot), using a cached, scaled QPixmap for drawing
class MPlotImageBasic : public MPlotAbstractImage {

public:
	/// Constructor
	MPlotImageBasic(const MPlotAbstractImageData* data = 0)
		: MPlotAbstractImage(data),
		image_(1,1, QImage::Format_ARGB32)
	{
		if(data)
			onDataChanged();
	}


	/// Sets this plot item to view the model in 'data';
	virtual void setModel(const MPlotAbstractImageData* data) {

		MPlotAbstractImage::setModel(data);

		// fill the pixmap and trigger an update
		onDataChanged();
		update();

	}

	/// Paint: must be implemented in subclass.
	virtual void paint(QPainter* painter,
					   const QStyleOptionGraphicsItem* option,
					   QWidget* widget) {
		Q_UNUSED(option)
		Q_UNUSED(widget)

		if(data_) {
			painter->drawImage(data_->boundingRect(), image_, QRectF(QPointF(0,0), QSizeF(data_->size())));

			if(selected()) {
				QColor selectionColor(MPLOT_SELECTION_COLOR);
				QPen selectionPen(selectionColor, MPLOT_SELECTION_LINEWIDTH);
				selectionPen.setCosmetic(true);
				painter->setPen(selectionPen);
				selectionColor.setAlphaF(MPLOT_SELECTION_OPACITY);
				painter->setBrush(selectionColor);
				painter->drawRect(data_->boundingRect());
			}
		}

		/// \todo selection border
	}

	// boundingRect: reported in PlotItem coordinates, which are just the actual data coordinates.
	// using parent implementation, but adding extra room on edges for our selection highlight.
	virtual QRectF boundingRect() const {
		QRectF br = MPlotAbstractImage::boundingRect();
		if(br.isValid()) {
			// create rectangle at least as big as our selection highlight, and if we have a marker, the marker size.
			QRectF hs = QRectF(0, 0, MPLOT_SELECTION_LINEWIDTH, MPLOT_SELECTION_LINEWIDTH);

			// these sizes so far are in pixels (hopefully scene coordinates... trusting on an untransformed view.) Converting to local coordinates.
			hs = mapRectFromScene(hs);
			// really we just need 1/2 the marker size and 1/2 the selection highlight width. But extra doesn't hurt.
			br.adjust(-hs.width(),-hs.height(),hs.width(), hs.height());
		}
		return br;
	}


protected:
	/// Called when the z-data changes, so that the plot needs to be updated. This fills the pixmap buffer
	virtual void onDataChanged() {

		if(data_) {
			// resize if req'd:
			QSize dataSize = data_->size();

			if(image_.size() != dataSize)
				image_ = QImage(dataSize, QImage::Format_ARGB32);

			for(int yy=0; yy<dataSize.height(); yy++)
				for(int xx=0; xx<dataSize.width(); xx++)
					image_.setPixel(xx, yy, map_->rgb(data_->z(QPoint(xx,yy)), data_->range()));

		}

		// schedule a draw update
		update();

		/// \todo Determine if needed: Emit(0, "dataChanged");
	}

	/// If the bounds of the data change (in x- and y-) this might require re-auto-scaling of a plot.
	virtual void onBoundsChanged(const QRectF& newBounds) {
		Q_UNUSED(newBounds)
		// signal a re-scaling needed on the plot:
		Emit(0, "dataChanged");

		// schedule an update of the plot, but computing a new pixmap is not needed
		update();
	}



protected:

	QImage image_;

};





#endif // MPLOTIMAGE_H
