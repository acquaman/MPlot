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

	MPlotAbstractImage(const MPlotAbstractImageData* data = 0);

	virtual ~MPlotAbstractImage();



	// Properties:
	/// Set the color map, used to convert numeric values into pixel colors. \c map must be a reference to a color map that exists elsewhere, and must exist as long as it is set. (We don't make a copy of the map).
	virtual void setColorMap(MPlotAbstractColorMap* map);

	/// Returns a reference to the active color map.
	virtual MPlotAbstractColorMap* colorMap();

	// Sets this series to view the model in 'data';
	virtual void setModel(const MPlotAbstractImageData* data);

	virtual const MPlotAbstractImageData* model() const;


	// Required functions:
	//////////////////////////
	/// Bounding rect: reported in PlotItem coordinates, which are just the actual data coordinates. This is used by the graphics view system to figure out how much we cover/need to redraw.  Subclasses that draw selection borders or markers need to add their size on top of this.
	virtual QRectF boundingRect() const;

	/// Data rect: also reported in PlotItem coordinates, which are the actual data coordinates. This is used by the auto-scaling to figure out the range of our data on an axis.
	virtual QRectF dataRect() const;




protected:

	const MPlotAbstractImageData* data_;

	MPlotLinearColorMap defaultColorMap_;
	MPlotAbstractColorMap* map_;

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

	/// Sets this plot item to view the model in 'data';
	virtual void setModel(const MPlotAbstractImageData* data);

	/// Paint: must be implemented in subclass.
	virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

	// boundingRect: reported in PlotItem coordinates, which are just the actual data coordinates.
	// using parent implementation, but adding extra room on edges for our selection highlight.
	virtual QRectF boundingRect() const;


protected:
	/// Called when the z-data changes, so that the plot needs to be updated. This fills the pixmap buffer
	virtual void onDataChanged();

	/// If the bounds of the data change (in x- and y-) this might require re-auto-scaling of a plot.
	virtual void onBoundsChanged(const QRectF& newBounds);

protected:

	QImage image_;

};





#endif // MPLOTIMAGE_H
