#include "PostProcPaintVisitor.h"
#include <QOpenGLShaderProgram>
#include "shaders.h"
#include "PTriMesh.h"
#include "GLFunctions.h"
#include "PaintInformation.h"
#include "GLTexture.h"
#include "Light.h"
#include "GLFrameBufferObject.h"

void mcl::ToneMapPaintVisitor::setUniform(PaintInfomation* info)
{
	info->mtrTex[GLMtrFrameBufferObject::PRIMID]->bindToUniform("primId", getDefaultShader());
	info->finalHdrTexture->bindToUniform("finalHdrTex", getDefaultShader());
	info->bloomMipTex[0]->bindToUniform("bloomMip0", getDefaultShader());
	getDefaultShader()->setUniformValue("clearColor", QVector3D(info->clearColor));
	getDefaultShader()->setUniformValue("size", QVector2D(info->width, info->height));
}

QOpenGLShaderProgram* mcl::ToneMapPaintVisitor::getDefaultShader()
{
	return ToneMapShader::ptr();
}

void mcl::DirectLightPaintVisitor::setUniform(PaintInfomation* info)
{
	getDefaultShader()->setUniformValue("world2Screen", info->projMat * info->viewMat);
	getDefaultShader()->setUniformValue("lightCount", GLint(info->lights.size()));
	getDefaultShader()->setUniformValue("sceneExtent", (info->sceneBnd.pMax() - info->sceneBnd.pMin()).length());
	info->mtrTex[GLMtrFrameBufferObject::ALBEDO]->bindToUniform("albedo", getDefaultShader());
	info->mtrTex[GLMtrFrameBufferObject::WORLD_POS]->bindToUniform("worldPos", getDefaultShader());
	info->mtrTex[GLMtrFrameBufferObject::NORMAL]->bindToUniform("normal", getDefaultShader());
	info->mtrTex[GLMtrFrameBufferObject::DEPTH]->bindToUniform("depth", getDefaultShader());
	for (int j = 0; j < info->lights.size(); j++) {
		info->lights[j]->bind(getDefaultShader(), j, info->activeTextrueCnt);
	}
}

QOpenGLShaderProgram* mcl::DirectLightPaintVisitor::getDefaultShader()
{
	return DirectLightShader::ptr();
}

void mcl::SsdoPaintVisitor::setUniform(PaintInfomation* info)
{
	getDefaultShader()->setUniformValue("world2Screen", info->projMat * info->viewMat);
	getDefaultShader()->setUniformValue("lightCount", GLint(info->lights.size()));
	getDefaultShader()->setUniformValue("sceneExtent", (info->sceneBnd.pMax() - info->sceneBnd.pMin()).length());
	info->mtrTex[GLMtrFrameBufferObject::ALBEDO]->bindToUniform("albedo", getDefaultShader());
	info->mtrTex[GLMtrFrameBufferObject::WORLD_POS]->bindToUniform("worldPos", getDefaultShader());
	info->mtrTex[GLMtrFrameBufferObject::NORMAL]->bindToUniform("normal", getDefaultShader());
	info->mtrTex[GLMtrFrameBufferObject::DEPTH]->bindToUniform("depth", getDefaultShader());
	info->directLightFilterTexture->bindToUniform("directLightTexture", getDefaultShader());
	for (int j = 0; j < info->lights.size(); j++) {
		info->lights[j]->bind(getDefaultShader(), j, info->activeTextrueCnt);
	}
}

QOpenGLShaderProgram* mcl::SsdoPaintVisitor::getDefaultShader()
{
	return SsdoShader::ptr();
}

void mcl::BloomFilterPaintVisitor::setUniform(PaintInfomation* info)
{
	getDefaultShader()->setUniformValue("isDownSample", info->bloomSampleState > 0);
	if (info->bloomSampleState > 0) {
		if (info->bloomSampleState == 1) {
			info->finalHdrTexture->bindToUniform("prevTex", getDefaultShader());
			getDefaultShader()->setUniformValue("firstDownSample", true);
		}
		else {
			info->bloomMipTex[info->bloomSampleState - 2]->bindToUniform("prevTex", getDefaultShader());
			getDefaultShader()->setUniformValue("firstDownSample", false);
		}
		getDefaultShader()->setUniformValue("size", QVector2D(info->bloomMipTex[info->bloomSampleState - 1]->size().x(), info->bloomMipTex[info->bloomSampleState - 1]->size().y()));
	}
	if (info->bloomSampleState < 0) {
		info->bloomMipTex[-info->bloomSampleState - 1]->bindToUniform("prevTex", getDefaultShader());
		info->bloomMipTex[-info->bloomSampleState - 2]->bindToUniform("curTex", getDefaultShader());
		getDefaultShader()->setUniformValue("firstDownSample", false);
		getDefaultShader()->setUniformValue("size", QVector2D(info->bloomMipTex[-info->bloomSampleState - 2]->size().x(), info->bloomMipTex[-info->bloomSampleState - 2]->size().y()));
	}
}

QOpenGLShaderProgram* mcl::BloomFilterPaintVisitor::getDefaultShader()
{
	return BloomFilterShader::ptr();
}

void mcl::FxaaPaintVisitor::setUniform(PaintInfomation* info)
{
	info->finalLdrTexture->bindToUniform("ldrTexture", getDefaultShader());
	getDefaultShader()->setUniformValue("viewportSize", info->width, info->height);
}

QOpenGLShaderProgram* mcl::FxaaPaintVisitor::getDefaultShader()
{
	return FxaaShader::ptr();
}

void mcl::SsrPaintVisitor::setUniform(PaintInfomation* info)
{
	info->ssdoFilterTexture->bindToUniform("gFinalImage", getDefaultShader());
	info->mtrTex[GLMtrFrameBufferObject::ALBEDO]->bindToUniform("gAlbedo", getDefaultShader());
	info->mtrTex[GLMtrFrameBufferObject::SPECULAR]->bindToUniform("gSpecular", getDefaultShader());
	info->mtrTex[GLMtrFrameBufferObject::NORMAL]->bindToUniform("gWorldNormal", getDefaultShader());
	info->mtrTex[GLMtrFrameBufferObject::WORLD_POS]->bindToUniform("gWorldPos", getDefaultShader());
	info->mtrTex[GLMtrFrameBufferObject::DEPTH]->bindToUniform("gDepth", getDefaultShader());
	getDefaultShader()->setUniformValue("viewportSize", info->width, info->height);
	getDefaultShader()->setUniformValue("sceneExtent", (info->sceneBnd.pMax() - info->sceneBnd.pMin()).length());
	getDefaultShader()->setUniformValue("view", info->viewMat);
	getDefaultShader()->setUniformValue("invView", info->viewMat.inverted());
	getDefaultShader()->setUniformValue("project", info->projMat);
	getDefaultShader()->setUniformValue("invProject", info->projMat.inverted());
	getDefaultShader()->setUniformValue("bgColor", QVector3D(info->clearColor));
}

QOpenGLShaderProgram* mcl::SsrPaintVisitor::getDefaultShader()
{
	return SsrShader::ptr();
}

void mcl::DirectLightFilterPaintVisitor::setUniform(PaintInfomation* info)
{
	info->directLightTexture->bindToUniform("gLightImage", getDefaultShader());
	info->mtrTex[GLMtrFrameBufferObject::ALBEDO]->bindToUniform("gAlbedo", getDefaultShader());
	info->mtrTex[GLMtrFrameBufferObject::PRIMID]->bindToUniform("gPrimId", getDefaultShader());
}

QOpenGLShaderProgram* mcl::DirectLightFilterPaintVisitor::getDefaultShader()
{
	return DirectLightFilterShader::ptr();
}

void mcl::SsdoFilterPaintVisitor::setUniform(PaintInfomation* info)
{
	info->mtrTex[GLMtrFrameBufferObject::COLOR]->bindToUniform("gRasterColor", getDefaultShader());
	info->mtrTex[GLMtrFrameBufferObject::PRIMID]->bindToUniform("gPrimId", getDefaultShader());
	info->ssdoTexture->bindToUniform("gSsdoTexture", getDefaultShader());
	info->directLightFilterTexture->bindToUniform("gFinalImage", getDefaultShader());
}

QOpenGLShaderProgram* mcl::SsdoFilterPaintVisitor::getDefaultShader()
{
	return SsdoFilterShader::ptr();
}

void mcl::SsrFilterPaintVisitor::setUniform(PaintInfomation* info)
{
	info->ssdoFilterTexture->bindToUniform("gDirectLightImage", getDefaultShader());
	info->ssrTexture->bindToUniform("gSsrImage", getDefaultShader());
	info->mtrTex[GLMtrFrameBufferObject::PRIMID]->bindToUniform("gPrimId", getDefaultShader());
	info->mtrTex[GLMtrFrameBufferObject::NORMAL]->bindToUniform("gWorldNormal", getDefaultShader());
}

QOpenGLShaderProgram* mcl::SsrFilterPaintVisitor::getDefaultShader()
{
	return SsrFilterShader::ptr();
}

int mcl::PostProcPaintVisitor::paintTris(PaintInfomation* info, PTriMesh* tri)
{
	GLFUNC->glDisable(GL_DEPTH_TEST);
	getDefaultShader()->bind();
	setUniform(info);
	tri->getVAO()->bind();
	GLFUNC->glDrawArrays(GL_TRIANGLES, 0, tri->getTriNumer() * 3);
	return 0;
}

void mcl::HiZFilterPaintVisitor::setUniform(PaintInfomation* info)
{
}

QOpenGLShaderProgram* mcl::HiZFilterPaintVisitor::getDefaultShader()
{
	return HiZFilterShader::ptr();
}

void mcl::HiZSsrPaintVisitor::setUniform(PaintInfomation* info)
{
	info->ssdoFilterTexture->bindToUniform("gFinalImage", getDefaultShader());
	info->mtrTex[GLMtrFrameBufferObject::ALBEDO]->bindToUniform("gAlbedo", getDefaultShader());
	info->mtrTex[GLMtrFrameBufferObject::SPECULAR]->bindToUniform("gSpecular", getDefaultShader());
	info->mtrTex[GLMtrFrameBufferObject::NORMAL]->bindToUniform("gWorldNormal", getDefaultShader());
	info->mtrTex[GLMtrFrameBufferObject::WORLD_POS]->bindToUniform("gWorldPos", getDefaultShader());
	info->mtrTex[GLMtrFrameBufferObject::DEPTH]->bindToUniform("gHiZMips[0]", getDefaultShader());
	// HiZ uniforms
	getDefaultShader()->setUniformValue("MipSizes[0]", info->width, info->height);
	for (int i = 0; i < info->hiZMipCnt; i++) {
		info->hiZMipTex[i]->bindToUniform(("gHiZMips[" + std::to_string(i + 1) + "]"), getDefaultShader());
		getDefaultShader()->setUniformValue(("MipSizes[" + std::to_string(i + 1) + "]").c_str(), info->hiZMipTex[i]->size().x(), info->hiZMipTex[i]->size().y());
	}
	getDefaultShader()->setUniformValue("HiZLevelCnt", info->hiZMipCnt+1);
	getDefaultShader()->setUniformValue("viewportSize", info->width, info->height);
	getDefaultShader()->setUniformValue("sceneExtent", (info->sceneBnd.pMax() - info->sceneBnd.pMin()).length());
	getDefaultShader()->setUniformValue("view", info->viewMat);
	getDefaultShader()->setUniformValue("invView", info->viewMat.inverted());
	getDefaultShader()->setUniformValue("project", info->projMat);
	getDefaultShader()->setUniformValue("invProject", info->projMat.inverted());
	getDefaultShader()->setUniformValue("bgColor", QVector3D(info->clearColor));
}

QOpenGLShaderProgram* mcl::HiZSsrPaintVisitor::getDefaultShader()
{
	return HiZSSRShader::ptr();
}
