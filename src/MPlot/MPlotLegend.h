#ifndef __MPlotLegend_H__
#define __MPlotLegend_H__

#include <QGraphicsTextItem>

class MPlot;
class MPlotItem;

class MPlotLegend: public QGraphicsTextItem {

public:
	MPlotLegend(MPlot* plot, QGraphicsItem* parent = 0);

	/// Set the maximum width the legend should take up. (The height is out of your control; text will wrap to determine height)
	void setWidth(double width) {
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
	void onLegendContentChanged(MPlotItem* changedItem);

protected:
	QString titleText_, bodyText_, fullText_;
	QColor titleTextColor_, bodyTextColor_;

	MPlot* plot_;
	bool defaultLegendEnabled_;

	void redoText();

};

#endif
