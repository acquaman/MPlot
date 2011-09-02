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
        /// Constructor.  Builds a signal handler for the MPlotImage object.
	MPlotImageSignalHandler(MPlotAbstractImage* parent);
        /// Giving access to the MPlotAbstractImage to the signal handler.
	friend class MPlotAbstractImage;

protected slots:
        /// Slot that handles updating the data in the the image.
	void onDataChanged();
        /// Slot that handles updating the bounds of the image.
	void onBoundsChanged();

protected:
        /// Pointer to the image this signal handler manages.
	MPlotAbstractImage* image_;
};

/// This class represents a plot item that represents a function z = f(x, y) as a color map / image
class MPlotAbstractImage : public MPlotItem {

public:
        /// An enum that defines the Type as an MPlotItem::Image.
	enum { Type = MPlotItem::Image };
        /// Returns the type cast to an int.
	virtual int type() const { return Type; }
        /// Returns the rank.  Should be 2 because an image has two dimension.
	virtual int rank() const { return 2; }

        /// Constructor.
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

        /// Returns the data model for this image.
	virtual const MPlotAbstractImageData* model() const;


	// Required functions:
	//////////////////////////

	/// Data rect: reported in actual data coordinates. This is used by the auto-scaling to figure out the range of our data on an axis.
	virtual QRectF dataRect() const;

protected:
        /// Helper function that sets some defaults for the image.
        virtual void setDefaults();

        /// When the z-data changes, this is called to allow an update:
        virtual void onDataChanged() = 0;
        /// When the bounds change, this is called to allow whatever needs to happen for computing a new raster grid, etc.
        virtual void onBoundsChanged(const QRectF& newBounds) = 0;

        /// Pointer to the data model.
	const MPlotAbstractImageData* data_;
        /// Bool for determining if the image owns the model or not.
	bool ownsModel_;

        /// The color map used to paint the image.
	MPlotColorMap map_;

        /// The signal hander for the image.
	MPlotImageSignalHandler* signalHandler_;
        /// Friending the image handler so it has access to its methods.
	friend class MPlotImageSignalHandler;

private:
	/// This is called within the base class to handle signals from the signal handler
	void onBoundsChangedPrivate();
        /// Called within the base class to handle the data changed signal from the signal hander.
	void onDataChangedPrivate();

};


/// This class implements an image (2d intensity plot), using a cached, scaled QPixmap for drawing
class MPlotImageBasic : public MPlotAbstractImage {

public:
	/// Constructor
	MPlotImageBasic(const MPlotAbstractImageData* data = 0);

        /// The paint function.  Paints the image.
	virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

	/// boundingRect: using parent implementation, but adding extra room on edges for our selection highlight.
	virtual QRectF boundingRect() const;


protected:	// "slots"
	/// Called when the z-data changes, so that the plot needs to be updated. This fills the pixmap buffer
	virtual void onDataChanged();

	/// If the bounds of the data change (in x- and y-) this might require re-auto-scaling of a plot.
	virtual void onBoundsChanged(const QRectF& newBounds);

protected:
        /// The variable that holds the image.
	QImage image_;

	/// indicates that the data has changed, and that the image_ cache is out of date. re-filling the image_ from the data is necessary before redrawing
	bool imageRefillRequired_;

	/// helper function to fill image_ based on the data
	void fillImageFromData();
};

#endif // MPLOTIMAGE_H
