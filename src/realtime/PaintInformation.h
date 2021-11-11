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
		DIRECT_LIGHT,
		DIRECT_LIGHT_FILTER,
		SSR,
		SSR_FILTER,
		SSDO,
		SSDO_FILTER,
		BLOOM,
		TONE_MAP,
		FXAA,
		FORWARD_SHADING,
	};

	class PaintInfomation
	{
	public:
		QMatrix4x4 projMat, viewMat, modelMat;
		std::vector<std::shared_ptr<Light>> lights;
		FillMode fillmode;
		double lineWidth;
		float pointSize;
		float width, height;
		bool selected = false;
		bool hasNormal = false;
		Color3f selectedColor = Color3f(1,0,0);
		int activeTextrueCnt = 0;
		void setUniformValue(QOpenGLShaderProgram* shader, PaintStage stage);
		Bound3f sceneBnd;

		std::vector<std::shared_ptr<GLAbstractTexture>> mtrTex;
		std::shared_ptr<GLAbstractTexture> directLightTexture; //vec4(未加阴影的directLightColor, shadowRate)
		std::shared_ptr<GLAbstractTexture> directLightFilterTexture; //vec4(未加阴影的directLightColor, shadowRate)
		std::shared_ptr<GLAbstractTexture> ssdoTexture;
		std::shared_ptr<GLAbstractTexture> ssdoFilterTexture;
		std::shared_ptr<GLAbstractTexture> ssrTexture;
		std::shared_ptr<GLAbstractTexture> ssrFilterTexture;
		std::shared_ptr<GLAbstractTexture> finalHdrTexture;
		std::shared_ptr<GLAbstractTexture> finalLdrTexture;
		std::vector<std::shared_ptr<GLAbstractTexture>> bloomMipTex;
		int bloomSampleState; //if (0,), down sample from Mip i-1 to Mip i. if (,0), up sample from Mip -i to Mip -i-1

		Color3f clearColor = Color3f(0.7, 0.7, 0.8);
		
		Color3f emptyPrimIdColor = Color3f(1, 1, 1);
	};
}