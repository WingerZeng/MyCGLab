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
		std::vector<PType3f> lp_; // #PERF1 �����Ա���Ըĳ�ָ�룬�����ͷ�
		std::vector<Float> pts_;
		bool isloop;

		QOpenGLShaderProgram lineShaderProgram; // #PERF1 ��ɫ������prototypeģʽ�Ż�
		QOpenGLBuffer vbo;
		QOpenGLVertexArrayObject vao;
	};
}

