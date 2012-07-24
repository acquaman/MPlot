#ifndef MPLOTIMAGE_CPP
#define MPLOTIMAGE_CPP

#include "MPlot/MPlotImage.h"
#include <QPainter>

MPlotImageSignalHandler::MPlotImageSignalHandler(MPlotAbstractImage *parent)
	: QObject(0) {
	image_ = parent;
}

void MPlotImageSignalHandler::onBoundsChanged() {
	image_->onBoundsChangedPrivate();
}

void MPlotImageSignalHandler::onDataChanged() {
	image_->onDataChangedPrivate();
}

MPlotAbstractImage::MPlotAbstractImage()
	: MPlotItem()
{

	signalHandler_ = new MPlotImageSignalHandler(this);

	data_ = 0;

	// Set style defaults:
	setDefaults();	// override in subclasses for custom appearance

}

MPlotAbstractImage::~MPlotAbstractImage() {
	if(data_) {
		QObject::disconnect(data_->signalSource(), 0, signalHandler_, 0);
		if(ownsModel_) {
			delete data_;
			data_ = 0;
		}
	}

	delete signalHandler_;
	signalHandler_ = 0;
}



// Properties:

// Set the color map, used to convert numeric values into pixel colors.
void MPlotAbstractImage::setColorMap(const MPlotColorMap &map) {

	map_ = map;
	onDataChanged();
}

// Returns the active color map.
MPlotColorMap MPlotAbstractImage::colorMap() const {
	return map_;
}

// Sets this series to view the model in 'data';
void MPlotAbstractImage::setModel(const MPlotAbstractImageData* data, bool ownsModel) {

	// efficiency check: if new model is the same one as old model, don't change anything.
	if(data == data_) {
		ownsModel_ = ownsModel;
		return;
	}

	// Changing models.

	// If there was an old model, disconnect old signals, and delete if required
	if(data_) {
		QObject::disconnect(data_->signalSource(), 0, signalHandler_, 0);
		if(ownsModel_)
			delete data_;
	}

	// new data from here:
	data_ = data;
	ownsModel_ = ownsModel;

	// If there's a new valid model:
	if(data_) {
		QObject::connect(data_->signalSource(), SIGNAL(dataChanged()), signalHandler_, SLOT(onDataChanged()));
		QObject::connect(data_->signalSource(), SIGNAL(boundsChanged()), signalHandler_, SLOT(onBoundsChanged()));
	}


	onBoundsChanged(data_ ? data_->boundingRect() : QRectF());
	onDataChanged();
	emitBoundsChanged();

}

const MPlotAbstractImageData* MPlotAbstractImage::model() const {
	return data_;
}

// Required functions:
//////////////////////////

// Data rect: also reported in PlotItem coordinates, which are the actual data coordinates. This is used by the auto-scaling to figure out the range of our data on an axis.
QRectF MPlotAbstractImage::dataRect() const {
	if(data_)
		return data_->boundingRect();
	else
		return QRectF();
}

void MPlotAbstractImage::onBoundsChangedPrivate() {
	onBoundsChanged(data_? data_->boundingRect() : QRectF());
	emitBoundsChanged();
}

void MPlotAbstractImage::onDataChangedPrivate() {
	onDataChanged();
}

void MPlotAbstractImage::setDefaults() {

	map_ = MPlotColorMap::Jet;
}



// This class implements an image (2d intensity plot), using a cached, scaled QImage for drawing

// Constructor
MPlotImageBasic::MPlotImageBasic(const MPlotAbstractImageData* data)
	: MPlotAbstractImage(),
	  image_(1,1, QImage::Format_ARGB32)
{
	imageRefillRequired_ = true;
	setModel(data);
}

// Paint: must be implemented in subclass.
void MPlotImageBasic::paint(QPainter* painter,
							const QStyleOptionGraphicsItem* option,
							QWidget* widget) {
	Q_UNUSED(option)
	Q_UNUSED(widget)

	if(!yAxisTarget() || !xAxisTarget()) {
		qWarning() << "MPlotImageBasic: No axis scale set. Abandoning painting because we don't know what scale to use.";
		return;
	}


	if(data_) {

		if(imageRefillRequired_)
			fillImageFromData();

		// the MPlotItem implementation of boundingRect() takes our dataRect() and maps it to drawing coordinates... This is where we need to draw into.
		QRectF destinationRect = MPlotItem::boundingRect();
		painter->drawImage(destinationRect, image_, QRectF(QPointF(0,0), QSizeF(data_->size())));

		if(selected()) {
			QColor selectionColor(MPLOT_SELECTION_COLOR);
			QPen selectionPen(selectionColor, MPLOT_SELECTION_LINEWIDTH);
			painter->setPen(selectionPen);
			selectionColor.setAlphaF(MPLOT_SELECTION_OPACITY);
			painter->setBrush(selectionColor);
			painter->drawRect(destinationRect);
		}
	}

	/// \todo selection border
}

// boundingRect: reported in PlotItem coordinates, which are just the actual data coordinates.
// using parent implementation, but adding extra room on edges for our selection highlight.
QRectF MPlotImageBasic::boundingRect() const {
	QRectF br = MPlotAbstractImage::boundingRect();
	if(br.isValid()) {
		// create rectangle at least as big as our selection highlight, and if we have a marker, the marker size.
		QRectF hs = QRectF(0, 0, MPLOT_SELECTION_LINEWIDTH, MPLOT_SELECTION_LINEWIDTH);

		// these sizes so far are in pixels (hopefully scene coordinates... trusting on an untransformed view.) Converting to local coordinates.
		// UNECESSARY in the new coordinate system: hs = mapRectFromScene(hs);

		// really we just need 1/2 the marker size and 1/2 the selection highlight width. But extra doesn't hurt.
		br.adjust(-hs.width(),-hs.height(),hs.width(), hs.height());
	}
	return br;
}



// Called when the z-data changes, so that the plot needs to be updated. This fills the pixmap buffer
// \todo CRITICAL TODO: only schedule this, and perform it the next time it needs to be drawn... Don't re-compute the image every time.
void MPlotImageBasic::onDataChanged() {

	// flag the image as dirty; this avoids the expensive act of re-filling the image every time the data changes, if we're not re-drawing as fast as the data is changing.
	imageRefillRequired_ = true;

	// schedule a draw update
	update();

}


void MPlotImageBasic::fillImageFromData() {

	if(data_) {

//		QTime runTime;
//		runTime.start();
//		qDebug() << "MPlotImageBasic: fillImageFromData()";

		imageRefillRequired_ = false;

		// resize if req'd:
		QSize dataSize = data_->size();

		if(image_.size() != dataSize)
			image_ = QImage(dataSize, QImage::Format_ARGB32);
//		qDebug() << "   QImage resize time:" << runTime.restart();

		int yHeight = dataSize.height();
		int xWidth = dataSize.width();

//		qDebug() << "   data source get height, width:" << runTime.restart();

		if(xWidth > 0 && yHeight > 0) {
			QVector<qreal> dataBuffer(xWidth*yHeight);
//			qDebug() << "   vector creation time:" << runTime.restart();
			data_->zValues(0, 0, xWidth-1, yHeight-1, dataBuffer.data());
//			qDebug() << "   block data access time:" << runTime.restart();

			qreal minZ, maxZ;
			minZ = maxZ = dataBuffer.at(0);
			foreach(qreal d, dataBuffer) {
				if(d<minZ) minZ = d;
				if(d>maxZ) maxZ = d;
			}
			MPlotInterval range(minZ,maxZ);
//			qDebug() << "   Manually search range:" << runTime.restart() << range;

			for(int xx=0; xx<xWidth; ++xx) {
				int xc = xx*yHeight;
				for(int yy=0; yy<yHeight; ++yy)
					image_.setPixel(xx, yHeight-1-yy, map_.rgbAt(dataBuffer.at(xc+yy), range));// note the inversion here. It's necessary because we'll be painting in graphics drawing coordinates.
			}
//			qDebug() << "   rgb conversion time:" << runTime.restart();
		}
	}
}



// If the bounds of the data change (in x- and y-) this might require re-auto-scaling of a plot.
void MPlotImageBasic::onBoundsChanged(const QRectF& newBounds) {
	Q_UNUSED(newBounds)
	// signal a re-scaling needed on the plot: (REDUNDANT... already done in base class)
	// signalSource()->emitBoundsChanged();

	// schedule an update of the plot, but computing a new pixmap is not needed
	update();
}

// MPlotImageBasicwDefault
//////////////////////////////////////

MPlotImageBasicwDefault::MPlotImageBasicwDefault(const MPlotAbstractImageData *data, QColor defaultImageColor)
	: MPlotImageBasic(data)
{
	defaultColor_ = defaultImageColor;
	defaultValue_ = 0;
}

void MPlotImageBasicwDefault::fillImageFromData()
{
	if(data_) {
		imageRefillRequired_ = false;

		// resize if req'd:
		QSize dataSize = data_->size();

		if(image_.size() != dataSize)
			image_ = QImage(dataSize, QImage::Format_ARGB32);

		int yHeight = dataSize.height();
		int xWidth = dataSize.width();
		MPlotInterval range = data_->range();

		// This is slower than MPlotImageBasic because of the check that needs to be done for every pixel.
		for(int yy=0; yy<yHeight; yy++)
			for(int xx=0; xx<xWidth; xx++){

				if (data_->z(QPoint(xx, yy)) == -1 || data_->z(QPoint(xx, yy)) == defaultValue_ || (data_->z(QPoint(xx, yy)) < range.first || data_->z(QPoint(xx, yy)) > range.second))
					image_.setPixel(xx, yHeight-1-yy, defaultColor().rgb());

				else
					image_.setPixel(xx, yHeight-1-yy, map_.rgbAt(data_->z(QPoint(xx,yy)), data_->range()));	// note the inversion here. It's necessary because we'll be painting in graphics drawing coordinates.
			}
	}
}

#endif // MPLOTIMAGE_H

