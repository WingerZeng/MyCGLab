#include "ViewCamera.h"

namespace mcl{
	
	ViewCamera::ViewCamera(QVector3D position, QVector3D up, QVector3D front)
		:position(position), up(up), front(front)
	{

	}

	void ViewCamera::initialize(Float sceneWidth, Float sceneHeight)
	{
		width = sceneWidth;
		height = sceneHeight;
	}

	QMatrix4x4 ViewCamera::getViewMatrix()
	{
		QMatrix4x4 view;
		view.lookAt(this->position, this->position + this->front, this->up);
		return view;
	}

	QMatrix4x4 ViewCamera::getViewMatrixForAxis()
	{
		QMatrix4x4 view;
		view.lookAt(-this->front, QVector3D(0, 0, 0), this->up);
		return view;
	}

	QMatrix4x4 ViewCamera::getProjMatrixForAxis()
	{
		QMatrix4x4 proj;
		proj.ortho(-width / height, width / height, -1, 1, -5, 5);
		return proj;
	}

	void ViewCamera::setLookAt(const QVector3D& aposition, const QVector3D& aup, const QVector3D& acenter)
	{
		this->position = aposition;
		this->up = Normalize(aup);
		this->front = Normalize(acenter - aposition);
	}

	void ViewCamera::getLookAt(QVector3D& aPosition, QVector3D& aUp, QVector3D& center)
	{
		aPosition = position;
		aUp = up;
		center = position + front;
	}

}