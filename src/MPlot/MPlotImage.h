#ifndef MPLOTIMAGE_H
#define MPLOTIMAGE_H

#include "MPlot/MPlot_global.h"

#include "MPlot/MPlotImageData.h"
#include "MPlot/MPlotColorMap.h"
#include "MPlot/MPlotItem.h"


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
class MPLOTSHARED_EXPORT MPlotAbstractImage : public MPlotItem {

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
	/// Returns the range of the image.  May be the same as the MPlotAbstractImageData::range(), but not guaranteed.
	MPlotInterval range() const;
	/// Sets the minimum value of the image.
	void setMinimum(qreal min);
	/// Sets the maximum value of the image.
	void setMaximum(qreal max);
	/// Sets the flag that determines if the minimum and maximum must be confined to the data or not.
	void setConstrainToData(bool constrain);
	/// Returns the state of the flag that determines if the minimum and maximum must be confined to the data or not.
	bool constrainToData() const;
	/// Clears the flag associated with the minimum range.  The min will now use the actual minimum of the data.
	void clearMinimum();
	/// Clears the flag assocated with the maximum range.  The max will now use the actual maximum of the data.
	void clearMaximum();
	/// Clears both minimum and maximum flags for the range.  This brings it back to the default where the min and max follow the data.
	void clearRange();

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
	/// Virtual helper method to help notify that the image needs to be repainted.
	virtual void repaintRequired() = 0;

	/// Pointer to the data model.
	const MPlotAbstractImageData* data_;
	/// Bool for determining if the image owns the model or not.
	bool ownsModel_;

	/// The color map used to paint the image.
	MPlotColorMap map_;
	/// The range that the image will base the color around.
	MPlotInterval range_;
	/// The flag that holds whether we need to update the minimum range.
	bool manualMinimum_;
	/// The flag that holds whether we need to update the maxium range.
	bool manualMaximum_;
	/// The flag that holds whether the image constrains the range to the data.
	bool constrainToData_;

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


/// This class implements an image (2d intensity plot), using a cached, scaled QImage for drawing
class MPLOTSHARED_EXPORT MPlotImageBasic : public MPlotAbstractImage {

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
	/// Virtual helper method to help notify that the image needs to be repainted.
	virtual void repaintRequired();


protected:
		/// The variable that holds the image.
	QImage image_;

	/// indicates that the data has changed, and that the image_ cache is out of date. re-filling the image_ from the data is necessary before redrawing
	bool imageRefillRequired_;

	/// helper function to fill image_ based on the data
	virtual void fillImageFromData();
};

/// This class is a simple extension to MPlotImageBasic where you can define a colour for pixels that are invalid (ie: not range.min <= z <= range.max).  The default is white, but can be customized.
/*!
	The default is for an intensity plot.  Since intensities will only be positive numbers, the default colour is used for the following situations:

	1) If somehow the z value is not in the range of the data.
	2) If the z value is invalid.  In these cases, the model returns -1.

	However, the default value can be set to anything depending on the circumstances you wish to plot.
  */
class MPLOTSHARED_EXPORT MPlotImageBasicwDefault : public MPlotImageBasic
{

public:
	/// Constructor.  Takes an MPlotAbstractImageData pointer.
	MPlotImageBasicwDefault(const MPlotAbstractImageData *data = 0, QColor defaultImageColor = Qt::white);

	/// Sets the default value.  This is the value associated with the default colour.
	void setDefaultValue(qreal val) { defaultValue_ = val; onDataChanged(); }
	/// Returns the default value.
	qreal defaultValue() const { return defaultValue_; }
	/// Returns the default colour.
	QColor defaultColor() const { return defaultColor_; }
	/// Sets the default color.
	void setDefaultColor(QColor color) { defaultColor_ = color; onDataChanged(); }

protected:
	/// Reimplemented to utilize the default color.  Fills image_ based on the data.
	virtual void fillImageFromData();

	/// The default color.
	QColor defaultColor_;
	/// The default value.
	qreal defaultValue_;
};

#endif // MPLOTIMAGE_H
