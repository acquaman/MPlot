#ifndef __MPlotLegend_H__
#define __MPlotLegend_H__

#include "MPlot/MPlot_global.h"

#include <QGraphicsTextItem>

class MPlot;
class MPlotItem;

class MPLOTSHARED_EXPORT MPlotLegend: public QGraphicsTextItem {

public:
        /// Constructor.  Builds a legend for the given \param plot.
	MPlotLegend(MPlot* plot, QGraphicsItem* parent = 0);

	/// Set the maximum width the legend should take up. (The height is out of your control; text will wrap to determine height)
	void setWidth(qreal width) {
		setTextWidth(width);
	}

	/// Set the font: use QGraphicsTextItem::setFont()

	/// Set the title text color and body text color. You can override this by placing rich text html tags into the strings.
	void setFontColors(const QColor& titleTextColor, const QColor& bodyTextColor) {
		titleTextColor_ = titleTextColor;
		bodyTextColor_ = bodyTextColor;
	}


	/// Set the title (top line) in the legend
	void setTitleText(const QString& titleText) {
		titleText_ = titleText;
		redoText();
	}

	/// Set the body text of the legend
	void setBodyText(const QString& bodyText) {
		bodyText_ = bodyText;
		redoText();
	}

	/// Show or hide the default legend content (ie: list of items on the plot)
	void enableDefaultLegend(bool defaultLegendEnabled = true) {
		defaultLegendEnabled_ = defaultLegendEnabled;
		redoText();
	}

	/// Trigger an update to the legend:
	void onLegendContentChanged(MPlotItem* changedItem = 0);

protected:
        /// Method that rewrites all the text in the legend.
        void redoText();

        /// String holding the title, body and full text for the legend.
	QString titleText_, bodyText_, fullText_;
        /// Holding the color for the title and the body of the legend.
	QColor titleTextColor_, bodyTextColor_;

        /// Pointer to the plot the legend resides inside.
	MPlot* plot_;
        /// Bool holding whether or not a default legend look is used.
	bool defaultLegendEnabled_;
};

#endif
