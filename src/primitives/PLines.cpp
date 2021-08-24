#include "PLines.h"
#include "PPolygon.h"
#include "utilities.h"
#include "PaintInformation.h"
#include "shaders.h"
namespace mcl{
	
	PLines::~PLines()
	{
		vbo.destroy();
		vao.destroy();
	}

	void PLines::initialize()
	{
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

		int attr = -1;
		attr = LineShader::ptr()->attributeLocation("aPos");
		LineShader::ptr()->setAttributeBuffer(attr, GL_FLOAT, 0, 3, 0);
		LineShader::ptr()->enableAttributeArray(attr);
	}

	void PLines::paint(PaintInfomation* info)
	{
		doBeforePaint(info);

		// #PERF2 改了vao，着色器可以不用重分配么？
		vao.bind();
		LineShader::ptr()->bind();
		LineShader::ptr()->setUniformValue("modelMat", QMatrix4x4());
		LineShader::ptr()->setUniformValue("viewMat", info->viewMat);
		LineShader::ptr()->setUniformValue("projMat", info->projMat);
		LineShader::ptr()->setUniformValue("ourColor",QVector4D(color().x(),color().y(),color().z(), 1.0f));
		LineShader::ptr()->setUniformValue("u_viewportSize", info->width, info->height);
		LineShader::ptr()->setUniformValue("u_thickness", GLfloat(info->lineWidth));
		if(isloop)
			glDrawArrays(GL_LINE_LOOP, 0, pts_.size() / 3);
		else
			glDrawArrays(GL_LINE_STRIP, 0, pts_.size() / 3);

		LineShader::ptr()->release();
		doAfterPaint(info);
	}

	void PLines::doBeforePaint(PaintInfomation* info)
	{
		this->glPolygonOffset(-2, -2);
		glEnable(GL_POLYGON_OFFSET_FILL);

		tempTrans_ = info->viewMat;
		info->viewMat = info->viewMat * localTransform().toQMatrix();
	}

}