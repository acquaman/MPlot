#include "MPlotImageRangeDialog.h"

#include <QPushButton>
#include <QLineEdit>
#include <QCheckBox>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>

MPlotImageRangeDialog::MPlotImageRangeDialog(MPlotAbstractImage *image, QWidget *parent)
	: QDialog(parent)
{
	image_ = image;

	minimum_ = new QLineEdit;
	minimum_->setText(QString::number(image_->range().first, 'g', 3));

	maximum_ = new QLineEdit;
	maximum_->setText(QString::number(image_->range().second, 'g', 3));

	QPushButton *clearRangeButton = new QPushButton("Reset Range");
	QPushButton *clearMinimumButton = new QPushButton("Reset Minimum");
	QPushButton *clearMaximumButton = new QPushButton("Reset Maximum");

	QCheckBox *constrainCheckBox = new QCheckBox("Constrain range to data");
	constrainCheckBox->setChecked(image_->constrainToData());

	QHBoxLayout *minimumLayout = new QHBoxLayout;
	minimumLayout->addWidget(new QLabel("Minimum:"));
	minimumLayout->addWidget(minimum_);
	minimumLayout->addWidget(clearMinimumButton);

	QHBoxLayout *maximumLayout = new QHBoxLayout;
	maximumLayout->addWidget(new QLabel("Maximum:"));
	maximumLayout->addWidget(maximum_);
	maximumLayout->addWidget(clearMaximumButton);

	QVBoxLayout *dialogLayout = new QVBoxLayout;
	dialogLayout->addWidget(constrainCheckBox, 0, Qt::AlignLeft);
	dialogLayout->addLayout(minimumLayout);
	dialogLayout->addLayout(maximumLayout);
	dialogLayout->addWidget(clearRangeButton, 0, Qt::AlignRight);

	setLayout(dialogLayout);
}

void MPlotImageRangeDialog::onClearClicked()
{
	image_->clearRange();
}

void MPlotImageRangeDialog::onClearMinimumClicked()
{
	image_->clearMinimum();
}

void MPlotImageRangeDialog::onClearMaximumClicked()
{
	image_->clearMaximum();
}

void MPlotImageRangeDialog::onConstrainRangeToData(bool constrain)
{
	image_->setConstrainToData(constrain);
}

void MPlotImageRangeDialog::onManualMinimumChanged()
{
	image_->setMinimum(minimum_->text().toDouble());
}

void MPlotImageRangeDialog::onManualMaximumChanged()
{
	image_->setMaximum(maximum_->text().toDouble());
}
