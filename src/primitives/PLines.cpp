#include "PLines.h"
#include "PPolygon.h"
#include "utilities.h"
#include "PaintInformation.h"
#include "shaders.h"
#include "PaintVisitor.h"
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

		this->initializeOpenGLFunctions();

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
	}

	void PLines::paint(PaintInfomation* info, PaintVisitor* visitor)
	{
		visitor->paintLines(info, this);
	}

	void PLines::initialize(PaintVisitor* visitor)
	{
		visitor->initLines(this);
	}
}