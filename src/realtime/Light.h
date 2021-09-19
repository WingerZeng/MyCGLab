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
		virtual void bind(QOpenGLShaderProgram* shader, int index);

		virtual bool castShadow();

	protected:
		std::shared_ptr<Primitive> prim;
		Color3f e;
		Bound3f bd;
	};

	class PointLight : public Light
	{
	public:
		using Light::Light;

		enum LightOrient
		{
			POS_X = 0,
			NEG_X,
			POS_Y,
			NEG_Y,
			POS_Z,
			NEG_Z
		};

		QMatrix4x4 getProjectMatrix();
		QMatrix4x4 getViewMatrix(LightOrient ori);
		virtual void bind(QOpenGLShaderProgram* shader, int index);

		virtual bool castShadow();
	private:
		GLCubeTexture cubeMapId;
		const double FarPlane = 1e3;
	};
}
