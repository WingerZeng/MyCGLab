#include "PaintInformation.h"
#include <QOpenGLShaderProgram>
#include "light.h"
#include "GLFunctions.h"
#include "GLTexture.h"
#include "GLFrameBufferObject.h"
//#TEST
#include "MainWindow.h"
#include "Scene.h"

void mcl::PaintInfomation::setUniformValue(QOpenGLShaderProgram* shader, PaintStage stage)
{
	switch (stage)
	{
	case mcl::LIGHT_PREPARE:
		break;
	case mcl::DEFFER_PREPARE:
		shader->setUniformValue("modelMatInv", modelMat.inverted());
		shader->setUniformValue("modelMat", modelMat);
		shader->setUniformValue("modelViewMatInv", (viewMat * modelMat).inverted());
		shader->setUniformValue("viewMat", viewMat);
		shader->setUniformValue("projMat", projMat);
		shader->setUniformValue("selected", GLint(selected));
		shader->setUniformValue("selectedColor", QVector3D(selectedColor));
		shader->setUniformValue("hasNormal", hasNormal);
		break;
	case mcl::DEFFER_DIRECT_LIGHT:
		shader->setUniformValue("world2Screen", projMat * viewMat);
		shader->setUniformValue("lightCount", GLint(lights.size()));
		shader->setUniformValue("sceneExtent", (sceneBnd.pMax() - sceneBnd.pMin()).length());
		mtrTex[GLMtrFrameBufferObject::ALBEDO   ]->bindToUniform("albedo", shader);
		mtrTex[GLMtrFrameBufferObject::WORLD_POS]->bindToUniform("worldPos", shader);
		mtrTex[GLMtrFrameBufferObject::NORMAL   ]->bindToUniform("normal", shader);
		mtrTex[GLMtrFrameBufferObject::DEPTH    ]->bindToUniform("depth", shader);
		for (int j = 0; j < lights.size(); j++) {
			lights[j]->bind(shader, j, this->activeTextrueCnt);
		}
		break;
	case mcl::DEFFER_SSDO:
		shader->setUniformValue("world2Screen", projMat * viewMat);
		shader->setUniformValue("lightCount", GLint(lights.size()));
		shader->setUniformValue("sceneExtent", (sceneBnd.pMax() - sceneBnd.pMin()).length());
		mtrTex[GLMtrFrameBufferObject::ALBEDO]->bindToUniform("albedo", shader);
		mtrTex[GLMtrFrameBufferObject::WORLD_POS]->bindToUniform("worldPos", shader);
		mtrTex[GLMtrFrameBufferObject::NORMAL]->bindToUniform("normal", shader);
		mtrTex[GLMtrFrameBufferObject::DEPTH]->bindToUniform("depth", shader);
		directLightTexture->bindToUniform("directLightTexture", shader);
		for (int j = 0; j < lights.size(); j++) {
			lights[j]->bind(shader, j, this->activeTextrueCnt);
		}
		break;
	case mcl::DEFFER_COMPOSITE:
		mtrTex[GLMtrFrameBufferObject::COLOR]->bindToUniform("fragColor", shader);
		mtrTex[GLMtrFrameBufferObject::PRIMID]->bindToUniform("primId", shader);
		directLightTexture->bindToUniform("directLightTexture", shader);
		ssdoTexture->bindToUniform("ssdoTexture", shader);
		break;
	case DEFFER_SSR:
		lightCompositedTexture->bindToUniform("gFinalImage", shader);
		mtrTex[GLMtrFrameBufferObject::ALBEDO]->bindToUniform("gAlbedo", shader);
		mtrTex[GLMtrFrameBufferObject::SPECULAR]->bindToUniform("gSpecular", shader);
		mtrTex[GLMtrFrameBufferObject::NORMAL]->bindToUniform("gWorldNormal", shader);
		mtrTex[GLMtrFrameBufferObject::WORLD_POS]->bindToUniform("gWorldPos", shader);
		mtrTex[GLMtrFrameBufferObject::DEPTH]->bindToUniform("gDepth", shader);
		shader->setUniformValue("viewportSize", width, height);
		shader->setUniformValue("sceneExtent", (sceneBnd.pMax() - sceneBnd.pMin()).length());
		shader->setUniformValue("view", viewMat);
		shader->setUniformValue("invView", viewMat.inverted());
		shader->setUniformValue("project", projMat);
		shader->setUniformValue("invProject", projMat.inverted());
		shader->setUniformValue("bgColor", QVector3D(clearColor));
		break;
	case mcl::TONE_MAP:
		mtrTex[GLMtrFrameBufferObject::PRIMID]->bindToUniform("primId", shader);
		finalHdrTexture->bindToUniform("finalHdrTex", shader);
		bloomMipTex[0]->bindToUniform("bloomMip0", shader);
		shader->setUniformValue("clearColor", QVector3D(clearColor));
		break;
	case mcl::FXAA:
		finalLdrTexture->bindToUniform("ldrTexture", shader);
		shader->setUniformValue("viewportSize", width, height);
		break;
	case mcl::FORWARD_SHADING:
		shader->setUniformValue("u_viewportSize", width, height);
		shader->setUniformValue("u_thickness", GLfloat(lineWidth));
		shader->setUniformValue("pointSize", GLfloat(pointSize));
		break;
	default:
		break;
	}
}
