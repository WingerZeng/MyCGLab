#pragma once
#include "mcl.h"
#include "types.h"
#include "Primitive.h"
#include <QtGui/QOpenGLVertexArrayObject>
#include <QtGui/QOpenGLBuffer>
#include <QtGui/QOpenGLShaderProgram>
namespace mcl {
	class PLines:public GeometryPrimitive
	{
		RTTI_CLASS(PLines)
	public:
		PLines(std::vector<PType3f> pts, bool isLoop = false)
			:lp_(pts), isloop(isLoop) {}
		~PLines();
		virtual void initializeGL() override;

		virtual void paint(PaintInfomation* info, PaintVisitor* visitor) override;

		QOpenGLBuffer& getVBO() { return vbo; }
		QOpenGLVertexArrayObject& getVAO() { return vao; }

		bool isLoop() { return isloop; }

		int getPtNum() const { return pts_.size() / 3; }

	private:
		std::vector<PType3f> lp_; // #PERF1 �����Ա���Ըĳ�ָ�룬�����ͷ�
		std::vector<Float> pts_;
		bool isloop;

		QOpenGLBuffer vbo;
		QOpenGLVertexArrayObject vao;
	};
}

