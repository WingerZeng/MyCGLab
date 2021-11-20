#pragma once

#include <set>
#include <memory>
#include <QtWidgets/QOpenGLWidget>
#include <QVector3D>
#include "mcl.h"
#include "axis.h"
#include "camera.h"
#include "PaintInformation.h"
#include "Painter.h"

class QOpenGLDebugLogger;

namespace mcl {
	class Primitive;

	class Scene : public QOpenGLWidget
	{
		Q_OBJECT

	public:
		Scene(QWidget* parent = nullptr);
		~Scene();
		
		void addPrimitive(std::shared_ptr<Primitive> prim);
		void delPrimitive(int id);
		template <class T>
		void delPrimitives(const std::vector < std::shared_ptr<T>>& prims);
		template <class T>
		void addPrimitives(const std::vector < std::shared_ptr<T>>& prims);
		static void debugOpenGL();

		int wireFrameMode(bool wfmode);

		int setCameraLookAt(const Point3f& pos, const Vector3f& up, const Vector3f& center);
		int printCameraLookAt();
		int setCameraFov(Float fov);

		inline Camera& getCamera() { return *camera; }
		const std::map<int, std::shared_ptr<Primitive>>& getPrimitives();

		enum UpdateReason {
			NONE				= 00,
			PRIMITIVE			= 01,
			CAMERA				= 02,
			PAINTING_INFO		= 04,
			VIEWPORT			= 010,
			ALL					= 0777
		};
		void updateScene(UpdateReason reason);

		void prepareLight();

		std::shared_ptr<RTScene> createRTScene();

		int setSkyBox(PathString path) {
			if (!path.exist()) return -1;
			rtinfo.skyboxpath = QString::fromStdString(path.str);
			prepareLight();
			return 0;
		}


	protected:
		virtual void initializeGL() override;
		virtual void resizeGL(int w, int h) override;
		virtual void paintGL() override;
		virtual void mouseMoveEvent(QMouseEvent *ev) override;
		virtual void wheelEvent(QWheelEvent *ev) override;
		virtual void mousePressEvent(QMouseEvent *ev) override;
		virtual void keyPressEvent(QKeyEvent* ev) override;
		virtual void keyReleaseEvent(QKeyEvent* ev) override;
		void doPrimAdd();

	signals:
		void updated(UpdateReason reason);

	private:
		struct RayTraceInfo {
			QString skyboxpath;
		}rtinfo;

		std::shared_ptr<Camera> camera;
		Axis axis;

		//鼠标点选相关
		bool hitted = false;
		QVector2D hitPoint;

		std::map<int, std::shared_ptr<Primitive>> prims_;
		std::vector<std::shared_ptr<Primitive>> primsToAdd;
		std::vector<std::shared_ptr<Light>> lights_;

		static QOpenGLDebugLogger* logger;

		bool wfmode_; // mode of wire frame
		bool bNeedInitLight = false;

		static const int MaxBloomMipLevel = 5;
		static const int BloomMipStopSize = 8;

		std::shared_ptr<AbstractGLFrameBufferObject> pingpongFbo[3];

		std::shared_ptr<Painter> lightSMPainter;
		std::shared_ptr<Painter> mtrPainter;
		std::shared_ptr<Painter> directLightPainter;
		std::shared_ptr<Painter> directLightFilterPainter;
		std::shared_ptr<Painter> hizFilterPainter;
		std::shared_ptr<Painter> ssrPainter;
		std::shared_ptr<Painter> ssrFilterPainter;
		std::shared_ptr<Painter> ssdoPainter;
		std::shared_ptr<Painter> ssdoFilterPainter;
		std::shared_ptr<Painter> bloomFilterPainter;
		std::shared_ptr<Painter> toneMapPainter;
		std::shared_ptr<Painter> fxaaPainter;

		Bound3f sceneBound;

		PaintInfomation info;
	};

	template<class T>
	void Scene::addPrimitives(const std::vector < std::shared_ptr<T>>& prims)
	{
		for (const auto& prim : prims) {
			addPrimitive(prim);
		}
		updateScene(PRIMITIVE);
	}

	template <class T>
	void Scene::delPrimitives(const std::vector < std::shared_ptr<T>>& prims)
	{
		for (const auto& prim : prims) {
			delPrimitive(prim->id());
		}
		updateScene(PRIMITIVE);
	}
}