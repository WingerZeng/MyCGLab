#include "PPoint.h"
#include "PPolygon.h"
#include "utilities.h"
#include "PaintInformation.h"
#include "shaders.h"
namespace mcl{

	PPoint::~PPoint()
	{
		vao.destroy();
		vbo.destroy();
	}

	void PPoint::initialize()
	{
		this->initializeOpenGLFunctions();

		vao.create();
		vao.bind();
		vbo.create();
		vbo.bind();
		vbo.allocate(cd, 3 * sizeof(Float));

		int attr = -1;
		attr = PointShader::ptr()->attributeLocation("aPos");
		PointShader::ptr()->setAttributeBuffer(attr, GL_FLOAT, 0, 3, 0);
		PointShader::ptr()->enableAttributeArray(attr);
	}

	void PPoint::paint(PaintInfomation* info)
	{
		doBeforePaint(info);
		glPolygonOffset(-1, -1);

		// #PERF2 改了vao，着色器可以不用重分配么？
		vao.bind();
		PointShader::ptr()->bind();
		PointShader::ptr()->setUniformValue("modelMat", QMatrix4x4());
		PointShader::ptr()->setUniformValue("viewMat", info->viewMat);
		PointShader::ptr()->setUniformValue("projMat", info->projMat);
		PointShader::ptr()->setUniformValue("ourColor", QVector4D(color().x(), color().y(), color().z(), 1.0f));
		PointShader::ptr()->setUniformValue("u_viewportSize", info->width, info->height);
		PointShader::ptr()->setUniformValue("pointSize", GLfloat(info->pointSize));

		glDrawArrays(GL_POINTS, 0, 1);

		glDisable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(0, 0);

		PointShader::ptr()->release();
		doAfterPaint(info);
	}

}