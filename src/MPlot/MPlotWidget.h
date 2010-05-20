#ifndef __MPlotWidget_H__
#define __MPlotWidget_H__

#include "MPlotSceneAndView.h"
#include "MPlot.h"

// TODO: test performance of:
// setItemIndexMethod(NoIndex);
// makes a big difference if drawing plots using many separate QGraphicsItem elements (for ex: separate QGraphicsLineItems for each line element in a series)


class MPlotWidget : public MPlotSceneAndView {
    Q_OBJECT

public:
	MPlotWidget(QWidget* parent = 0) : MPlotSceneAndView(parent) {
		// Not holding a plot right now:
		plot_ = 0;
	}
	
	
	virtual ~MPlotWidget() {
	}

	/// Sets the plot attached to this widget. to remove a plot, pass \c plot = 0.
	void setPlot(MPlot* plot) {

		// remove old plot?
		if(plot_)
			scene()->removeItem(plot_);
		// todo: disconnect any signals?

		if(plot) {
			scene()->addItem(plot);
			plot_ = plot;
		}
	}
	
	MPlot* plot() {
		return plot_;
	}
	

	



	
protected:
	// Member variables:
	MPlot* plot_;
	
	// On resize events: notify the plot to resize it, and fill the viewport with the canvas.
	virtual void resizeEvent ( QResizeEvent * event ) {
		MPlotSceneAndView::resizeEvent(event);
		
		if(plot_) {
			plot_->setRect(scene()->sceneRect());
			fitInView(plot_->rect(), Qt::KeepAspectRatioByExpanding);
		}
	}

};

#endif
