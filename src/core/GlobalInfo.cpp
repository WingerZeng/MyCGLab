#include "GlobalInfo.h"
#include <QThread>
#include <QCoreApplication>
namespace mcl{
	
	QString GlobalInfo::getDataString(QString str, bool* succ)
	{
		auto datanode = data->fd(str);
		if (datanode) {
			if (succ) *succ = true;
			return datanode->getV();
		}
		if (succ) *succ = false;
		return "";
	}

	QStringList GlobalInfo::getKeyList()
	{
		QStringList list;
		for (auto it = data->begin(); it != data->end(); it++) {
			list.push_back((*it)->getK());
		}
		return list;
	}

	GlobalInfo::GlobalInfo()
	{
		QCoreApplication::applicationDirPath;
		data = std::make_unique<DataNode>();
		//命令行参数转义
		data->setChild("MachCoreNum",QString::number(QThread::idealThreadCount()));
		data->setChild("ExeDir", QCoreApplication::applicationDirPath());
	}

	int setGlobalData(std::string key, std::string value)
	{
		GLBPTR->data->setChild(QString::fromStdString(key), QString::fromStdString(value));
		return 0;
	}

	std::shared_ptr<DataNode> getGlobalData(QString key, QString defValue)
	{
		return GLBPTR->data->fd(key, defValue);
	}

}