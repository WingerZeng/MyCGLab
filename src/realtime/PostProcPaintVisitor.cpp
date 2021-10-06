#include "PostProcPaintVisitor.h"
#include "shaders.h"
#include "PTriMesh.h"
#include "GLFunctions.h"
#include "PaintInformation.h"
namespace vrt{
	
}

mcl::GammaPaintVisitor::GammaPaintVisitor(GLfloat gamma /*= 2.2*/)
	:PostProcPaintVisitor(),gamma(gamma)
{
}

int mcl::GammaPaintVisitor::paintTris(PaintInfomation* info, PTriMesh* tri)
{
	GLFUNC->glDisable(GL_DEPTH_TEST);
	GLFUNC->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	GammaCorrectShader::ptr()->bind();
	GammaCorrectShader::ptr()->setUniformValue("gamma", gamma);
	tri->getVAO()->bind();
	GLFUNC->glDrawArrays(GL_TRIANGLES, 0, tri->getTriNumer() * 3);
	return 0;
}

int mcl::GammaPaintVisitor::initTris(PTriMesh* tri)
{
	return 0;
}

int mcl::DeferedPaintVisitor::paintTris(PaintInfomation* info, PTriMesh* tri)
{
	GLFUNC->glDisable(GL_DEPTH_TEST);
	GLFUNC->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	DeferedShader::ptr()->bind();
	GLFUNC->resetActiveTexture();
	info->setUniformValue(DeferedShader::ptr(), DEFFER_LIGHTING);
	tri->getVAO()->bind();
	GLFUNC->glDrawArrays(GL_TRIANGLES, 0, tri->getTriNumer() * 3);
	return 0;
}

int mcl::DeferedPaintVisitor::initTris(PTriMesh* tri)
{
	return 0;
}
