#include "ItemManager.h"
#include "Primitive.h"
#include "MainWindow.h"
#include "realtime/Scene.h"
#include "primitives/PPolygonMesh.h"
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include "MaterialSettingWidget.h"
#include "Material.h"
#include "FileIO.h"
namespace mcl{

	ItemManager::ItemManager(QTreeWidget* treeWgt) :treeWgt_(treeWgt)
	{
		treeWgt_->connect(treeWgt_, &QTreeWidget::itemSelectionChanged, [this]() {
			auto list = this->treeWgt_->selectedItems();

			this->clearSelected();
			for (const auto& item : list) {
				items_[item->text(1).toInt()].prim->setSelected(true);
				//#PERF5 每次都要重新清空选择项
				selectedPrims.push_back(items_[item->text(1).toInt()].prim);
			}
			MAIPTR->getScene()->update();
			});
	}

	void ItemManager::addItem(std::shared_ptr<Primitive> prim)
	{
		if (isExist(prim))
			return;

		QTreeWidgetItem* newItem = new QTreeWidgetItem({ prim->name() ,QString::number(prim->id())});
		treeWgt_->addTopLevelItem(newItem);
		items_[prim->id()] = Item(prim, newItem);

		MAIPTR->getScene()->addPrimitive(prim);
	}

	void ItemManager::addItem(std::vector<std::shared_ptr<Primitive>> prims)
	{
		for (const auto& prim : prims) {
			addItem(prim);
		}
	}

	std::shared_ptr<Primitive> ItemManager::getItem(int id)
	{
		if (items_.find(id) != items_.end()) return nullptr;
		return items_[id].prim;
	}

	void ItemManager::delItem(std::shared_ptr<Primitive> prim)
	{
		if (!isExist(prim))
			return;

		treeWgt_->takeTopLevelItem(treeWgt_->indexOfTopLevelItem(items_[prim->id()].treeItem));
		MAIPTR->getScene()->delPrimitive(items_[prim->id()].prim->id());

		items_.erase(prim->id());

		clearSelected();

		//#TODO2 删除子节点
	}

	int ItemManager::clearSelected()
	{
		for (const auto& prim : selectedPrims) {
			prim->setSelected(false);
		}
		//MAIPTR->getScene()->delPrimitives(selectedPrims);
		selectedPrims.clear();
		return 0;
	}

	int ItemManager::clonePrimitive(int id)
	{
		if (!isExist(id))
			return -1;
		std::shared_ptr<Primitive> newprim = std::move(items_[id].prim->clone());
		newprim->copyAttribute(items_[id].prim);
		addItem(newprim);
		MAIPTR->getScene()->updateScene(Scene::PRIMITIVE);
		return 0;
	}

	int ItemManager::setMaterial(int id, ReadRemainString remain)
	{
		if (!isExist(id))
			return -1;
		if (remain.empty()) { //如果没有输入多余参数，则弹出界面
			auto prim = items_[id].prim;
			MaterialSettingWidget* widget = new MaterialSettingWidget;
			widget->setAttribute(Qt::WA_DeleteOnClose);
			auto node = prim->getMaterialNode();
			if (!node) return -1;
			widget->init(prim);
			widget->show();
			return 0;
		}
		else {
			auto prim = items_[id].prim;
			int ret = Material::inputToNode(remain, prim->getMaterialNode().get());
			if (!ret)
				prim->initMaterial();
			return ret;
		}
	}

	int ItemManager::scale(int id, Vector3f factor)
	{
		if (!isExist(id))
			return -1;
		items_[id].prim->appendLocalTransform(Transform::scale(factor));
		MAIPTR->getScene()->updateScene(Scene::PRIMITIVE);
		return 0;
	}

	int ItemManager::rotate(int id, Vector3f axis, double angle)
	{
		if (!isExist(id))
			return -1;
		items_[id].prim->appendLocalTransform(Transform::rotate(axis, angle));
		MAIPTR->getScene()->updateScene(Scene::PRIMITIVE);
		return 0;
	}

	int ItemManager::translate(int id, Vector3f vec)
	{
		if (!isExist(id))
			return -1;
		items_[id].prim->appendLocalTransform(Transform::translate(vec));
		MAIPTR->getScene()->updateScene(Scene::PRIMITIVE);
		return 0;
	}

	int ItemManager::loadObj(PathString path)
	{
		if (!path.exist()) return -1;
		std::vector<std::shared_ptr<Primitive>> prims;
		readObj(QString::fromStdString(path.str), prims);
		addItem(prims);
		return 0;
	}

	int ItemManager::getPolygonCount()
	{
		int count = 0;

		for (const auto& item : items_) {
			if (item.second.prim->dynamic_tag() == PPolygonMesh::tag()) {
				auto pplg = std::dynamic_pointer_cast<PPolygonMesh>(item.second.prim);
				count += pplg->getPlgs().size();
			}
		}

		return count;
	}

}