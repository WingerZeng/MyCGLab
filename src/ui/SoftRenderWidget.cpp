#include "SoftRenderWidget.h"
#include "utilities.h"
#include "ui_softrenderwidget.h"
#include "softrender/SoftRenderer.h"
#include "softrender/StratiZBufferRenderer.h"
#include "softrender/StratiZBufferOctreeRenderer.h"
#include "MainWindow.h"
#include "ItemManager.h"
namespace mcl{
	
	SoftRenderWidget::SoftRenderWidget(Scene* scene, QWidget* parent /*= nullptr*/)
		:QWidget(parent), ui(new Ui::SoftRenderWidget)
	{
		ui->setupUi(this);
		connect(ui->pushButton, &QPushButton::released, this, &SoftRenderWidget::refresh);
		//扫描线zbuffer
		renderers[0].reset(new SoftRenderer(scene, nPixelX, nPixelY));
		imagelabels[0] = ui->label_sweep;
		connect(imagelabels[0], &clickableLabel::doubleClicked, [this]() {
			this->showFullResImage(0);
			});

		//层次zbuffer
		renderers[1].reset(new StratiZBufferRenderer(scene, nPixelX, nPixelY));
		imagelabels[1] = ui->label_strati;
		connect(imagelabels[1], &clickableLabel::doubleClicked, [this]() {
			this->showFullResImage(1);
			});

		//层次zbuffer + Octree
		renderers[2].reset(new StratiZBufferOctreeRenderer(scene, nPixelX, nPixelY));
		imagelabels[2] = ui->label_stratioc;
		connect(imagelabels[2], &clickableLabel::doubleClicked, [this]() {
			this->showFullResImage(2);
			});
	}

	SoftRenderWidget::~SoftRenderWidget()
	{
		delete ui;
	}
	 
	void SoftRenderWidget::showFullResImage(int i)
	{
		QLabel* label = new QLabel(nullptr, Qt::Window);
		label->setAttribute(Qt::WA_DeleteOnClose);
		label->resize(renderers[i]->getFilm()->getSize().x(), renderers[i]->getFilm()->getSize().y());
		renderers[i]->getFilm()->renderToQLabel(label);
		label->show();
	}

	void SoftRenderWidget::refresh()
	{
		for (int i = 0; i < nRenderer; i++) {
			if (renderers[i]) {
				renderers[i]->paint();
				renderers[i]->getFilm()->renderToQLabel(imagelabels[i]);
			}

			repaint();
		}

		//计算与输出耗时
		QString tag[4];
		tag[0] = QString::fromLocal8Bit("数据结构构造耗时");
		tag[1] = QString::fromLocal8Bit("消隐与光栅化耗时");
		tag[2] = QString::fromLocal8Bit("着色耗时");
		tag[3] = QString::fromLocal8Bit("总耗时");

		double cost[3][4];
		cost[0][1] = Clock<ZBUFFER_SWEEPLINE_CULL>::millsecond();
		cost[0][2] = Clock<ZBUFFER_SWEEPLINE_SHADER>::millsecond();
		cost[0][3] = Clock<ZBUFFER_SWEEPLINE>::millsecond();

		cost[1][1] = Clock<ZBUFFER_STRATIFY_CULL>::millsecond();
		cost[1][2] = Clock<ZBUFFER_STRATIFY_SHADER>::millsecond();
		cost[1][3] = Clock<ZBUFFER_STRATIFY>::millsecond();

		cost[2][1] = Clock<ZBUFFER_STRATIFY_OCTREE_CULL>::millsecond();
		cost[2][2] = Clock<ZBUFFER_STRATIFY_OCTREE_SHADER>::millsecond();
		cost[2][3] = Clock<ZBUFFER_STRATIFY_OCTREE>::millsecond();

		for (int i = 0; i < 3; i++) {
			cost[i][0] = cost[i][3] - cost[i][2] - cost[i][1];
		}

		QString tempstr(""
			"%1 = %5 ms\n"
			"%2 = %6 ms\n"
			"%3 = %7 ms\n"
			"%4 = %8 ms\n");
		tempstr = tempstr.arg(tag[0], 10).arg(tag[1], 10).arg(tag[2], 10).arg(tag[3], 10);

		ui->label_sweep_t->setText(tempstr.arg(cost[0][0], 6).arg(cost[0][1], 6).arg(cost[0][2], 6).arg(cost[0][3], 6));
		ui->label_starti_t->setText(tempstr.arg(cost[1][0], 6).arg(cost[1][1], 6).arg(cost[1][2], 6).arg(cost[1][3], 6));
		ui->label_stratioc_t->setText(tempstr.arg(cost[2][0], 6).arg(cost[2][1], 6).arg(cost[2][2], 6).arg(cost[2][3], 6));
	
		//显示额外信息
		QString infostr = QString::fromLocal8Bit("多边形数: %1		双击图片可查看大图");
		infostr = infostr.arg(MAIPTR->getItemMng()->getPolygonCount());
		ui->label->setText(infostr);

	}

	void clickableLabel::mouseDoubleClickEvent(QMouseEvent *event)
	{
		emit doubleClicked();
		QLabel::mouseDoubleClickEvent(event);
	}
}