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
		//����
		virtual void fitToBound(const Bound3f& bd);

		//���ʵ����Ŵ�С
		virtual Float fitZoom() { return ZOOM; }

		Float zoom() { return zoom_; }
		void setZoom(Float zoom) { zoom_ = zoom; }

		void zoomControl(int delta);


		//��������ߴ磬�ݴ�����������ŷ�Χ�Լ�Ĭ�����Ŵ�С
		void setMaxZoom(Float meshSize);

	private:
		Float zoom_ = 1.0f;

		//�������Ĭ��ֵ����Χ
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

