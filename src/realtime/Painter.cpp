#include "Painter.h"
#include "GLFrameBufferObject.h"
#include "PTriMesh.h"
#include "PaintInformation.h"
#include "GLTexture.h"
#include "GLFunctions.h"
#include "Light.h"
#include "shaders.h"
namespace mcl {

	MipmapBufferPainter::MipmapBufferPainter(int maxLevels, int stopSize /*= 1*/, GLTextureFormat format)
		:stopSize(stopSize)
	{
		for (int i = 0; i < maxLevels; i++) {
			fbos.push_back(std::make_shared<GLColorFrameBufferObject>());
			fbos.back()->castedTexture<GLTexture2D>()->setFormat(format);
		}
	}

	void MipmapBufferPainter::resize(int w, int h)
	{
		int i = 0;
		for (i = 0; i < fbos.size(); i++) {
			w = (w + 1) / 2;
			h = (h + 1) / 2;
			if (w <= stopSize && h <= stopSize) {
				if (w == stopSize || h == stopSize) {
					fbos[i]->resize(h, w);
					i++;
				}	
				break;
			}
			fbos[i]->resize(h, w);
		}
		mipLevel = i;
	}

	SingleBufferPainter::SingleBufferPainter(std::shared_ptr<AbstractGLFrameBufferObject> fbo)
		:fbo(fbo)
	{
	}

	void SingleBufferPainter::resize(int w, int h)
	{
		fbo->resize(h, w);
	}

	void DirectLightPainter::init(PaintInfomation* info)
	{
		info->directLightTexture = fbo->texture();
	}

	void DirectLightFilterPainter::init(PaintInfomation* info)
	{
		info->directLightFilterTexture = fbo->texture();
	}

	void SsrPainter::init(PaintInfomation* info)
	{
		info->ssrTexture = fbo->texture();
	}

	void SsrFilterPainter::init(PaintInfomation* info)
	{
		info->ssrFilterTexture = fbo->texture();
		info->finalHdrTexture = fbo->texture();
	}

	void SsdoPainter::init(PaintInfomation* info)
	{
		info->ssdoTexture = fbo->texture();
	}

	void SsdoFilterPainter::init(PaintInfomation* info)
	{
		info->ssdoFilterTexture = fbo->texture();
	}


	void SsdoFilterPainter::setTexture(std::shared_ptr<GLTexture2D> tex)
	{
		tex->setFilter(GL_LINEAR, GL_LINEAR);
		tex->setWrap(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	}

	void ToneMapPainter::init(PaintInfomation* info)
	{
		info->finalLdrTexture = fbo->texture();
	}

	void ToneMapPainter::setTexture(std::shared_ptr<GLTexture2D> tex)
	{
		tex->setFilter(GL_LINEAR, GL_LINEAR);
		tex->setWrap(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	}

	void FxaaPainter::init(PaintInfomation* info)
	{
	}

	DeferredMtrPainter::DeferredMtrPainter()
		:SingleBufferPainter(std::make_shared<GLMtrFrameBufferObject>(1))
	{
	}

	void DeferredMtrPainter::paint(PaintInfomation* info)
	{
		GLFUNC->glViewport(0, 0, info->width, info->height);
		fbo->bind();
		fbo->clear(info);
		for (const auto& primpair : *info->prims) {
			primpair.second->paint(info, Singleton<DeferredMtrPaintVisitor>::getSingleton());
		}
	}

	void DeferredMtrPainter::init(PaintInfomation* info)
	{
		info->mtrTex = std::dynamic_pointer_cast<GLMtrFrameBufferObject>(fbo)->transferedTextureId();
	}

	void BloomFilterPainter::paint(PaintInfomation* info)
	{
		//Bloom down sample
		for (int i = 0; i < mipLevel; i++) {
			GLFUNC->glViewport(0, 0, fbos[i]->texture()->size().x(), fbos[i]->texture()->size().y());
			fbos[i]->bind();
			fbos[i]->clear(info);
			info->bloomSampleState = i + 1;
			PTriMesh::getBillBoard()->paint(info, Singleton<BloomFilterPaintVisitor>::getSingleton());
		}
		//Bloom up sample
		for (int i = mipLevel - 1; i >= 1; i--) {
			GLFUNC->glViewport(0, 0, fbos[i - 1]->texture()->size().x(), fbos[i - 1]->texture()->size().y());
			fbos[i - 1]->bind();
			info->bloomSampleState = -i - 1;
			PTriMesh::getBillBoard()->paint(info, Singleton<BloomFilterPaintVisitor>::getSingleton());
		}
	}

	void BloomFilterPainter::init(PaintInfomation* info)
	{
		for (const auto& mip : fbos) {
			info->bloomMipTex.push_back(mip->texture());
		}
	}

	void LightsShadowMapPainter::paint(PaintInfomation* info)
	{
		for (int i = 0; i < info->lights.size(); i++) {
			std::shared_ptr<PointLight> ptlight = std::dynamic_pointer_cast<PointLight>(info->lights[i]);
			if (ptlight) {
				std::shared_ptr<CubeShadowMapPaintVisitor> smpainter = std::make_shared<CubeShadowMapPaintVisitor>(ptlight);
				ptlight->initFbo();
				ptlight->getFbo()->bind();
				ptlight->getFbo()->clear(info);
				GLFUNC->glViewport(0, 0, ptlight->shadowMapSize().x(), ptlight->shadowMapSize().y());
				for (auto& prim : *info->prims) {
					if (prim.first == ptlight->primitive()->id())
						continue;
					prim.second->paint(info, smpainter.get());
				}
			}
		}
	}

	HiZFilterPainter::HiZFilterPainter(int maxLevels)
		:MipmapBufferPainter(maxLevels, 1, GLTextureFormat(GL_RG32F, GL_RG, 
			GL_FLOAT, GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE))
	{
	}

	void HiZFilterPainter::paint(PaintInfomation* info)
	{
		Vector2i lastSize(info->width, info->height), curSize;
		curSize = (lastSize + 1) / 2;
		HiZFilterShader::ptr()->bind();
		for (int i = 0; i < mipLevel; i++) {
			GLFUNC->glViewport(0, 0, curSize.x(), curSize.y());
			fbos[i]->bind();
			fbos[i]->clear(info);
			if (i == 0)
				info->mtrTex[GLMtrFrameBufferObject::DEPTH]->bindToUniform("gPrevImage", HiZFilterShader::ptr());
			else
				fbos[i-1]->texture()->bindToUniform("gPrevImage", HiZFilterShader::ptr());
			HiZFilterShader::ptr()->setUniformValue("preMipSize", lastSize.x(), lastSize.y());
			HiZFilterShader::ptr()->setUniformValue("curMipSize",  curSize.x(), curSize.y());
			PTriMesh::getBillBoard()->paint(info, Singleton<HiZFilterPaintVisitor>::getSingleton());
			lastSize = curSize;
			curSize = (lastSize + 1) / 2;
		}
		info->hiZMipCnt = mipLevel;
	}

	void HiZFilterPainter::init(PaintInfomation* info)
	{
		for (const auto& mip : fbos) {
			info->hiZMipTex.push_back(mip->texture());
		}
	}

	void HiZSsrPainter::init(PaintInfomation* info)
	{
		info->ssrTexture = fbo->texture();
	}

}



