/*
Copyright 2010, 2011 Mark Boots, David Chevrier, and Darren Hunter.

This file is part of the Acquaman Data Acquisition and Management framework ("Acquaman").

Acquaman is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Acquaman is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Acquaman.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef MPLOTMARKERTRANSPARENTVERTICALRECTANGLE_H
#define MPLOTMARKERTRANSPARENTVERTICALRECTANGLE_H

#include "MPlot/MPlot_global.h"

#include "MPlot/MPlotItem.h"

#include <QPen>
#include <QBrush>

class MPLOTSHARED_EXPORT MPlotMarkerTransparentVerticalRectangle : public MPlotItem
{
public:
	/// Constructor.  Requires the original geometry of the ROI to set correctly.
	MPlotMarkerTransparentVerticalRectangle(QString name = "", double center = 0, double low = 0, double high = 0);

	/// Returns the description of the marker.
	QString description() const { return name_; }
	/// Set the description of the marker.
	void setDescription(const QString &description) { name_ = description; }
	/// Returns the position of the center line.
	double center() const { return center_; }
	/// Sets the center position of marker.
	void setCenter(double position) { prepareGeometryChange(); center_ = position; updatePlot(); }
	/// Returns the lower end of the boundingRect.
	double lowEnd() const { return low_; }
	/// Sets the position of the lower end of the boundingRect.
	void setLowEnd(double position) { prepareGeometryChange(); low_ = position; updatePlot(); }
	/// Returns the higher end of the boundingRect.
	double highEnd() const { return high_; }
	/// Sets the position of the higher end of the boundingRect.
	void setHighEnd(double position) { prepareGeometryChange(); high_ = position; updatePlot(); }
	/// Sets whether the marker has been "selected" or not.  Changes the colour of the marker here.
	void setHighlighted(bool highlight);
	/// Returns whether the marker is highlighted or not.
	bool getHighlighted() { return isHighlighted_; }

	/// Returns the pen used to draw the rectangle's outline
	QPen pen() const { return pen_; }
	/// Returns the brush used to fill in the rectangle.
	QBrush brush() const { return brush_; }

	/// Set the pen used to draw the rectangle's outline
	void setPen(const QPen& pen) {
		prepareGeometryChange();
		pen_ = pen;
		update();
	}

	/// Set the brush used to fill in the rectangle.  Try a semi-transparent brush for sexiness.
	void setBrush(const QBrush& brush) {
		brush_ = brush;
		update();
	}

	/// Bounding rect: reported in our PlotItem coordinates, which are just the actual data coordinates. This is used by the graphics view system to figure out how much we cover/need to redraw.  Subclasses that draw selection borders or markers need to add their size on top of this.
	virtual QRectF boundingRect() const;

	/// Data rect: also reported in our PlotItem coordinates, which are the actual data coordinates. This is used by the auto-scaling to figure out the range of our data on an axis.
	virtual QRectF dataRect() const;

	/// Paint: Draws a semi-transparent beam to show the width of a particular region of interest.
	virtual void paint(QPainter* painter,
					   const QStyleOptionGraphicsItem* option,
					   QWidget* widget);

	/// Updates the plot to show the new size and shape of the marker.
	virtual void updatePlot() { emitBoundsChanged(); emitLegendContentChanged(); if (low_ == -1 && high_ == -1) hide(); else show(); }

	/// The color used to represent this plot item in the legend.  Subclasses can re-implement this for more detail.
	virtual QBrush legendColor() const { return QBrush(QColor(0, 0, 0)); }

private:
	QString name_;
	double center_;
	double low_;
	double high_;

	/// If the ROI is "selected" in the ROI list, then the marker is to change colour.  This keeps track to ensure that the colour of the marker is correct.
	bool isHighlighted_;

	// Painting tools.
	QPen pen_;
	QBrush brush_;
};

#endif // MPLOTMARKERTRANSPARENTVERTICALRECTANGLE_H
