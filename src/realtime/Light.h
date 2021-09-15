#pragma once
#include "mcl.h"
#include "types.h"
class QOpenGLShaderProgram;
namespace mcl {
	class PaintInfomation;

	class Light
	{
	public:
		Light(const mcl::Color3f& e, const std::shared_ptr<mcl::Primitive>& prim);

		virtual ~Light() {};
		virtual Color3f emission();
		virtual Point3f position();
		virtual Color3f ambient();
		virtual Bound3f bound();
		virtual std::shared_ptr<Primitive> primitive();
		virtual void setUniformValue(QOpenGLShaderProgram* shader, int index);

	protected:
		std::shared_ptr<Primitive> prim;
		Color3f e;
		Bound3f bd;
	};
}
