#include "MainWindow.h"
#include "ui_mainwindow.h"
#include <QLabel>
#include "ItemManager.h"
#include "Scripts.h"
#include "SoftRenderWidget.h"
#include "realtime/Scene.h"
#include "database.h"
#include "RTPrimitive.h"
#include "Film.h"
#include "Sampler.h"
#include "RayTracer.h"
#include "RTScene.h"
#include "ToneMapper.h"
#include "Denoiser.h"
#include "Bsdf.h"
#include "RTLight.h"
#include "utilities.h"
namespace mcl{
	
	MainWindow::MainWindow(QWidget* parent)
		:QMainWindow(parent),ui(new Ui::MainWindow)
	{
		ui->setupUi(this);
		cadapi = new CadInterface();
		connect(cadapi, SIGNAL(elementChanged(ELEMEMNT_TYPE, ElemHandle)), this, SLOT(cleanAllSelection()));
		connect(getCadTreeWidget(), SIGNAL(itemSelectionChanged()), cadapi, SLOT(itemSelectionChanged()));

		ui_scplist = new ScriptList;
		connect(ui->actionScript_List, &QAction::triggered, [this]() { this->scplist()->show(); });

		setupScripts();

		itemMng_.reset(new ItemManager(ui->treeWidget_general));

		globalData = std::make_shared<DataNode>();
	}

	void MainWindow::setupScripts()
	{
		scplist()->init();
	}

	void MainWindow::setCurrenRaytraceResult(QLabel* label, std::shared_ptr<Film> film)
	{
		if (curRtLabel) {
			disconnect(labelcon);
		}
		curRtLabel = label;
		curFilm = film;
		if (label) {
			labelcon = connect(label, &QLabel::destroyed, [this]() {
				this->setCurrenRaytraceResult(nullptr, nullptr);
				});
		}
		return;
	}

	void MainWindow::cleanAllSelection()
	{
		getCadTreeWidget()->clearSelection();
	}

	MainWindow::~MainWindow()
	{
		//ui_scplist->saveAll();
		delete ui;
	}

	int MainWindow::resize(int w, int h)
	{
		ui->openGLWidget->resize(w, h);
		return 0;
	}

	mcl::Scene* MainWindow::getScene() const
	{
		return ui->openGLWidget;
	}

	QTreeWidget* MainWindow::getCadTreeWidget() const
	{
		return ui->treeWidget;
	}

	mcl::Console* MainWindow::getConsole()
	{
		return ui->textEdit;
	}

	void MainWindow::cadTreeUnselectedAll()
	{
		auto list = getCadTreeWidget()->selectedItems();
		for (const auto& item:list)
		{
			item->setSelected(false);
		}
	}

	int MainWindow::softRenderer()
	{
		if (!softrdWgt_) {
			softrdWgt_.reset(new SoftRenderWidget(getScene()));
		}
		softrdWgt_->show();
		softrdWgt_->repaint();
		softrdWgt_->refresh();
		return 0;
	}

	int MainWindow::runRayTrace()
	{
		ui->openGLWidget->repaint();
		/* create instances */
		std::shared_ptr<Film> film(new Film(getScene()->width(), getScene()->height()));
		std::shared_ptr<Film> denoisedFilm(new Film(getScene()->width(), getScene()->height()));
		std::shared_ptr<Sampler> sampler(new Sampler);
		auto rtscene = getScene()->createRTScene();
		std::shared_ptr<RayTracer> raytracer(new DirectLightPathTracer(film, sampler));

		/* init parameters */
		auto ns = getData("samplePerPixelDim");
		raytracer->setSampleNum(ns ? ns->toInt() : 8);
		auto ts = getData("threadNum");
		raytracer->setThreadNum(ts ? ts->toInt() : 6);

		//raytracer->renderPixel(*rtscene, Point2i(130,90));
		/* render */
		Clock<RAYTRACER>::clear();
		Clock<RAYTRACER>::start();
		raytracer->render(*rtscene);
		Clock<RAYTRACER>::stop();
		EXEPTR->printMessage("*** time cost in ray trace: " + QString::number(Clock<RAYTRACER>::millsecond()) + "ms ***", QConsole::Partial);

		/* display */
		QLabel* displayer[2];
		for (int i = 0; i < 2; i++) {
			displayer[i] = new QLabel;
			displayer[i]->setAttribute(Qt::WA_DeleteOnClose);
			displayer[i]->resize(film->getSize().x(), film->getSize().y());
			displayer[i]->show();
		}

		denoisedFilm = std::make_shared<Film>(*film);
		GammaCorrectMapper().map(*film);
		film->renderToQLabel(displayer[0]);
		film->saveToBmp("./ori.bmp");
		Denoiser(denoisedFilm, Denoiser::DenoiserOption(Denoiser::USE_ALBEDO | Denoiser::USE_NORMAL | Denoiser::USE_HDR)).denoise();
		GammaCorrectMapper().map(*denoisedFilm);
		denoisedFilm->renderToQLabel(displayer[1]);
		setCurrenRaytraceResult(displayer[1], denoisedFilm);

		/* for test */
	 //	std::shared_ptr<Film> film2(new Film(getScene()->width(), getScene()->height()));
		//std::shared_ptr<RayTracer> raytracer2(new MaterialTestTracer(film2, sampler));
		//raytracer2->setSampleNum(ns ? ns->toInt() : 8);
		//raytracer2->setThreadNum(ts ? ts->toInt() : 6);
	 //	raytracer2->render(*rtscene);
	 //	QLabel* displayer1 = new QLabel;
	 //	displayer1->resize(film2->getSize().x(), film2->getSize().y());
	 //	displayer1->show();
	 //	film2->renderToQLabel(displayer1);
		return 0;
	}

	int MainWindow::setData(std::string key, std::string value)
	{
		globalData->setChild(QString::fromStdString(key), QString::fromStdString(value));
		return 0;
	}

	int MainWindow::saveRtFilm(PathString path)
	{
		QString qstr = QString::fromStdString(path.str);
		curFilm->saveToBmp(qstr);
		return 0;
	}

	std::shared_ptr<DataNode> MainWindow::getData(QString key)
	{
		return globalData->fd(key);
	}

}