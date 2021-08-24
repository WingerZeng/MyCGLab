#pragma once
#include "mcl.h"
#include "types.h"
#include "camera.h"
namespace mcl {
	class ViewCamera:public Camera
	{
	public:
		ViewCamera(QVector3D position, QVector3D up, QVector3D front);

		virtual void initialize(Float sceneWidth, Float sceneHeight) override;		
		
		virtual QMatrix4x4 getViewMatrix() override;

		virtual QMatrix4x4 getViewMatrixForAxis() override;

		virtual QMatrix4x4 getProjMatrixForAxis() override;

		virtual void setLookAt(const QVector3D& position, const QVector3D& up, const QVector3D& center);

		void getLookAt(QVector3D& position, QVector3D& up, QVector3D& center);
	protected:
		QVector3D position;
		QVector3D up;
		QVector3D front;


		Float width;
		Float height;

	};
}

