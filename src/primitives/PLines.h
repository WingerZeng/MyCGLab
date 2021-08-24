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
		virtual void initialize() override;
		virtual void paint(PaintInfomation* info) override;
		void doBeforePaint(PaintInfomation* info = nullptr) override;

	private:
		std::vector<PType3f> lp_; // #PERF1 这个成员可以改成指针，用完释放
		std::vector<Float> pts_;
		bool isloop;

		QOpenGLShaderProgram lineShaderProgram; // #PERF1 着色器采用prototype模式优化
		QOpenGLBuffer vbo;
		QOpenGLVertexArrayObject vao;
	};
}

