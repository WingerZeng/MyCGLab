#include <QtWidgets/QApplication>
#include <algorithm>
#include <math.h>
#include "Scene.h"
#include "primitives/PPolygon.h"
#include "core/CadCore.h"
#include "ui/MainWindow.h"
#include "ui/Scripts.h"
#include "ui/ProjectMagager.h"
#include <QTextCodec>
#include <QFileInfo>
#include <QDir>

#include "glog/logging.h"
using namespace mcl;

int main(int argc, char *argv[])
{
	google::InitGoogleLogging(argv[0]);
	QApplication a(argc, argv);
	QFileInfo exefile(QString::fromLocal8Bit(argv[0]));
	PROPTR->setSoftwarePath(exefile.absoluteDir().path());
	MAIPTR->show();

	return a.exec();
}