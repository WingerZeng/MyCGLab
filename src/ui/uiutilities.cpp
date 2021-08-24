#include "uiutilities.h"

namespace mcl {

	QString getFileByDialog(FileMode mode, const QString &caption /*= QString()*/, const QString &filterName /*= QString()*/, std::vector<QString> suffixes /*= std::vector<QString>()*/)
	{
		QString filter;
		if (!suffixes.empty()) {
			filter = filterName + " (";
			bool first = 1;
			for (const auto& suffix : suffixes) {
				if (first) first = 0;
				else filter += ' ';
				filter += "*." + suffix;
			}
			filter += ')';
		}
		QString path;
		QString start_path; //TODO: 设置一个合适的路径作为初始路径
		if (mode == SAVEFILE)
			path = QFileDialog::getSaveFileName(nullptr, caption, start_path, filter);
		else if (mode == OPENFILE)
			path = QFileDialog::getOpenFileName(nullptr, caption, start_path, filter);
		//change '\' to '/'
		path = checkPathFormat(path);
		int check = -1;
		if (mode == SAVEFILE) check = checkSaveFilePath(path);
		else if (mode == OPENFILE) check = checkOpenFilePath(path);
		if (check) {
			return "";
		}
		QString suffix = QFileInfo(path).suffix();
		if (find(suffixes.begin(), suffixes.end(), suffix) == suffixes.end()) {
			return "";
		}
		return path;
	}

}