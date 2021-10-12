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

	enum PaintStage {
		LIGHT_PREPARE,
		DEFFER_PREPARE,
		DEFFER_DIRECT_LIGHT,
		DEFFER_SSDO,
		DEFFER_COMPOSITE,
		FORWARD_SHADING
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
		bool hasNormal = false;
		Color3f selectedColor = Color3f(1,0,0);
		int activeTextrueCnt = 0;
		void setUniformValue(QOpenGLShaderProgram* shader, PaintStage stage);
		Bound3f sceneBnd;

		std::vector<std::shared_ptr<GLAbstractTexture>> mtrTex;
		std::shared_ptr<GLAbstractTexture> directLightTexture; //vec4(Œ¥º”“ı”∞µƒdirectLightColor, shadowRate)
		std::shared_ptr<GLAbstractTexture> ssdoTexture;
		std::shared_ptr<GLAbstractTexture> finalHdrTexture;
		std::vector<std::shared_ptr<GLAbstractTexture>> bloomMipTex;
		int bloomSampleState; //if (0,), down sample from Mip i-1 to Mip i. if (,0), up sample from Mip -i to Mip -i-1
	};
}