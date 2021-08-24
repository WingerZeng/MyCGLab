#pragma once
#include <QString>
#include <QDir>
#include <QFileDialog>

namespace mcl {
	/**
 * @brief 格式化文件路径，将'\'替换为'/'
 */
	inline QString checkPathFormat(QString path) {
		//change '\' to '/'
		path.replace('\\', '/');
		return path;
	}
	/**
	 * @brief 检测保存文件路径的合法性
	 * @param path 待检测路径
	 * @return 如合法则返回0，否则返回-1
	 */
	inline int checkSaveFilePath(QString path) {
		if (path == "") return -1;
		path = checkPathFormat(path);
		QString dir = path.left(path.lastIndexOf('/'));
		if (!QDir(dir).exists()) return -1;
		return 0;
	}
	/**
	 * @brief 检测打开文件路径的合法性
	 * @param path 待检测路径
	 * @return 如合法则返回0，否则返回-1
	 */
	inline int checkOpenFilePath(QString path) {
		if (path == "") return -1;
		if (!QFile(path).exists()) return -1;
		return 0;
	}
	inline QString rmSuffix(QString rawString, const QString& suffix) {
		if (suffix == "") return rawString;
		rawString.replace(suffix, "");
		rawString.replace(" ", "");
		return rawString;
	}
	/**
	 * @brief 获取文件路径模式
	 */
	enum FileMode
	{
		SAVEFILE = 0, /**< 保存模式 */
		OPENFILE    /**< 打开模式 */
	};

	/**
	 * @brief 通过对话框获取文件路径，包含合法性检查
	 * @param mode          获取模式，打开文件模式或保存文件模式
	 * @param caption       对话框标题
	 * @param filterName    目标文件类型名称，如“文本文件”或“工程文件”
	 * @param suffixes      目标文件类型后缀名数组，可指定多个后缀名
	 * @return 成功，则返回相应路径，否则返回空字符串
	 */
	QString getFileByDialog(FileMode mode, const QString &caption = QString(), const QString &filterName = QString(), std::vector<QString> suffixes = std::vector<QString>());

}