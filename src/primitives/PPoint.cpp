#include "PPoint.h"
#include "PPolygon.h"
#include "utilities.h"
#include "PaintInformation.h"
#include "shaders.h"
#include "PaintVisitor.h"
#include "GLFunctions.h"
namespace mcl{

	PPoint::~PPoint()
	{
		vao.destroy();
		vbo.destroy();
	}

	void PPoint::initializeGL()
	{
		if (vbo.isCreated())
			vbo.destroy();
		if (vao.isCreated())
			vao.destroy();

		vao.create();
		vao.bind();
		vbo.create();
		vbo.bind();
		vbo.allocate(cd, 3 * sizeof(Float));

		GLFUNC->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(Float), 0);
		GLFUNC->glEnableVertexAttribArray(0);
	}

	void PPoint::paint(PaintInfomation* info, PaintVisitor* visitor)
	{
		visitor->paintPoint(info, this);
	}
}