#include "PerspectiveCamera.h"
#include "RTCamera.h"
#include "algorithms.h"
namespace mcl{
	
	PerspectiveCamera::PerspectiveCamera()
		:ViewCamera(QVector3D(0, 0, 3), QVector3D(0, 1, 0), QVector3D(0, 0, -1)), keyStartTime{ 0 }, keyCurTime{ 0 }
	{
		updateLookat();
	}

	QMatrix4x4 PerspectiveCamera::getProjMatrix()
	{
		QMatrix4x4 proj;
		proj.perspective(fov, width / height, 0.1, 1e3);
		return proj;
	}

	QMatrix4x4 PerspectiveCamera::getViewMatrix()
	{
		doMove();
		return ViewCamera::getViewMatrix();
	}

	void PerspectiveCamera::mouseMoveEvent(QMouseEvent *ev)
	{
		//鼠标右键控制旋转
		if (ev->buttons() & Qt::LeftButton)
		{
			Float dx = (ev->x() - mousePos.x()) / width;
			Float dy = (ev->y() - mousePos.y()) / height;
			mousePos.setX(ev->x());
			mousePos.setY(ev->y());

			QQuaternion rotation;

			//横向旋转
			rotation = QQuaternion::fromAxisAndAngle(up,- 50 * dx);
			horizonFront = rotation.rotatedVector(horizonFront);
			horizonFront.normalize();

			//纵向旋转
			yaw -= 50 * dy;
			yaw = std::clamp(yaw, MIN_YAW, MAX_YAW);
			rotation = QQuaternion::fromAxisAndAngle(QVector3D::crossProduct(horizonFront, up).normalized(), yaw);
			front = rotation.rotatedVector(horizonFront);
			front.normalize();
		}
	}

	void PerspectiveCamera::wheelEvent(QWheelEvent *ev)
	{
		if (ev->delta() > 0) {
			fov *= 0.9;
		}
		else {
			fov *= 1.1;
		}
		fov = std::clamp(fov, MIN_FOV, MAX_FOV);
	}

	void PerspectiveCamera::mousePressEvent(QMouseEvent *ev)
	{		
		//获取点击时状态
		if (ev->button() == Qt::LeftButton) {
			mousePos.setX(ev->x());
			mousePos.setY(ev->y()); 
		}
	}

	void PerspectiveCamera::keyPressEvent(QKeyEvent *ev)
	{
		if (ev->isAutoRepeat()) return;
		if (ev->key() == Qt::Key_W) {
			keyStartTime[0] = keyCurTime[0] = clock();
		}
		if (ev->key() == Qt::Key_A) {
			keyStartTime[1] = keyCurTime[1] = clock();
		}
		if (ev->key() == Qt::Key_S) {
			keyStartTime[2] = keyCurTime[2] = clock();
		}
		if (ev->key() == Qt::Key_D) {
			keyStartTime[3] = keyCurTime[3] = clock();
		}
	}

	void PerspectiveCamera::keyReleaseEvent(QKeyEvent *ev)
	{
		if (ev->isAutoRepeat()) return;
		doMove();
		if (ev->key() == Qt::Key_W) {
			keyStartTime[0] = keyCurTime[0] = 0;
		}
		if (ev->key() == Qt::Key_A) {
			keyStartTime[1] = keyCurTime[1] = 0;
		}
		if (ev->key() == Qt::Key_S) {
			keyStartTime[2] = keyCurTime[2] = 0;
		}
		if (ev->key() == Qt::Key_D) {
			keyStartTime[3] = keyCurTime[3] = 0;
		}
	}

	std::shared_ptr<mcl::RTCamera> PerspectiveCamera::createRTCamera()
	{
		doMove();
		auto lookat = Transform::lookAt(Point3f(position), Vector3f(front), Vector3f(up));
		return std::make_shared<RTPerspectiveCamera>(degreeToRad(fov), width*1.0 / height, lookat);
	}

	int PerspectiveCamera::scaleZoom(Float factor)
	{
		if (factor <= 0) return 0;
		fov = fov * factor;
		fov = std::clamp(fov, MIN_FOV, MAX_FOV);
		return 0;
	}

	int PerspectiveCamera::rotate(Vector3f axis, Float degree)
	{
		auto rotation = QQuaternion::fromAxisAndAngle(QVector3D(axis), degree);
		front = rotation.rotatedVector(front);
		horizonFront = QVector3D::crossProduct(up, QVector3D::crossProduct(front, up));
		front.normalize();
		horizonFront.normalize();
		return 0;
	}

	void PerspectiveCamera::setLookAt(const QVector3D& position, const QVector3D& up, const QVector3D& center)
	{
		ViewCamera::setLookAt(position, up, center);
		updateLookat();
	}

	int PerspectiveCamera::setFov(Float afov)
	{
		this->fov = afov;
		return 0;
	}

	void PerspectiveCamera::doMove()
	{
		Float displace[4];
		for (int i = 0; i < 4; i++) {
			if (keyStartTime[i]) {
				clock_t newtime = clock();
				Float dt = (newtime - keyCurTime[i]) * 1.0 / CLOCKS_PER_SEC;
				Float dt_total = (newtime - keyStartTime[i]) * 1.0 / CLOCKS_PER_SEC;
				keyCurTime[i] = newtime;
				displace[i] = dt * (dt_total)* (dt_total) * Speed; //速度指数增长
				//std::cout << dt << ' ' << dt_total << ' ' << displace[i] << std::endl;
			}
			else {
				displace[i] = 0;
			}
		}
		position += front * displace[0] - horizonRight() * displace[1] - front * displace[2] + horizonRight() *displace[3];
	}

	void PerspectiveCamera::updateLookat()
	{
		horizonFront = QVector3D::crossProduct(up, QVector3D::crossProduct(front, up));
		horizonFront.normalize();
		yaw = 90 - (std::acos(QVector3D::dotProduct(front,up)) / PI * 180);
	}

	QVector3D PerspectiveCamera::horizonRight()
	{
		QVector3D vec = QVector3D::crossProduct(horizonFront, up);
		vec.normalize();
		return vec;
	}

}