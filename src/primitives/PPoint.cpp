#include "PPoint.h"
#include "PPolygon.h"
#include "utilities.h"
#include "PaintInformation.h"
#include "shaders.h"
#include "PaintVisitor.h"
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

		this->initializeOpenGLFunctions();

		vao.create();
		vao.bind();
		vbo.create();
		vbo.bind();
		vbo.allocate(cd, 3 * sizeof(Float));
	}

	void PPoint::paint(PaintInfomation* info, PaintVisitor* visitor)
	{
		visitor->paintPoint(info, this);
	}

	void PPoint::initialize(PaintVisitor* visitor)
	{
		visitor->initPoint(this);
	}

}