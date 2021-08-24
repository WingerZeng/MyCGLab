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
	 * @class ���դ���࣬Ĭ��ʹ��ɨ����zbuffer���Լ���Ƭ�ι���
	 */
	class SoftRenderer: public QObject
	{
		Q_OBJECT
	public:
		SoftRenderer(Scene* scene);
		SoftRenderer(Scene* scene,int px,int py);
		virtual ~SoftRenderer();

		/**
		 * ���Ƴ�����������դ���������㷨��Ĭ��ʹ��ɨ����zbuffer
		 */
		virtual int paint(PaintInfomation* info = nullptr);
		virtual void sceneUpdated(Scene::UpdateReason reason);
		inline Film* getFilm() const {
			return film_.get();
		}

		struct FragInfo
		{
			Color3f color;
			Point3f worldPos; //��������ϵ�µ�����
			Normal3f normal;
			double z;
		};

		static constexpr int minDepth = 0;
		static constexpr int maxDepth = MaxInt;
	protected:

		/**
		 * @brief ��Ƭ����ɫ����Ĭ��ʹ����Ƭ�ι���
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
