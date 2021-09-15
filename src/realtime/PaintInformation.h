#pragma once
#include <memory>
#include <vector>
#include <QMatrix4x4>
#include "mcl.h"
#include "types.h"
class QOpenGLShaderProgram;
namespace mcl {
	class Light;

	enum FillMode
	{
		FILL = 0,
		WIREFRAME,
		FILL_WIREFRAME
	};

	class PaintInfomation
	{
	public:
		QMatrix4x4 projMat, viewMat, modelMat;
		std::vector<std::shared_ptr<Light>> lights;
		FillMode fillmode;
		double lineWidth;
		double pointSize;
		double width,height;
		bool selected = false;
		Color3f selectedColor = Color3f(1,0,0);
		void setUniformValue(QOpenGLShaderProgram* shader);
	};
}