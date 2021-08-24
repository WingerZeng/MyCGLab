#pragma once
#include "mcl.h"
#include "types.h"
#include <QWidget>
#include <QDoubleSpinBox>
namespace Ui {
	class ColorInputWidget;
	class PathInputWidget;
}

namespace mcl {
	class ParameterInputWidget: public QWidget
	{
		Q_OBJECT
	public:
		ParameterInputWidget(QString name,QWidget* parent = nullptr);
		virtual ~ParameterInputWidget() {};
		virtual QString getParameter() = 0;
		virtual void setParameter(QString str) = 0;
		QString getName() { return name; }

	private:
		QString name;
	};

	class ColorInputWidget : public ParameterInputWidget
	{
		Q_OBJECT
	public:
		ColorInputWidget(QString name, QWidget* parent = nullptr);
		~ColorInputWidget();

		QString getParameter() override;
		void setParameter(QString str) override;
	private:
		Ui::ColorInputWidget* ui;
	};

	class FloatInputWidget : public ParameterInputWidget
	{
		Q_OBJECT
	public:
		FloatInputWidget(QString name, QWidget* parent = nullptr);

		QString getParameter() override;
		void setParameter(QString str) override;
	private:
		QDoubleSpinBox* spinbox;
	};

	class PathInputWidget : public ParameterInputWidget 
	{
		Q_OBJECT
	public:
		PathInputWidget(QString name, QWidget* parent = nullptr);
		~PathInputWidget();
		QString getParameter() override;
		void setParameter(QString str) override;

	private slots:
		void on_pushButtionClicked();

	private:
		Ui::PathInputWidget* ui;
	};
}

