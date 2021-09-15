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

	void Light::setUniformValue(QOpenGLShaderProgram* shader, int index)
	{
		std::string lightname = ("lights[" + std::to_string(index) + "]").c_str();
		shader->setUniformValue((lightname + ".ambient").c_str(), QVector3D(ambient()));
		shader->setUniformValue((lightname + ".pos").c_str(), QVector3D(position()));
		shader->setUniformValue((lightname + ".bound[0]").c_str(), QVector3D(bd.pMin()));
		shader->setUniformValue((lightname + ".bound[1]").c_str(), QVector3D(bd.pMax()));
		shader->setUniformValue((lightname + ".emission").c_str(), QVector3D(emission()));
	}

}

