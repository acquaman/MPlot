#ifndef MPLOTIMAGE_H
#define MPLOTIMAGE_H

#include "MPlotImageData.h"
#include "MPlotColorMap.h"
#include "MPlotItem.h"


class MPlotAbstractImage;

/// This class receives and processes signals for MPlotAbstractImage. You should never need to use it directly.
/*! To avoid multiple-inheritance restrictions, MPlotAbstractImage does not inherit from QObject.  However, it needs a way to receive signals from MPlotAbstractImageData. This proxy signal handling is enabled by this class.*/
class MPlotImageSignalHandler : public QObject {
	Q_OBJECT
protected:
	MPlotImageSignalHandler(MPlotAbstractImage* parent);
	friend class MPlotAbstractImage;

protected slots:
	void onDataChanged();
	void onBoundsChanged();

protected:
	MPlotAbstractImage* image_;
};

/// This class represents a plot item that represents a function z = f(x, y) as a color map / image
class MPlotAbstractImage : public MPlotItem {

public:

	enum { Type = MPlotItem::Image };
	virtual int type() const { return Type; }
	virtual int rank() const { return 2; }

	MPlotAbstractImage();

	/// The destructor deletes the model if its been set with \c ownsModel = true in setModel().
	virtual ~MPlotAbstractImage();



	// Properties:
	/// Set the color map, used to convert numeric values into pixel colors.
	virtual void setColorMap(const MPlotColorMap& map);

	/// Returns the active color map.
	virtual MPlotColorMap colorMap() const;

	/// Sets this series to view the model in 'data'.  If the image should delete the model when it gets deleted, set \c ownsModel to true.  (If there was a previous model, and \c ownsModel was set for it, this function will delete the old model.)
	virtual void setModel(const MPlotAbstractImageData* data, bool ownsModel = false);

	virtual const MPlotAbstractImageData* model() const;


	// Required functions:
	//////////////////////////

	/// Data rect: reported in actual data coordinates. This is used by the auto-scaling to figure out the range of our data on an axis.
	virtual QRectF dataRect() const;




protected:

	const MPlotAbstractImageData* data_;
	bool ownsModel_;

	MPlotColorMap map_;

	virtual void setDefaults();

	/// When the z-data changes, this is called to allow an update:
	virtual void onDataChanged() = 0;
	/// When the bounds change, this is called to allow whatever needs to happen for computing a new raster grid, etc.
	virtual void onBoundsChanged(const QRectF& newBounds) = 0;


	MPlotImageSignalHandler* signalHandler_;
	friend class MPlotImageSignalHandler;

private:
	/// This is called within the base class to handle signals from the signal handler
	void onBoundsChangedPrivate();
	void onDataChangedPrivate();

};


/// This class implements an image (2d intensity plot), using a cached, scaled QPixmap for drawing
class MPlotImageBasic : public MPlotAbstractImage {

public:
	/// Constructor
	MPlotImageBasic(const MPlotAbstractImageData* data = 0);


	virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

	/// boundingRect: using parent implementation, but adding extra room on edges for our selection highlight.
	virtual QRectF boundingRect() const;


protected:	// "slots"
	/// Called when the z-data changes, so that the plot needs to be updated. This fills the pixmap buffer
	virtual void onDataChanged();

	/// If the bounds of the data change (in x- and y-) this might require re-auto-scaling of a plot.
	virtual void onBoundsChanged(const QRectF& newBounds);

protected:

	QImage image_;

	/// indicates that the data has changed, and that the image_ cache is out of date. re-filling the image_ from the data is necessary before redrawing
	bool imageRefillRequired_;

	/// helper function to fill image_ based on the data
	void fillImageFromData();


};





#endif // MPLOTIMAGE_H
