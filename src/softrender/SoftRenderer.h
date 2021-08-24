#pragma once
#include "mcl.h"
#include "types.h"
#include <QLabel>
#include "QObject"
#include "Film.h"
#include "Scene.h"
namespace mcl {
	class PaintInfomation;
	class RasPolygon;
	/**
	 * @class 软光栅基类，默认使用扫描线zbuffer，以及逐片段光照
	 */
	class SoftRenderer: public QObject
	{
		Q_OBJECT
	public:
		SoftRenderer(Scene* scene);
		SoftRenderer(Scene* scene,int px,int py);
		virtual ~SoftRenderer();

		/**
		 * 绘制场景，包括光栅化与消隐算法，默认使用扫描线zbuffer
		 */
		virtual int paint(PaintInfomation* info = nullptr);
		virtual void sceneUpdated(Scene::UpdateReason reason);
		inline Film* getFilm() const {
			return film_.get();
		}

		struct FragInfo
		{
			Color3f color;
			Point3f worldPos; //世界坐标系下的坐标
			Normal3f normal;
			double z;
		};

		static constexpr int minDepth = 0;
		static constexpr int maxDepth = MaxInt;
	protected:

		/**
		 * @brief 软片段着色器，默认使用逐片段光照
		 */
		void fragShader(const FragInfo& info,int px,int py);

		void paintFinished();

	protected:
		int createRasPolygon(RasPolygon* curpolygon, Primitive* prim, const Transform& viewProjMat, const Transform& invTotTrans, const Bound2i& screenBound, const std::vector<int>& lp, const std::vector<PType3f>& pts);

		std::unique_ptr<Film> film_;
		Scene* scene_;

		std::vector<std::shared_ptr<Light>> lights;
		Transform invViewMatrix;

		bool isPainting = false;
		Scene::UpdateReason needUpdate = Scene::NONE;

		Color3f bgcolor = Color3f(0, 0, 0);

		bool bindViewportToScene;
	};
}
