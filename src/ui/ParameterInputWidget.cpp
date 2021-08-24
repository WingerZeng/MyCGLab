#include "ParameterInputWidget.h"
#include "ui_ColorInputWidget.h"
#include "ui_PathInputWidget.h"
#include "uiutilities.h"
#include <QHBoxLayout>
namespace mcl{
	
	ParameterInputWidget::ParameterInputWidget(QString name, QWidget* parent /*= nullptr*/)
		:QWidget(parent),name(name)
	{
	}

	ColorInputWidget::ColorInputWidget(QString name, QWidget* parent /*= nullptr*/)
		:ParameterInputWidget(name,parent), ui(new Ui::ColorInputWidget)
	{
		ui->setupUi(this);
	}

	ColorInputWidget::~ColorInputWidget()
	{
		delete ui;
	}

	QString ColorInputWidget::getParameter()
	{
		QString ret;
		ret += ui->doubleSpinBox->text() + " ";
		ret += ui->doubleSpinBox_2->text() + " ";
		ret += ui->doubleSpinBox_3->text();
		return ret;
	}

	void ColorInputWidget::setParameter(QString str)
	{
		std::stringstream sin(str.toStdString());
		double val1,val2,val3;
		sin >> val1 >> val2 >> val3;
		ui->doubleSpinBox->setValue(val1);
		ui->doubleSpinBox_2->setValue(val2);
		ui->doubleSpinBox_3->setValue(val3);
	}

	FloatInputWidget::FloatInputWidget(QString name, QWidget* parent /*= nullptr*/)
		:ParameterInputWidget(name, parent)
	{
		spinbox = new QDoubleSpinBox;
		spinbox->setDecimals(3);
		spinbox->setMinimum(0);
		spinbox->setMaximum(1e10);
		spinbox->setSingleStep(0.01);

		QHBoxLayout* layout = new QHBoxLayout;
		layout->addWidget(spinbox);
		this->setLayout(layout);
	}

	QString FloatInputWidget::getParameter()
	{
		return spinbox->text();
	}

	void FloatInputWidget::setParameter(QString str)
	{
		std::stringstream sin(str.toStdString());
		double val;
		sin >> val;
		spinbox->setValue(val);
	}

	PathInputWidget::PathInputWidget(QString name, QWidget* parent /*= nullptr*/)
		:ParameterInputWidget(name, parent), ui(new Ui::PathInputWidget)
	{
		ui->setupUi(this);
		connect(ui->pushButton, &QPushButton::clicked, this, &PathInputWidget::on_pushButtionClicked);
	}

	PathInputWidget::~PathInputWidget()
	{
		delete ui;
	}

	QString PathInputWidget::getParameter()
	{
		return ui->lineEdit->text();
	}

	void PathInputWidget::setParameter(QString str)
	{
		ui->lineEdit->setText(str);
	}

	void PathInputWidget::on_pushButtionClicked()
	{
		QString path = getFileByDialog(OPENFILE, "select image file", "image", std::vector<QString>{"png", "jpg", "hdr"});
		if (path=="") return;
		ui->lineEdit->setText(path);
	}

}