#pragma once
#include "mcl.h"
#include "types.h"
#include "Primitive.h"
#include <QtGui/QOpenGLVertexArrayObject>
#include <QtGui/QOpenGLBuffer>
#include <QtGui/QOpenGLShaderProgram>
namespace mcl {
	class PPoint:public GeometryPrimitive
	{
		RTTI_CLASS(PPoint)
	public:
		PPoint(const Point3f& coord)
			:cd{coord.x(),coord.y(),coord.z()}{}
		~PPoint();
		virtual void initializeGL() override;


		virtual void paint(PaintInfomation* info, PaintVisitor* visitor) override;
		

		QOpenGLVertexArrayObject& getVAO() { return vao; }
		QOpenGLBuffer& getVBO() { return vbo; }

	private:
		Point3f coord_; // #PERF1 �����Ա���Ըĳ�ָ�룬�����ͷ�
		Float cd[3];

		QOpenGLShaderProgram lineShaderProgram; // #PERF1 ��ɫ������prototypeģʽ�Ż�
		QOpenGLBuffer vbo;
		QOpenGLVertexArrayObject vao;
	};
}

