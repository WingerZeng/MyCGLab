#pragma once
#include "mcl.h"
#include "types.h"
#include <QMainWindow>
#include <QTreeWidget>
#include "Singleton.h"
#include "CadInterface.h"
#define MAIPTR (Singleton<MainWindow>::getSingleton())

namespace Ui {
	class MainWindow;
}

class QTreeWidget;
class QLabel;
class DataNode;
namespace mcl {
	class SoftRenderWidget;
	class CadInterface;
	class ScriptList;
	class ItemManager;
	class Console;
	class MainWindow: public QMainWindow, private Singleton<MainWindow>
	{
		Q_OBJECT
		friend Singleton<MainWindow>;
	public:
		~MainWindow();
		mcl::CadInterface* CadApi() const { return cadapi; }
		Scene* getScene() const;
		QTreeWidget* getCadTreeWidget() const;
		Console* getConsole();
		void cadTreeUnselectedAll();

		mcl::ScriptList* scplist() const { return ui_scplist; }
		std::shared_ptr<mcl::ItemManager> getItemMng() const { return itemMng_; }
		std::shared_ptr<DataNode> getData(QString key);

		/* console interfaces */
		int resize(int w, int h);
		int softRenderer();
		int runRayTrace();
		int setData(std::string key, std::string value);
		int saveRtFilm(PathString path);

	private:
		MainWindow(QWidget* parent = nullptr);
		void setupScripts();
		void setCurrenRaytraceResult(QLabel* label, std::shared_ptr<Film> film);

	public slots:
		//void updateCadElements(CadInterface::ELEMEMNT_TYPE elemtype, CadInterface::ElemHandle elem);
		void cleanAllSelection();

	private:
		CadInterface* cadapi;

		std::map<CadInterface::ElemHandle, QTreeWidgetItem*> cadElemMap;

		Ui::MainWindow* ui;

		ScriptList* ui_scplist;

		std::shared_ptr<ItemManager> itemMng_;

		std::unique_ptr<SoftRenderWidget> softrdWgt_;

		std::shared_ptr<DataNode> globalData;

		QLabel* curRtLabel = nullptr;
		QMetaObject::Connection labelcon;
		std::shared_ptr<Film> curFilm;
	};
}

