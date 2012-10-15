#ifndef __MPlotLegend_CPP__
#define __MPlotLegend_CPP__

#include "MPlot/MPlotLegend.h"
#include "MPlot/MPlot.h"

MPlotLegend::MPlotLegend(MPlot* plot, QGraphicsItem* parent) : QGraphicsTextItem(parent) {
	plot_ = plot;
	titleTextColor_ = QColor(Qt::black);
	bodyTextColor_ = QColor(121, 121, 121);
	defaultLegendEnabled_ = true;
}

void MPlotLegend::redoText() {

	fullText_ = QString();

	if(!titleText_.isEmpty()) {
		fullText_.append(QString("<p align=right><font color=#%1%2%3 size=+1>")
					.arg(titleTextColor_.red(), 2, 16, QChar('0'))
					.arg(titleTextColor_.green(), 2, 16, QChar('0'))
					.arg(titleTextColor_.blue(), 2, 16, QChar('0')));
		fullText_.append(titleText_);
		fullText_.append("</font><br>");
	}
	if(!bodyText_.isEmpty()) {
		fullText_.append(QString("<font color=#%1%2%3 size=-1>")
						 .arg(bodyTextColor_.red(), 2, 16, QChar('0'))
						 .arg(bodyTextColor_.green(), 2, 16, QChar('0'))
						 .arg(bodyTextColor_.blue(), 2, 16, QChar('0')));
		fullText_.append(bodyText_);
		fullText_.append("</font><br>");
	}

	if(defaultLegendEnabled_ && plot_) {

		for(int i=0; i<plot_->numItems(); i++) {

			if (plot_->item(i)->legendVisibility()){

				QString description = plot_->item(i)->description();
				if(description.isEmpty())
					description = QString("Item %1").arg(i);

				QColor color = plot_->item(i)->legendColor().color();
				fullText_.append(QString("<font color=#%1%2%3 size=-1>%4</font><br>")
								 .arg(color.red(), 2, 16, QChar('0'))
								 .arg(color.green(), 2, 16, QChar('0'))
								 .arg(color.blue(), 2, 16, QChar('0'))
								 .arg(description));
			}
		}

	}


	fullText_.append("</p>");

	setHtml(fullText_);
}

void MPlotLegend::onLegendContentChanged(MPlotItem *changedItem) {
	Q_UNUSED(changedItem)

	if(defaultLegendEnabled_)
		redoText();

	/// \todo: optimize: do this only once when going back to the event loop?

}

#endif
