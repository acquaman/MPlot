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

	MPlotRange range = image_->range();

	minimum_ = new QLineEdit;
	minimum_->setText(QString::number(range.x(), 'g', 3));

	maximum_ = new QLineEdit;
	maximum_->setText(QString::number(range.y(), 'g', 3));

	QPushButton *clearRangeButton = new QPushButton("Reset Range");
	clearRangeButton->setAutoDefault(false);
	clearRangeButton->setDefault(false);
	QPushButton *clearMinimumButton = new QPushButton("Reset Minimum");
	clearMinimumButton->setAutoDefault(false);
	clearMinimumButton->setDefault(false);
	QPushButton *clearMaximumButton = new QPushButton("Reset Maximum");
	clearMaximumButton->setAutoDefault(false);
	clearMaximumButton->setDefault(false);

	QCheckBox *constrainCheckBox = new QCheckBox("Constrain range to data");
	constrainCheckBox->setChecked(image_->constrainToData());

	connect(minimum_, SIGNAL(editingFinished()), this, SLOT(onManualMinimumChanged()));
	connect(maximum_, SIGNAL(editingFinished()), this, SLOT(onManualMaximumChanged()));
	connect(clearRangeButton, SIGNAL(clicked()), this, SLOT(onClearClicked()));
	connect(clearMinimumButton, SIGNAL(clicked()), this, SLOT(onClearMinimumClicked()));
	connect(clearMaximumButton, SIGNAL(clicked()), this, SLOT(onClearMaximumClicked()));
	connect(constrainCheckBox, SIGNAL(toggled(bool)), this, SLOT(onConstrainRangeToData(bool)));

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
	updateDialog();
}

void MPlotImageRangeDialog::onClearMinimumClicked()
{
	image_->clearMinimum();
	updateDialog();
}

void MPlotImageRangeDialog::onClearMaximumClicked()
{
	image_->clearMaximum();
	updateDialog();
}

void MPlotImageRangeDialog::onConstrainRangeToData(bool constrain)
{
	image_->setConstrainToData(constrain);
	updateDialog();
}

void MPlotImageRangeDialog::onManualMinimumChanged()
{
	image_->setMinimum(minimum_->text().toDouble());
	updateDialog();
}

void MPlotImageRangeDialog::onManualMaximumChanged()
{
	image_->setMaximum(maximum_->text().toDouble());
	updateDialog();
}

void MPlotImageRangeDialog::updateDialog()
{
	MPlotRange range = image_->range();
	minimum_->setText(QString::number(range.x(), 'g', 3));
	maximum_->setText(QString::number(range.y(), 'g', 3));
}
