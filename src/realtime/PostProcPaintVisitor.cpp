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

int mcl::DeferredDirectLightPaintVisitor::paintTris(PaintInfomation* info, PTriMesh* tri)
{
	GLFUNC->glDisable(GL_DEPTH_TEST);
	DeferredDirectLightShader::ptr()->bind();
	info->setUniformValue(DeferredDirectLightShader::ptr(), DEFFER_DIRECT_LIGHT);
	tri->getVAO()->bind();
	GLFUNC->glDrawArrays(GL_TRIANGLES, 0, tri->getTriNumer() * 3);
	return 0;
}

int mcl::DeferredSsdoPaintVisitor::paintTris(PaintInfomation* info, PTriMesh* tri)
{
	GLFUNC->glDisable(GL_DEPTH_TEST);
	DeferredSsdoShader::ptr()->bind();
	info->setUniformValue(DeferredSsdoShader::ptr(), DEFFER_SSDO);
	tri->getVAO()->bind();
	GLFUNC->glDrawArrays(GL_TRIANGLES, 0, tri->getTriNumer() * 3);
	return 0;
}

int mcl::BloomFilterPaintVisitor::paintTris(PaintInfomation* info, PTriMesh* tri)
{
	GLFUNC->glDisable(GL_DEPTH_TEST);
	BloomFilterShader::ptr()->bind();
	BloomFilterShader::ptr()->setUniformValue("isDownSample", info->bloomSampleState > 0);
	if (info->bloomSampleState > 0) {
		if (info->bloomSampleState == 1) {
			info->finalHdrTexture->bindToUniform("prevTex", BloomFilterShader::ptr());
			BloomFilterShader::ptr()->setUniformValue("firstDownSample", true);
		}
		else {
			info->bloomMipTex[info->bloomSampleState - 2]->bindToUniform("prevTex", BloomFilterShader::ptr());
			BloomFilterShader::ptr()->setUniformValue("firstDownSample", false);
		}
		BloomFilterShader::ptr()->setUniformValue("size", QVector2D(info->bloomMipTex[info->bloomSampleState - 1]->size().x(), info->bloomMipTex[info->bloomSampleState - 1]->size().y()));
	}
	if (info->bloomSampleState < 0) {
		info->bloomMipTex[-info->bloomSampleState - 1]->bindToUniform("prevTex", BloomFilterShader::ptr());
		info->bloomMipTex[-info->bloomSampleState - 2]->bindToUniform("curTex", BloomFilterShader::ptr());
		BloomFilterShader::ptr()->setUniformValue("firstDownSample", false);
		BloomFilterShader::ptr()->setUniformValue("size", QVector2D(info->bloomMipTex[-info->bloomSampleState - 2]->size().x(), info->bloomMipTex[-info->bloomSampleState - 2]->size().y()));
	}
	tri->getVAO()->bind();
	GLFUNC->glDrawArrays(GL_TRIANGLES, 0, tri->getTriNumer() * 3);
	return 0;
}

int mcl::DeferredCompositePaintVisitor::paintTris(PaintInfomation* info, PTriMesh* tri)
{
	GLFUNC->glDisable(GL_DEPTH_TEST);
	DeferredCompositeShader::ptr()->bind();
	info->setUniformValue(DeferredCompositeShader::ptr(), DEFFER_COMPOSITE);
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
