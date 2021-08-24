#ifndef CAMERA_H
#define CAMERA_H
#include <QVector3D>
#include <QMatrix4x4>
#include <QMouseEvent>
#include "core/types.h"
#include "RTCamera.h"
namespace mcl {
	class Camera {
	public:
		virtual ~Camera() {};

		virtual QMatrix4x4 getViewMatrix() = 0;

		virtual QMatrix4x4 getViewMatrixForAxis() = 0;

		virtual QMatrix4x4 getProjMatrix() = 0;

		virtual QMatrix4x4 getProjMatrixForAxis() = 0;

		virtual void mouseMoveEvent(QMouseEvent *ev) {};
		virtual void wheelEvent(QWheelEvent *ev) {};
		virtual void mousePressEvent(QMouseEvent *ev) {};
		virtual void keyPressEvent(QKeyEvent *ev) {};
		virtual void keyReleaseEvent(QKeyEvent *ev) {};

		virtual std::shared_ptr<RTCamera> createRTCamera() = 0;

		virtual void initialize(Float sceneWidth, Float sceneHeight) = 0;

		virtual int scaleZoom(Float factor) = 0;
		virtual int rotate(Vector3f axis, Float degree) = 0;
	};

}


#endif // CAMERA_H