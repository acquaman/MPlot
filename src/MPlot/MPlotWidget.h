#ifndef __MPlotWidget_H__
#define __MPlotWidget_H__

#include "MPlot/MPlot_global.h"

#include <QGraphicsView>

#ifdef MPLOT_PRAGMA_WARNING_CONTROLS
#pragma clang diagnostic ignored "-Wunused-private-field"
#endif
#include <QResizeEvent>
#ifdef MPLOT_PRAGMA_WARNING_CONTROLS
#pragma clang diagnostic warning "-Wunused-private-field"
#endif

#include "MPlot/MPlot.h"


// TODO: test performance of:
// setItemIndexMethod(NoIndex);
// makes a big difference if drawing plots using many separate QGraphicsItem elements (for ex: separate QGraphicsLineItems for each line element in a series)

/// This class holds the scene and view for MPlot.  Everything inside the plot will be part of the MPlot scene and viewed through this class.
class MPLOTSHARED_EXPORT MPlotSceneAndView : public QGraphicsView {
	Q_OBJECT

public:
	/// Constructor.  Builds the scene and view that the plot will reside in.
	MPlotSceneAndView(QWidget* parent = 0);

	/// Setter to enable/disable anti-aliasing.
	void enableAntiAliasing(bool antiAliasingOn = true);
	/// Destructor.
	virtual ~MPlotSceneAndView();

protected:

	/// On resize events: keep the scene the same size as the view, and make the view look at this part of the scene.
	virtual void resizeEvent ( QResizeEvent * event );

};

/// This class is the widget that holds the plot.  It extends the standard scene and view requirements of basic visualization for the plot to be visualized properly.
class MPLOTSHARED_EXPORT MPlotWidget : public MPlotSceneAndView {
	Q_OBJECT

public:
	/// Constructor.  Builds the scene and view, and sets up everything to make viewing a plot possible.
	MPlotWidget(QWidget* parent = 0);
	/// Destructor. If there is a current plot(), it will be deleted automatically because it is an item within our scene.
	virtual ~MPlotWidget();

	/// Sets the plot displayed by this widget. To remove a plot, pass \c plot = 0.
	void setPlot(MPlot* plot);

	/// Returns a pointer to the plot that this widget is displaying, or 0 if none.
	MPlot* plot();

protected:

	// On resize events: notify the plot to resize it, and fill the viewport with the canvas.
	virtual void resizeEvent ( QResizeEvent * event );

	// Member variables:
	MPlot* plot_;
};

#endif
