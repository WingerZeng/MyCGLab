#pragma once
#include <QString>
#include <QDir>
#include <QFileDialog>

namespace mcl {
	/**
 * @brief ��ʽ���ļ�·������'\'�滻Ϊ'/'
 */
	inline QString checkPathFormat(QString path) {
		//change '\' to '/'
		path.replace('\\', '/');
		return path;
	}
	/**
	 * @brief ��Ᵽ���ļ�·���ĺϷ���
	 * @param path �����·��
	 * @return ��Ϸ��򷵻�0�����򷵻�-1
	 */
	inline int checkSaveFilePath(QString path) {
		if (path == "") return -1;
		path = checkPathFormat(path);
		QString dir = path.left(path.lastIndexOf('/'));
		if (!QDir(dir).exists()) return -1;
		return 0;
	}
	/**
	 * @brief �����ļ�·���ĺϷ���
	 * @param path �����·��
	 * @return ��Ϸ��򷵻�0�����򷵻�-1
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
	 * @brief ��ȡ�ļ�·��ģʽ
	 */
	enum FileMode
	{
		SAVEFILE = 0, /**< ����ģʽ */
		OPENFILE    /**< ��ģʽ */
	};

	/**
	 * @brief ͨ���Ի����ȡ�ļ�·���������Ϸ��Լ��
	 * @param mode          ��ȡģʽ�����ļ�ģʽ�򱣴��ļ�ģʽ
	 * @param caption       �Ի������
	 * @param filterName    Ŀ���ļ��������ƣ��硰�ı��ļ����򡰹����ļ���
	 * @param suffixes      Ŀ���ļ����ͺ�׺�����飬��ָ�������׺��
	 * @return �ɹ����򷵻���Ӧ·�������򷵻ؿ��ַ���
	 */
	QString getFileByDialog(FileMode mode, const QString &caption = QString(), const QString &filterName = QString(), std::vector<QString> suffixes = std::vector<QString>());

}