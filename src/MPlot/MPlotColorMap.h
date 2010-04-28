#ifndef MPLOTCOLORMAP_H
#define MPLOTCOLORMAP_H

#include <QPair>
#include <QColor>

/// only required for MPlotInterval typedef. \todo: move somewhere more appropriate?
#include "MPlotImageData.h"

/// This class defines the interface for color maps, which are used by image plots to turn a z-value into a color.
class MPlotAbstractColorMap {
public:
	///  return a QRgb (unsigned int) representing the color for a given \c value within a \c range. (Faster than returning a full QColor)
	virtual QRgb rgb(double value, MPlotInterval range = MPlotInterval(0.0, 1.1) ) const = 0;
	/// return the color representing a given \c value within a \c range.
	virtual QColor color(double value, MPlotInterval range = MPlotInterval(0, 1) ) const = 0;

};







/// This implementation of a color map linearly interpolates between two colors.
class MPlotLinearColorMap : public MPlotAbstractColorMap {
public:

	/// Constructs a color map with \c start and \c end values.
	MPlotLinearColorMap(const QColor& start = QColor("black"), const QColor& finish = QColor("black"))
		: start_(start),
		finish_(finish) {}

	///  return a QRgb (unsigned int) representing the color for a given \c value within a \c range. (Faster than returning a full QColor)
	virtual QRgb rgb(double value, MPlotInterval range = MPlotInterval(0.0, 1.1) ) const {

		double Ratio = (value - range.first)/(range.second - range.first);
		QRgb Col1 = start_.rgba();
		QRgb Col2 = finish_.rgba();

		int A1=qAlpha(Col1), R1=qRed(Col1), G1=qGreen(Col1), B1=qBlue(Col1);
		int A2=qAlpha(Col2), R2=qRed(Col2), G2=qGreen(Col2), B2=qBlue(Col2);
		//int A2=(Col2>>24)&0xFF, R2=(Col2>>16)&0xFF, G2=(Col2>>8)&0xFF, B2=Col2&0xFF;
		int Color;
		if (Ratio<=0) return Col1;	// Ratio parameter must be between 0 and 1
		else if (Ratio>=1) return Col2;
		Color=	(int)(A1+(A2-A1)*Ratio+0.5)<<24 |
				(int)(R1+(R2-R1)*Ratio+0.5)<<16 |		// rounding
				(int)(G1+(G2-G1)*Ratio+0.5)<<8  |
				(int)(B1+(B2-B1)*Ratio+0.5);
		return Color;

	}
	/// return the color representing a given \c value within a \c range.
	virtual QColor color(double value, MPlotInterval range = MPlotInterval(0, 1) ) const {
		return QColor::fromRgba(rgb(value, range));
	}


protected:
	QColor start_, finish_;
};


#endif // MPLOTCOLORMAP_H
