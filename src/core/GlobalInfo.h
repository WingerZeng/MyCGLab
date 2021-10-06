#pragma once
#include "mcl.h"
#include "types.h"
#include <map>
#include <string>
#include "Singleton.h"
#include "database.h"

#define GLBPTR (GlobalInfo::getSingleton())

namespace mcl {
	class GlobalInfo: public Singleton<GlobalInfo>
	{
	public:
		friend int setGlobalData(std::string key, std::string value);
		friend std::shared_ptr<DataNode> getGlobalData(QString key, QString defValue);

		friend Singleton<GlobalInfo>;
		QString getDataString(QString str, bool* succ = nullptr);
		QStringList getKeyList();

		bool shaderReady = false;
	private:
		GlobalInfo();
		std::unique_ptr<DataNode> data;
	};

	int setGlobalData(std::string key, std::string value);

	std::shared_ptr<DataNode> getGlobalData(QString key, QString defValue = "");
}

