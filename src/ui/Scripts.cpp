#include "Scripts.h"
#include <fstream>
#include <QFile>
#include <QDirIterator>
#include "ProjectMagager.h"
#include "ui_ScriptList.h"
#include "MainWindow.h"
#include "Console.h"
namespace mcl{
	void ScriptList::updateList()
	{
		updating = true;
		currenScp_ = nullptr;
		ui->textBrowser->clear();
		ui->textBrowser_2->clear();
		ui->listWidget->clear();
		for (const auto& scp: scps_) {
			ui->listWidget->addItem(scp.title);
		}
		updating = false;
	}

	void ScriptList::on_listSelectionChanged(const QString& title)
	{
		if (updating) return;
		Script temp;
		temp.title = title;
		auto it = scps_.find(temp);
		if (it == scps_.end()) return;
		ui->textBrowser->setText(it->content);
		ui->textBrowser_2->setText(it->description);
		currenScp_ = &(*it);
	}

	void ScriptList::scriptToFile(Script scp, QString filePath)
	{
		QFile file(filePath);
		file.open(QIODevice::WriteOnly | QIODevice::Text);
		QString output;
		output = "#title\n" + scp.title + "\n" +
			"#content\n" + scp.content + "\n" +
			"#description\n" + scp.description + "\n";
		file.write(output.toUtf8());
		file.close();

		return;
	}

	mcl::Script ScriptList::fileToScript(QString filePath, bool* isok)
	{
		QFile file(filePath);
		file.open(QIODevice::ReadOnly | QIODevice::Text);
		QByteArray t = file.readAll();
		QString qstr(t);
		file.close();

 		QString tag1 = "#title\n";
		QString tag2 = "#content\n";
		QString tag3 = "#description\n";

		int idx1 = qstr.indexOf(tag1);
		int idx2 = qstr.indexOf(tag2);
		int idx3 = qstr.indexOf(tag3);

		if (idx1 == -1 || idx2 == -1 || idx3 == -1) {
			if (isok) *isok = false;
			return Script();
		}

		Script scp;
		scp.title = qstr.mid(idx1 + tag1.size(), idx2 - idx1 - tag1.size() -1);
		scp.content = qstr.mid(idx2 + tag2.size(), idx3 - idx2 - tag2.size());
		scp.description = qstr.mid(idx3 + tag3.size());

		if (isok) *isok = true; 
		return scp;
	}

	void ScriptList::execute()
	{
		if (!currenScp_) return;
		EXEPTR->exec(currenScp_->content, MAIPTR->getConsole());
	}

	void ScriptList::execute(QString title)
	{
		Script temp;
		temp.title = title;
		auto it = scps_.find(temp);
		if (it == scps_.end())  return;
		EXEPTR->exec((*it).content, MAIPTR->getConsole());
	}

	ScriptList::ScriptList(QWidget* parent /*= nullptr*/)
		:QWidget(parent), ui(new Ui::ScriptList), updating(false)
	{
		currenScp_ = nullptr;
		ui->setupUi(this);
		connect(ui->listWidget, &QListWidget::currentTextChanged, this, &ScriptList::on_listSelectionChanged);
		connect(ui->pushButton, SIGNAL(released()), this, SLOT(execute()));
	}

	void ScriptList::init()
	{
		QDir dir(PROPTR->swPath() + "/scripts");
		QStringList files = dir.entryList(QStringList() << "*.scp", QDir::NoSymLinks | QDir::Files | QDir::NoDotAndDotDot);
		for (const auto& path : files) {
			addScript(fileToScript(PROPTR->swPath() + "/scripts/" + path));
		}

		//QDirIterator qdi(,
		//	QStringList() << "*.scp",
		//	QDir::NoSymLinks | QDir::Files | QDir::NoDotAndDotDot,
		//	QDirIterator::Subdirectories);
		//
		//while (qdi.hasNext()) {
		//	addScript(fileToScript(qdi.next()));
		//}
	}

	void ScriptList::saveAll()
	{
		QDir dir(PROPTR->swPath() + "/scripts");
		QStringList files = dir.entryList(QStringList() << "*.scp", QDir::NoSymLinks | QDir::Files | QDir::NoDotAndDotDot);
		for (const auto& path : files) {
			QFile::remove(PROPTR->swPath() + "/scripts/" + path);
		}

		for (const auto& scp : scps_) {
			QString title = scp.title;
			scriptToFile(scp, PROPTR->swPath() + "/scripts/" + title + ".scp");
		}
	}

	ScriptList::~ScriptList()
	{
	}

	void ScriptList::addScript(const Script& scp)
	{
		scps_.insert(scp);
		updateList();
	}

}