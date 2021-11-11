#include "PostProcPaintVisitor.h"
#include "shaders.h"
#include "PTriMesh.h"
#include "GLFunctions.h"
#include "PaintInformation.h"
#include "GLTexture.h"
namespace vrt{
	
}

mcl::ToneMapPaintVisitor::ToneMapPaintVisitor()
	:PostProcPaintVisitor()
{
}

int mcl::ToneMapPaintVisitor::paintTris(PaintInfomation* info, PTriMesh* tri)
{
	GLFUNC->glDisable(GL_DEPTH_TEST);
	ToneMapShader::ptr()->bind();
	info->setUniformValue(ToneMapShader::ptr(), TONE_MAP);
	tri->getVAO()->bind();
	ToneMapShader::ptr()->setUniformValue("size", QVector2D(info->width, info->height));
	GLFUNC->glDrawArrays(GL_TRIANGLES, 0, tri->getTriNumer() * 3);
	return 0;
}

int mcl::DirectLightPaintVisitor::paintTris(PaintInfomation* info, PTriMesh* tri)
{
	GLFUNC->glDisable(GL_DEPTH_TEST);
	DirectLightShader::ptr()->bind();
	info->setUniformValue(DirectLightShader::ptr(), DIRECT_LIGHT);
	tri->getVAO()->bind();
	GLFUNC->glDrawArrays(GL_TRIANGLES, 0, tri->getTriNumer() * 3);
	return 0;
}

int mcl::SsdoPaintVisitor::paintTris(PaintInfomation* info, PTriMesh* tri)
{
	GLFUNC->glDisable(GL_DEPTH_TEST);
	SsdoShader::ptr()->bind();
	info->setUniformValue(SsdoShader::ptr(), SSDO);
	tri->getVAO()->bind();
	GLFUNC->glDrawArrays(GL_TRIANGLES, 0, tri->getTriNumer() * 3);
	return 0;
}

int mcl::BloomFilterPaintVisitor::paintTris(PaintInfomation* info, PTriMesh* tri)
{
	GLFUNC->glDisable(GL_DEPTH_TEST);
	BloomFilterShader::ptr()->bind();
	info->setUniformValue(BloomFilterShader::ptr(), BLOOM);
	tri->getVAO()->bind();
	GLFUNC->glDrawArrays(GL_TRIANGLES, 0, tri->getTriNumer() * 3);
	return 0;
}

int mcl::FxaaPaintVisitor::paintTris(PaintInfomation* info, PTriMesh* tri)
{
	GLFUNC->glDisable(GL_DEPTH_TEST);
	FxaaShader::ptr()->bind();
	info->setUniformValue(FxaaShader::ptr(), FXAA);
	tri->getVAO()->bind();
	GLFUNC->glDrawArrays(GL_TRIANGLES, 0, tri->getTriNumer() * 3);
	return 0;
}

int mcl::SsrPaintVisitor::paintTris(PaintInfomation* info, PTriMesh* tri)
{
	GLFUNC->glDisable(GL_DEPTH_TEST);
	SsrShader::ptr()->bind();
	info->setUniformValue(SsrShader::ptr(), SSR);
	tri->getVAO()->bind();
	GLFUNC->glDrawArrays(GL_TRIANGLES, 0, tri->getTriNumer() * 3);
	return 0;
}

int mcl::DirectLightFilterPaintVisitor::paintTris(PaintInfomation* info, PTriMesh* tri)
{
	GLFUNC->glDisable(GL_DEPTH_TEST);
	DirectLightFilterShader::ptr()->bind();
	info->setUniformValue(DirectLightFilterShader::ptr(), DIRECT_LIGHT_FILTER);
	tri->getVAO()->bind();
	GLFUNC->glDrawArrays(GL_TRIANGLES, 0, tri->getTriNumer() * 3);
	return 0;
}

int mcl::SsdoFilterPaintVisitor::paintTris(PaintInfomation* info, PTriMesh* tri)
{
	GLFUNC->glDisable(GL_DEPTH_TEST);
	SsdoFilterShader::ptr()->bind();
	info->setUniformValue(SsdoFilterShader::ptr(), SSDO_FILTER);
	tri->getVAO()->bind();
	GLFUNC->glDrawArrays(GL_TRIANGLES, 0, tri->getTriNumer() * 3);
	return 0;
}

int mcl::SsrPaintFilterVisitor::paintTris(PaintInfomation* info, PTriMesh* tri)
{
	GLFUNC->glDisable(GL_DEPTH_TEST);
	SsrFilterShader::ptr()->bind();
	info->setUniformValue(SsrFilterShader::ptr(), SSR_FILTER);
	tri->getVAO()->bind();
	GLFUNC->glDrawArrays(GL_TRIANGLES, 0, tri->getTriNumer() * 3);
	return 0;
}
