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
		// Scene Config
		FillMode fillmode;
		double lineWidth;
		float pointSize;
		Color3f selectedColor = Color3f(1, 0, 0);
		Color3f clearColor = Color3f(0.7, 0.7, 0.8);
		Color3f emptyPrimIdColor = Color3f(1, 1, 1);

		// Scene Info
		float width, height;
		bool selected = false;
		bool hasNormal = false;
		Bound3f sceneBnd;
		QMatrix4x4 projMat, viewMat, modelMat;
		std::vector<std::shared_ptr<Light>> lights;
		int activeTextrueCnt = 0;
		std::map<int, std::shared_ptr<Primitive>>* prims;

		// Painter Texture
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
		std::vector<std::shared_ptr<GLAbstractTexture>> hiZMipTex;

		// Painter State
		int bloomSampleState; //if (0,), down sample from Mip i-1 to Mip i. if (,0), up sample from Mip -i to Mip -i-1
		int hiZMipCnt;
	};
}