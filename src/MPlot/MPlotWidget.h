#ifndef __MPlotWidget_H__
#define __MPlotWidget_H__

#include <QGraphicsView>
#include <QResizeEvent>
#include "MPlot.h"


// TODO: test performance of:
// setItemIndexMethod(NoIndex);
// makes a big difference if drawing plots using many separate QGraphicsItem elements (for ex: separate QGraphicsLineItems for each line element in a series)


class MPlotSceneAndView : public QGraphicsView {
	Q_OBJECT

public:
	MPlotSceneAndView(QWidget* parent = 0);

	void enableAntiAliasing(bool antiAliasingOn = true);


	virtual ~MPlotSceneAndView();

protected:

	// On resize events: keep the scene the same size as the view, and make the view look at this part of the scene.
	virtual void resizeEvent ( QResizeEvent * event );

};


class MPlotWidget : public MPlotSceneAndView {
	Q_OBJECT

public:
	MPlotWidget(QWidget* parent = 0);


	virtual ~MPlotWidget();

	/// Sets the plot attached to this widget. to remove a plot, pass \c plot = 0.
	void setPlot(MPlot* plot);

	MPlot* plot();

protected:
	// Member variables:
	MPlot* plot_;

	// On resize events: notify the plot to resize it, and fill the viewport with the canvas.
	virtual void resizeEvent ( QResizeEvent * event );

};

#endif
