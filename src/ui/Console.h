#pragma once
#include "mcl.h"
#include "types.h"
#include <string>
#include <QWidget>
#include <functional>
#include <tuple>
#include "qconsole.h"
#include "Singleton.h"
#include "htrie/htrie_set.h"
#include "htrie/htrie_map.h"


#define EXEPTR (Singleton<Executor>::getSingleton())

namespace mcl {
	class Console:public QConsole
	{
		Q_OBJECT
	public:
		Console(QWidget* parent=nullptr);
		~Console();
		QString addCommandToHistory(const QString &command) override;

	protected:
		QStringList suggestCommand(const QString &cmd, QString &prefix) override;
		bool isCommandComplete(const QString &command) override;

	private:
		static const QString welcomeText;
	};


	class Executor :public Singleton<Executor>
	{
	public:
		friend Singleton<Executor>;
		void exec(const QString& exec, Console* con=nullptr);
		static int getHelp();
		int printMessage(const QString& msg, Console::ResultType type);
		QStringList suggestCommand(const QString &cmd, QString &prefix);
	private:
		Executor();

		QString modifyCmd(const QString& exec);
		Console* con_ = nullptr;
		tsl::htrie_map<char, std::function<int(std::iostream& sin)>> triemap;
		std::map<std::string, std::vector<std::string>> funcArgNameList;  //函数形参类型名称列表
	};
}