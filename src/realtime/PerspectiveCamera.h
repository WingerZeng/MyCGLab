#pragma once
#include "mcl.h"
#include "types.h"
#include <QTimer>
#include "ViewCamera.h"
namespace mcl {
	class PerspectiveCamera: public ViewCamera
	{
	public:
		PerspectiveCamera();

		virtual QMatrix4x4 getProjMatrix() override;

		virtual QMatrix4x4 getViewMatrix() override;

		virtual void mouseMoveEvent(QMouseEvent *ev) override;


		virtual void wheelEvent(QWheelEvent *ev) override;


		virtual void mousePressEvent(QMouseEvent *ev) override;


		virtual void keyPressEvent(QKeyEvent *ev) override;


		virtual void keyReleaseEvent(QKeyEvent *ev) override;


		virtual std::shared_ptr<RTCamera> createRTCamera() override;


		virtual int scaleZoom(Float factor) override;


		virtual int rotate(Vector3f axis, Float degree) override;

		virtual void setLookAt(const QVector3D& position, const QVector3D& up, const QVector3D& center) override;

		int setFov(Float fov);

	private:
		void doMove();
		void updateLookat();
		QVector3D horizonRight();

		const Float MAX_FOV = 170;
		const Float MIN_FOV = 10;

		Float fov = 60;
		QVector2D mousePos;
		QVector2D wheelPos;

		QVector3D horizonFront;
		const Float MAX_YAW = 89;
		const Float MIN_YAW = -89;
		Float yaw = 0;

		const Float Speed = 4;

		clock_t keyStartTime[4]; //wasd
		clock_t keyCurTime[4]; //wasd
	};
}

