#include "Light.h"
#include "Primitive.h"
#include <QOpenGLShaderProgram>
#include "GLFrameBufferObject.h"
#include "GLFunctions.h"
namespace mcl {

	Light::Light(const Color3f& e, const std::shared_ptr<Primitive>& prim)
		:e(e), prim(prim), bd(prim->getBound())
	{

	}

	mcl::Color3f Light::emission()
	{
		return e;
	}

	mcl::Point3f Light::position()
	{
		return bd.mid();
	}

	mcl::Color3f Light::ambient()
	{
		return Color3f(0.05, 0.05, 0.05);
	}

	mcl::Bound3f Light::bound()
	{
		return bd;
	}

	std::shared_ptr<mcl::Primitive> Light::primitive()
	{
		return prim;
	}

	void Light::bind(QOpenGLShaderProgram* shader, int lightIdx, int& textureIdx)
	{
		std::string lightname = ("lights[" + std::to_string(lightIdx) + "]").c_str();
		shader->setUniformValue((lightname + ".ambient").c_str(), QVector3D(ambient()));
		shader->setUniformValue((lightname + ".emission").c_str(), QVector3D(emission()));
		shader->setUniformValue((lightname + ".pos").c_str(), QVector3D(position()));
		shader->setUniformValue((lightname + ".bound[0]").c_str(), QVector3D(bd.pMin()));
		shader->setUniformValue((lightname + ".bound[1]").c_str(), QVector3D(bd.pMax()));
		shader->setUniformValue((lightname + ".castShadow").c_str(), castShadow());
		shader->setUniformValue((lightname + ".shadowOffset").c_str(),GLfloat(0.02));
	}

	bool Light::castShadow() {
		return false;
	}

	PointLight::PointLight(const mcl::Color3f& e, const std::shared_ptr<mcl::Primitive>& prim, GLuint smWidth/*=1024*/, GLuint smHeight/*=1024*/)
		: Light(e, prim), smHeight(smHeight), smWidth(smWidth)
	{
		Bound3f bd = prim->getBound();
		halfArea += (bd.pMax().x() - bd.pMin().x()) * (bd.pMax().y() - bd.pMin().y());
		halfArea += (bd.pMax().y() - bd.pMin().y()) * (bd.pMax().z() - bd.pMin().z());
		halfArea += (bd.pMax().z() - bd.pMin().z()) * (bd.pMax().x() - bd.pMin().x());
	}

	PointLight::~PointLight()
	{
	}

	QMatrix4x4 PointLight::getProjectMatrix()
	{
		QMatrix4x4 mat;
		mat.perspective(90, 1, NearPlane, FarPlane);
		return mat;
	}

	QMatrix4x4 PointLight::getViewMatrix(LightOrient ori)
	{
		QMatrix4x4 mat;
		QVector3D pos(position());
		QVector3D front;
		QVector3D up;
		switch(ori)
		{
		case mcl::PointLight::POS_X:
			front = QVector3D(1, 0, 0);
			up = QVector3D(0, -1, 0);
			break;
		case mcl::PointLight::NEG_X:
			front = QVector3D(-1, 0, 0);
			up = QVector3D(0, -1, 0);
			break;
		case mcl::PointLight::POS_Y:
			front = QVector3D(0, 1, 0);
			up = QVector3D(0, 0, 1);
			break;
		case mcl::PointLight::NEG_Y:
			front = QVector3D(0, -1, 0);
			up = QVector3D(0, 0, -1);
			break;
		case mcl::PointLight::POS_Z:
			front = QVector3D(0, 0, 1);
			up = QVector3D(0, -1, 0);
			break;
		case mcl::PointLight::NEG_Z:
			front = QVector3D(0, 0, -1);
			up = QVector3D(0, -1, 0);
			break;
		default:
			break;
		}
		mat.lookAt(pos,pos+front,up);
		return mat;
	}

	void PointLight::bind(QOpenGLShaderProgram* shader, int lightIdx, int& textureIdx)
	{
		std::string lightname = ("lights[" + std::to_string(lightIdx) + "]").c_str();
		shader->setUniformValue((lightname + ".ambient").c_str(), QVector3D(ambient()));
		shader->setUniformValue((lightname + ".emission").c_str(), QVector3D(emission()));
		shader->setUniformValue((lightname + ".pos").c_str(), QVector3D(position()));
		shader->setUniformValue((lightname + ".bound[0]").c_str(), QVector3D(bd.pMin()));
		shader->setUniformValue((lightname + ".bound[1]").c_str(), QVector3D(bd.pMax()));
		shader->setUniformValue((lightname + ".castShadow").c_str(), castShadow());
		shader->setUniformValue((lightname + ".farPlane").c_str(), FarPlane);
		shader->setUniformValue((lightname + ".nearPlane").c_str(), NearPlane);
		shader->setUniformValue((lightname + ".shadowOffset").c_str(), shadowOffset);
		shader->setUniformValue((lightname + ".mapSize").c_str(), smWidth);
		GLFUNC->glActiveTexture(GL_TEXTURE0 + textureIdx);
		GLFUNC->glBindTexture(GL_TEXTURE_CUBE_MAP, fbo->textureId());
		shader->setUniformValue((lightname + ".shadowMap").c_str(), textureIdx++);
	}

	std::shared_ptr<mcl::GLShadowMapFrameBufferObject> PointLight::getFbo()
	{
		return fbo;
	}

	void PointLight::initFbo()
	{
		if(!fbo)
			fbo = std::make_shared<GLShadowMapFrameBufferObject>();
		fbo->resize(smWidth, smWidth);
	}

	bool PointLight::castShadow()
	{
		return true;
	}

	void PointLight::setShadowOffset(GLfloat offset)
	{
		shadowOffset = offset;
	}

}

