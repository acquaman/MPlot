#ifndef MPLOTIMAGERANGEDIALOG_H
#define MPLOTIMAGERANGEDIALOG_H

#include <QDialog>

#include "MPlot/MPlotImage.h"

class QLineEdit;

/// This dialog is built for changing the minimum and maximum z-values for MPlotImages.
class MPlotImageRangeDialog : public QDialog
{
	Q_OBJECT

public:
	/// Constructor.
	MPlotImageRangeDialog(MPlotAbstractImage *image, QWidget *parent = 0);

protected slots:
	/// Helper slot that handles the constrain range to data.
	void onConstrainRangeToData(bool constrain);
	/// Helper slot that handles when the clear min button is clicked.
	void onClearMinimumClicked();
	/// Helper slot that handles when the clear max button is clicked.
	void onClearMaximumClicked();
	/// Helper slot that handles when clear the range button is clicked.
	void onClearClicked();
	/// Helper slot that handles setting the minimum value manually.
	void onManualMinimumChanged();
	/// Helper slot that handles setting the maximum value manually.
	void onManualMaximumChanged();

protected:
	/// The pointer to the image.
	MPlotAbstractImage *image_;
	/// The spin box holding the minimum value.
	QLineEdit *minimum_;
	/// The spin box holding the maximum value.
	QLineEdit *maximum_;
};

#endif // MPLOTIMAGERANGEDIALOG_H
