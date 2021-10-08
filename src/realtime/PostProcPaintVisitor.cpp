#include "PostProcPaintVisitor.h"
#include "shaders.h"
#include "PTriMesh.h"
#include "GLFunctions.h"
#include "PaintInformation.h"
namespace vrt{
	
}

mcl::ToneMapPaintVisitor::ToneMapPaintVisitor()
	:PostProcPaintVisitor()
{
}

int mcl::ToneMapPaintVisitor::paintTris(PaintInfomation* info, PTriMesh* tri)
{
	GLFUNC->glDisable(GL_DEPTH_TEST);
	GLFUNC->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	ToneMapShader::ptr()->bind();
	tri->getVAO()->bind();
	GLFUNC->glDrawArrays(GL_TRIANGLES, 0, tri->getTriNumer() * 3);
	return 0;
}

int mcl::DeferredDirectLightPaintVisitor::paintTris(PaintInfomation* info, PTriMesh* tri)
{
	GLFUNC->glDisable(GL_DEPTH_TEST);
	GLFUNC->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	DeferredDirectLightShader::ptr()->bind();
	GLFUNC->resetActiveTexture();
	info->setUniformValue(DeferredDirectLightShader::ptr(), DEFFER_LIGHTING);
	tri->getVAO()->bind();
	GLFUNC->glDrawArrays(GL_TRIANGLES, 0, tri->getTriNumer() * 3);
	return 0;
}

int mcl::DeferredSsdoPaintVisitor::paintTris(PaintInfomation* info, PTriMesh* tri)
{
	GLFUNC->glDisable(GL_DEPTH_TEST);
	GLFUNC->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	DeferredSsdoShader::ptr()->bind();
	GLFUNC->resetActiveTexture();
	info->setUniformValue(DeferredSsdoShader::ptr(), DEFFER_LIGHTING);
	tri->getVAO()->bind();
	GLFUNC->glDrawArrays(GL_TRIANGLES, 0, tri->getTriNumer() * 3);
	return 0;
}
