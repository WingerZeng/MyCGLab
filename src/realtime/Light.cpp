#include "Light.h"
#include "Primitive.h"
#include <QOpenGLShaderProgram>
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
		return Color3f(0.2, 0.2, 0.2);
	}

	mcl::Bound3f Light::bound()
	{
		return bd;
	}

	std::shared_ptr<mcl::Primitive> Light::primitive()
	{
		return prim;
	}

	void Light::bind(QOpenGLShaderProgram* shader, int index)
	{
		std::string lightname = ("lights[" + std::to_string(index) + "]").c_str();
		shader->setUniformValue((lightname + ".ambient").c_str(), QVector3D(ambient()));
		shader->setUniformValue((lightname + ".emission").c_str(), QVector3D(emission()));
		shader->setUniformValue((lightname + ".pos").c_str(), QVector3D(position()));
		shader->setUniformValue((lightname + ".bound[0]").c_str(), QVector3D(bd.pMin()));
		shader->setUniformValue((lightname + ".bound[1]").c_str(), QVector3D(bd.pMax()));
		shader->setUniformValue((lightname + ".castShadow").c_str(), castShadow());
	}

	bool Light::castShadow() {
		return false;
	}

	QMatrix4x4 PointLight::getProjectMatrix()
	{
		QMatrix4x4 mat;
		mat.perspective(90, 1, 0.01, FarPlane);
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
			up = QVector3D(0, 1, 0);
			break;
		case mcl::PointLight::NEG_X:
			front = QVector3D(-1, 0, 0);
			up = QVector3D(0, 1, 0);
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
			up = QVector3D(0, 1, 0);
			break;
		case mcl::PointLight::NEG_Z:
			front = QVector3D(0, 0, -1);
			up = QVector3D(0, 1, 0);
			break;
		default:
			break;
		}
		mat.lookAt(pos,pos+front,up);
		return mat;
	}

	void PointLight::bind(QOpenGLShaderProgram* shader, int index)
	{
		std::string lightname = ("lights[" + std::to_string(index) + "]").c_str();
		//#TODO0 bind texture
	}

	bool PointLight::castShadow()
	{
		return true;
	}

}

