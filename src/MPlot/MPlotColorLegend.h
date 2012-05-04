#ifndef MPLOTCOLORLEGEND_H
#define MPLOTCOLORLEGEND_H

#include <QGraphicsItem>

class MPlot;
class MPlotItem;

class MPlotColorLegend : public QGraphicsItem
{

public:

	/// Constructor.  Builds a colour legend for the given \param plot.  If there are multiple images stacked on top of each other then this might not be an accurate representation of the colour map.
	MPlotColorLegend(MPlot *plot, QGraphicsItem *parent = 0);

	/// Pure virtual implementation.  Bounding rectangle.
	virtual QRectF boundingRect() const;
	/// Pure virutal implementation.  The paint function.
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

protected:
	/// Pointer to the plot the colour legend resides in.
	MPlot *plot_;
};

#endif // MPLOTCOLORLEGEND_H
