#pragma once
#include "mcl.h"
#include "types.h"
#include <map>
#include <string>
#include "Singleton.h"
#include "database.h"
namespace mcl {
	class GlobalInfo: public Singleton<GlobalInfo>
	{
	public:
		friend Singleton<GlobalInfo>;
		QString getDataString(QString str, bool* succ = nullptr);
		QStringList getKeyList();

	private:
		GlobalInfo();
		std::unique_ptr<DataNode> data;
	};
}

