#include "PLines.h"
#include "PPolygon.h"
#include "utilities.h"
#include "PaintInformation.h"
#include "shaders.h"
#include "PaintVisitor.h"
#include "GLFunctions.h"
namespace mcl{
	
	PLines::~PLines()
	{
		vbo.destroy();
		vao.destroy();
	}

	void PLines::initializeGL()
	{
		if (vbo.isCreated())
			vbo.destroy();
		if (vao.isCreated())
			vao.destroy();

		for (const auto& pts : lp_) {
			pts_.push_back(pts.x());
			pts_.push_back(pts.y());
			pts_.push_back(pts.z());
		}

		vao.create();
		vao.bind();
		vbo.create();
		vbo.bind();
		vbo.allocate(&pts_[0], pts_.size() * sizeof(Float));

		GLFUNC->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(Float), 0);
		GLFUNC->glEnableVertexAttribArray(0);
	}

	void PLines::paint(PaintInfomation* info, PaintVisitor* visitor)
	{
		visitor->paintLines(info, this);
	}
}