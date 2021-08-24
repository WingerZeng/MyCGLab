#pragma once
#include "mcl.h"
#include "types.h"
#include "database.h"
#include <QDialog>
namespace Ui {
	class MaterialSettingWidget;
}

class DataNode;
namespace mcl {
	class ParameterInputWidget;

	class MaterialSettingWidget: public QDialog
	{
		Q_OBJECT
	public:
		MaterialSettingWidget(QWidget* par = nullptr);
		~MaterialSettingWidget();
		void init(std::shared_ptr<DataNode> data=nullptr);

	public Q_SLOTS:
		virtual void accept();
		virtual void reject();
		void on_comboboxItemChanged(const QString& curStr);

	private:
		void initType(QString matType);

	private:
		std::map<QString, ParameterInputWidget*> widgets;
		std::shared_ptr<DataNode> datanode;
		Ui::MaterialSettingWidget* ui;
		bool initing = false;
	};
}

