#include "CubeShadowMapPaintVisitor.h"
#include "Light.h"
#include "shaders.h"
#include "PaintInformation.h"
#include "PTriMesh.h"
#include "GLFunctions.h"
namespace mcl{
	
	CubeShadowMapPaintVisitor::CubeShadowMapPaintVisitor(std::shared_ptr<PointLight> light)
		:light(light)
	{
	}

	int CubeShadowMapPaintVisitor::paintTris(PaintInfomation* info, PTriMesh* tri)
	{
		QMatrix4x4 tempTrans_ = info->modelMat;
		info->modelMat = info->modelMat * tri->localTransform().toQMatrix();

		QOpenGLShaderProgram* shader = CubeShaodwMapShader::ptr();
		//setup uniforms
		shader->bind();
		shader->setUniformValue("modelMat", info->modelMat);
		for (int i = 0; i < 6; i++) {
			shader->setUniformValue(("shadowMatrices[" + QString::number(i) + "]").toStdString().c_str(), light->getProjectMatrix() * light->getViewMatrix(PointLight::LightOrient(i)));
		}
		shader->setUniformValue("lightPos", QVector3D(light->bound().mid()));
		shader->setUniformValue("farPlane", light->getFarPlane());
		shader->setUniformValue("nearPlane", light->getNearPlane());

		tri->getVAO()->bind();
		GLFUNC->glDrawArrays(GL_TRIANGLES, 0, tri->getTriNumer() * 3);
		shader->release();

		info->modelMat = tempTrans_;
	}

	int CubeShadowMapPaintVisitor::initTris(PTriMesh* tri)
	{
		return 0;
	}

}