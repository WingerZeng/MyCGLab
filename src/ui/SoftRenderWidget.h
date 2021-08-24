#pragma once
#include "mcl.h"
#include "types.h"
#include <QWidget>
#include <QMouseEvent>
#include <QLabel>
#include <QCloseEvent>
namespace Ui {
	class SoftRenderWidget;
}

namespace mcl {
	class SoftRenderer;
	class Film;
	class clickableLabel;
	class SoftRenderWidget:public QWidget
	{
		Q_OBJECT
	public:
		SoftRenderWidget(Scene* scene, QWidget* parent = nullptr);
		~SoftRenderWidget();
	public slots:
		void showFullResImage(int i);
		void refresh();
	private:
		static constexpr int nPixelY = 1024;
		static constexpr int nPixelX = 1024;
		static constexpr int nRenderer = 3;
		Ui::SoftRenderWidget* ui;
		std::unique_ptr<SoftRenderer> renderers[nRenderer];
		clickableLabel* imagelabels[nRenderer];
	};

	class clickableLabel : public QLabel
	{
		Q_OBJECT
	public:
		clickableLabel(QWidget* parent = nullptr)
		:QLabel(parent){
		}
	signals:
		void doubleClicked();
	protected slots:
		virtual void mouseDoubleClickEvent(QMouseEvent *event) override;
	};
}

