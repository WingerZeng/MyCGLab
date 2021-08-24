#include "MaterialSettingWidget.h"
#include "ui_MaterialSettingWidget.h"
#include "ParameterInputWidget.h"
#include "Material.h"
#include "database.h"
namespace mcl{
	
	MaterialSettingWidget::MaterialSettingWidget(QWidget* par)
		:QDialog(par),ui(new Ui::MaterialSettingWidget)
	{
		ui->setupUi(this);

		QStringList itemlist;
		for (int i = 0; i < Material::MeterialTypeSize; i++) {
			itemlist.push_back(Material::matTypeToName[i]);
		}
		ui->comboBox->addItems(itemlist);

		connect(ui->comboBox, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(on_comboboxItemChanged(const QString&)));
	}

	MaterialSettingWidget::~MaterialSettingWidget()
	{
		delete ui;
	}

	void MaterialSettingWidget::init(std::shared_ptr<DataNode> data/*=nullptr*/)
	{
		initType(data->fd("type","")->getV());
		datanode = data;
		for (auto& widget : widgets) {
			auto node = data->fd("Pars","")->fd(widget.first);
			if (node)
				widget.second->setParameter(node->getV());
		}
		auto bumpnode = data->fd("map_Bump");
		if (bumpnode) {
			widgets["map_Bump"]->setParameter(bumpnode->getV());
		}
	}

	void MaterialSettingWidget::accept()
	{
		datanode->setChild("type", ui->comboBox->currentText());
		DataNode* parnode = datanode->fd("Pars", "").get();
		parnode->clear();
		for (const auto& widget : widgets) {
			if(widget.second->getParameter() != "")
				parnode->fd(widget.second->getName(), widget.second->getParameter());
		}
		auto bumpmapInput = widgets["map_Bump"]->getParameter();
		datanode->setChild("map_Bump", bumpmapInput);
		QDialog::accept();
	}

	void MaterialSettingWidget::reject()
	{
		//don't do anything
		QDialog::reject();
	}

	void MaterialSettingWidget::on_comboboxItemChanged(const QString& curStr)
	{
		if(!initing)
			initType(curStr);
	}

	void MaterialSettingWidget::initType(QString matType)
	{
		initing = true;
		widgets.clear();
		while (ui->formLayout->rowCount()) {
			ui->formLayout->removeRow(0);
		}
		int typeindex;
		if (Material::matNameToType.find(matType) != Material::matNameToType.end()) {
			typeindex = Material::matNameToType.find(matType)->second - Material::MeterialTypeBegin;
		}
		else {
			typeindex = 0;
		}
		ui->comboBox->setCurrentIndex(typeindex);
		auto parvec = Material::matparvec[typeindex];
		for (const auto& par : parvec) {
			QString name = par.first;
			ParameterInputWidget* parwidget;
			if (par.second & Material::P_Float) {
				parwidget = new FloatInputWidget(name);
				ui->formLayout->addRow(name, parwidget);
				widgets[name] = parwidget;
			}
			if (par.second & Material::P_Color) {
				parwidget = new ColorInputWidget(name);
				ui->formLayout->addRow(name, parwidget);
				widgets[name] = parwidget;
			}

			ParameterInputWidget* mapparwidget;
			QString mapname = "map_" + name;
			if (par.second & Material::P_Map) {
				mapparwidget = new PathInputWidget(mapname);
				ui->formLayout->addRow(mapname, mapparwidget);
				widgets[mapname] = mapparwidget;
			}
		}

		//����map_Bump������
		ParameterInputWidget* bumpmapwidget = new PathInputWidget("map_Bump");
		ui->formLayout->addRow("map_Bump", bumpmapwidget);
		widgets["map_Bump"] = bumpmapwidget;


		initing = false;
	}

}