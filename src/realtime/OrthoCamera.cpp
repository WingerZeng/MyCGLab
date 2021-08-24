#include "OrthoCamera.h"
#include "RTCamera.h"

namespace mcl{
	
	OrthoCamera::OrthoCamera(QVector3D position, QVector3D up)
		:ViewCamera(position, up, front) {}

	QMatrix4x4 OrthoCamera::getProjMatrix()
	{
		QMatrix4x4 proj;
		proj.ortho(-zoom_ * width / height, zoom_*width / height, -1 * zoom_, 1 * zoom_, 3 - MAX_ZOOM, 3 + MAX_ZOOM);
		return proj;
	}


	void OrthoCamera::mouseMoveEvent(QMouseEvent *ev)
	{
		//鼠标右键控制旋转
		if (ev->buttons() & Qt::LeftButton)
		{
			QVector2D rot;
			QQuaternion rotation;
			rot.setX((ev->x() - mousePos.x()) / width);
			rot.setY((ev->y() - mousePos.y()) / height);

			//横向旋转
			rotation = QQuaternion::fromAxisAndAngle(currentUp, -200 * rot.x());
			up = rotation.rotatedVector(currentUp);
			position = rotation.rotatedVector(currentPosition);
			front = rotation.rotatedVector(currentFront);

			//纵向旋转
			rotation = QQuaternion::fromAxisAndAngle(QVector3D::crossProduct(-currentFront, currentUp).normalized(), 200 * rot.y());
			up = rotation.rotatedVector(up);
			position = rotation.rotatedVector(position);
			front = rotation.rotatedVector(front);
		}
		//鼠标中键控制平移
		if (ev->buttons() & Qt::MidButton)
		{
			QVector2D move;
			move.setX((ev->x() - wheelPos.x()) / width);
			move.setY((ev->y() - wheelPos.y()) / height);

			position = currentPosition + QVector3D::crossProduct(-front, up).normalized() * 3 * zoom()*move.x()
				+ up.normalized() * 3 * zoom()*move.y();
		}
	}

	void OrthoCamera::wheelEvent(QWheelEvent *ev)
	{
		zoomControl(ev->delta());
	}

	void OrthoCamera::mousePressEvent(QMouseEvent *ev)
	{	
		//获取点击时状态
		if (ev->button() == Qt::LeftButton) {
			mousePos.setX(ev->x());
			mousePos.setY(ev->y());

			currentPosition = position;
			currentFront = front;
			currentUp = up;
		}

		if (ev->button() == Qt::MidButton) {
			wheelPos.setX(ev->x());
			wheelPos.setY(ev->y());

			currentPosition = position;
		}
	}

	std::shared_ptr<mcl::RTCamera> OrthoCamera::createRTCamera()
	{
		auto lookat = Transform::lookAt(Point3f(position), Vector3f(front), Vector3f(up));
		return std::make_shared<RTOrthoCamera>(-zoom_ * width / height, zoom_*width / height, -1 * zoom_, 1 * zoom_, lookat);
	}


	void OrthoCamera::fitToBound(const Bound3f& bd)
	{
		Point3f center;
		Float radius = 1;
		//bd.BoundingSphere(&center, &radius);

		zoom_ = radius;
	}

	void OrthoCamera::zoomControl(int delta)
	{
		if (delta > 0) {
			if (this->zoom_ >= MIN_ZOOM && this->zoom_ <= MAX_ZOOM)
				this->zoom_ = this->zoom_*0.9;
			if (this->zoom_ > MAX_ZOOM)
				this->zoom_ = MAX_ZOOM;
			if (this->zoom_ < MIN_ZOOM)
				this->zoom_ = MIN_ZOOM;
		}
		else {
			if (this->zoom_ >= MIN_ZOOM && this->zoom_ <= MAX_ZOOM)
				this->zoom_ = this->zoom_*1.2;
			if (this->zoom_ > MAX_ZOOM)
				this->zoom_ = MAX_ZOOM;
			if (this->zoom_ < MIN_ZOOM)
				this->zoom_ = MIN_ZOOM;
		}
	}

	
	int OrthoCamera::scaleZoom(Float factor)
	{
		if (factor <= 0) return 0;
		this->zoom_ = this->zoom_ / factor;
		return 0;
	}

	int OrthoCamera::rotate(Vector3f axis, Float degree)
	{
		auto rotation = QQuaternion::fromAxisAndAngle(QVector3D(axis), degree);

		up = rotation.rotatedVector(up);
		position = rotation.rotatedVector(position);
		front = rotation.rotatedVector(front);
		return 0;
	}

	void OrthoCamera::setMaxZoom(Float meshSize)
	{
		if (meshSize > maxMeshSize_) {
			maxMeshSize_ = meshSize;
			if (width / height < 1.0f) {
				ZOOM = maxMeshSize_ * 2.0f * height / width;
				MAX_ZOOM = ZOOM * 2.5f;
				zoom_ = ZOOM;
			}
			else {
				ZOOM = maxMeshSize_ * 2.0f;
				MAX_ZOOM = ZOOM * 2.5f;
				zoom_ = ZOOM;
			}
		}
	}

}