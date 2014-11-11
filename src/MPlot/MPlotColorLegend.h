#ifndef MPLOTCOLORLEGEND_H
#define MPLOTCOLORLEGEND_H

#include "MPlot/MPlot_global.h"

#include <QGraphicsItem>

class MPlot;
class MPlotItem;
class MPlotAbstractImage;
class MPlotColorLegend;

class MPLOTSHARED_EXPORT MPlotColorLegendSignalHandler : public QObject
{
	Q_OBJECT

protected:
	/// Constructor.  Builds a signal handler for the MPlotImage object.
	MPlotColorLegendSignalHandler(MPlotColorLegend *parent);
	/// Giving access to the MPlotAbstractImage to the signal handler.
	friend class MPlotColorLegend;

protected slots:
	/// Slot that handles updating the data in the the image.
	void onDataChanged();

protected:
	/// Pointer to the image this signal handler manages.
	MPlotColorLegend *legend_;
};

class MPLOTSHARED_EXPORT MPlotColorLegend : public QGraphicsItem
{

public:

	/// Constructor.  Builds a colour legend for the given \param plot.  If there are multiple images stacked on top of each other then this might not be an accurate representation of the colour map.
	MPlotColorLegend(MPlot *plot, QGraphicsItem *parent = 0);

	/// Destructor is responsible for cleaning up the signal handler in particular
	virtual ~MPlotColorLegend();

	/// Pure virtual implementation.  Bounding rectangle.
	virtual QRectF boundingRect() const;
	/// Pure virutal implementation.  The paint function.
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

	/// Set the number of boxes in the color legend..
	void setBoxNumber(int number) { boxNumber_ = number; prepareGeometryChange(); updateBoundingRect(); update(boundingRect_); }
	/// Set the top left position of the color legend.
	void setTopLeft(const QPoint& point) { topLeft_ = point; prepareGeometryChange(); updateBoundingRect(); update(boundingRect_); }
	/// Offset the color legend in the x direction.
	void setHorizontalOffset(qreal x) { topLeft_.setX(int(x)); prepareGeometryChange(); updateBoundingRect(); update(boundingRect_); }
	/// Offset the color legend in the y direction.
	void setVerticalOffset(qreal y) { topLeft_.setY(int(y)); prepareGeometryChange(); updateBoundingRect(); update(boundingRect_); }

protected:
	/// The double click event.
	virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
	/// Updates the bounding rect when things that effect it are changed.
	void updateBoundingRect();

	/// Pointer to the plot the colour legend resides in.
	MPlot *plot_;
	/// Pointer to the image the legend represents.
	MPlotAbstractImage *image_;
	/// Number of boxes in the colour legend.
	int boxNumber_;
	/// The top left point of the color legend.
	QPoint topLeft_;
	/// The bounding rect.
	QRectF boundingRect_;

	/// The signal hander for the image.
	MPlotColorLegendSignalHandler* signalHandler_;
	/// Friending the image handler so it has access to its methods.
	friend class MPlotColorLegendSignalHandler;

private:
	/// Called within the base class to handle the data changed signal from the signal hander.
	void onDataChangedPrivate();
};

#endif // MPLOTCOLORLEGEND_H
