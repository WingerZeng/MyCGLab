#pragma once
#include "mcl.h"
#include "types.h"
#include "Singleton.h"
#include <vector>
#include <set>
#include <QtWidgets/QWidget>

namespace Ui {
	class ScriptList;
}

namespace mcl {
	struct Script
	{
		bool operator<(const Script& rhs) const { return this->title < rhs.title; }
		QString title;
		QString content;
		QString description;
	};

	class ScriptList:public QWidget
	{
		Q_OBJECT
	public:
		~ScriptList();
		ScriptList(QWidget* parent = nullptr);
		/**@brief initialize and load all script from app dir.
		 */
		void init();
		void execute(QString title);
		/**
		 * @brief Save all script to application file
		 */
		void saveAll();
		void addScript(const Script& scp);
	private slots:
		void updateList();
		void on_listSelectionChanged(const QString& title);
		void scriptToFile(Script scp, QString filePath);
		void execute();
		Script fileToScript(QString filePath,bool *isok=nullptr);

	private:
		Ui::ScriptList* ui;

		std::set<Script> scps_;
		const Script* currenScp_;
		bool updating;
	};
}

