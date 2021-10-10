#include "PaintInformation.h"
#include <QOpenGLShaderProgram>
#include "light.h"
#include "GLFunctions.h"
#include "GLTexture.h"
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
	case mcl::DEFFER_LIGHTING:
		shader->setUniformValue("world2Screen", projMat * viewMat);
		shader->setUniformValue("lightCount", GLint(lights.size()));
		mtrTex[0]->bindToUniform("fragColor", shader);
		mtrTex[1]->bindToUniform("albedo", shader);
		mtrTex[2]->bindToUniform("worldPos", shader);
		mtrTex[3]->bindToUniform("normal", shader);
		mtrTex[4]->bindToUniform("depth", shader);
		for (int j = 0; j < lights.size(); j++) {
			lights[j]->bind(shader, j, this->activeTextrueCnt);
		}
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
