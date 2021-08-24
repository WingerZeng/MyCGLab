#pragma once
#include "mcl.h"
#include "types.h"
#include "ViewCamera.h"
namespace mcl {
	class OrthoCamera: public ViewCamera
	{
	public:
		OrthoCamera(QVector3D position, QVector3D up);

		virtual QMatrix4x4 getProjMatrix() override;


		virtual void mouseMoveEvent(QMouseEvent *ev) override;


		virtual void wheelEvent(QWheelEvent *ev) override;


		virtual void mousePressEvent(QMouseEvent *ev) override;


		virtual std::shared_ptr<RTCamera> createRTCamera() override;

		virtual int scaleZoom(Float factor) override;

		virtual int rotate(Vector3f axis, Float degree) override;

	private:
		//缩放
		virtual void fitToBound(const Bound3f& bd);

		//合适的缩放大小
		virtual Float fitZoom() { return ZOOM; }

		Float zoom() { return zoom_; }
		void setZoom(Float zoom) { zoom_ = zoom; }

		void zoomControl(int delta);


		//输入网格尺寸，据此设置相机缩放范围以及默认缩放大小
		void setMaxZoom(Float meshSize);

	private:
		Float zoom_ = 1.0f;

		//相机缩放默认值及范围
		Float ZOOM = 1.0f;
		Float MAX_ZOOM = 1e2f;
		Float MIN_ZOOM = 1e-2f;

		Float maxMeshSize_ = 0.0f;

		QVector2D mousePos;
		QVector2D wheelPos;

		QVector3D currentPosition;
		QVector3D currentFront;
		QVector3D currentUp;
	};
}

