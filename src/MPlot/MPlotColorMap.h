#ifndef MPLOTCOLORMAP_H
#define MPLOTCOLORMAP_H

#include <QPair>
#include <QColor>
#include <QDebug>

#include <cmath>

/// only required for MPlotInterval typedef. \todo: move somewhere more appropriate?
#include "MPlotImageData.h"

/// This class defines the interface for color maps, which are used by image plots to turn a z-value into a color.
class MPlotAbstractColorMap {
public:
	///  return a QRgb (unsigned int) representing the color for a given \c value within a \c range. (Faster than returning a full QColor)
	virtual QRgb rgb(double value, MPlotInterval range = MPlotInterval(0.0, 1.0) ) const = 0;
	/// return the color representing a given \c value within a \c range.
	virtual QColor color(double value, MPlotInterval range = MPlotInterval(0, 1) ) const = 0;

};


/// This implementation of a color map linearly interpolates between two colors.
class MPlotLinearColorMap : public MPlotAbstractColorMap {
public:

	/// Constructs a color map with \c start and \c end values.
	MPlotLinearColorMap(const QColor& start = QColor("black"), const QColor& finish = QColor("black"));

	///  return a QRgb (unsigned int) representing the color for a given \c value within a \c range. (Faster than returning a full QColor)
	virtual QRgb rgb(double value, MPlotInterval range = MPlotInterval(0.0, 1.0) ) const;

	/// return the color representing a given \c value within a \c range.
	virtual QColor color(double value, MPlotInterval range = MPlotInterval(0, 1) ) const;


protected:
	QColor start_, finish_;
};



#endif // MPLOTCOLORMAP_H
