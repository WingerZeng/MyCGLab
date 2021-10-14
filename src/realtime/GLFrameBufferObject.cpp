#include "GLFrameBufferObject.h"
#include "GLFunctions.h"
#include "Scene.h"
#include "GLTexture.h"
namespace mcl {
	
	GLColorFrameBufferObject::GLColorFrameBufferObject()
	{
		GLFUNC->glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		tex = std::make_shared<GLTexture2D>(GL_RGB16F, GL_RGB, GL_FLOAT);
		tex->bind();
		tex->setFilter(GL_LINEAR, GL_LINEAR);
		tex->setWrap(GL_CLAMP_TO_EDGE,GL_CLAMP_TO_EDGE);
		tex->release();

		GLFUNC->glGenRenderbuffers(1, &rbo);
		GLFUNC->glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		GLFUNC->glBindRenderbuffer(GL_RENDERBUFFER, 0);

		GLFUNC->glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	GLColorFrameBufferObject::~GLColorFrameBufferObject()
	{	
		GLFUNC->glBindRenderbuffer(GL_RENDERBUFFER, 0);
		GLFUNC->glDeleteRenderbuffers(1, &rbo);
	}

	void GLColorFrameBufferObject::bind()
	{
		GLFUNC->glDisable(GL_DEPTH_TEST);
		GLFUNC->glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		if (GLFUNC->glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			LOG(FATAL) << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!";
	}

	void GLColorFrameBufferObject::resize(int height, int width)
	{
		GLFrameBufferObject::resize(height, width);
		GLFUNC->glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		tex->resize(width, height);
		tex->bindToFbo(fbo, GL_COLOR_ATTACHMENT0);
		tex->release();

		GLFUNC->glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		GLFUNC->glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH32F_STENCIL8, width, height);

		GLFUNC->glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
		GLFUNC->glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}

	std::shared_ptr<mcl::GLAbstractTexture> GLColorFrameBufferObject::texture(int idx /*= 0*/)
	{
		return tex;
	}

	GLMultiSampleFrameBufferObject::GLMultiSampleFrameBufferObject(int nsample)
		:nsample(nsample)
	{
		GLFUNC->glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		mstex = std::make_shared<GLTextureMultiSample>(GL_RGB16F, nsample);

		GLFUNC->glGenRenderbuffers(1, &msrbo);
		GLFUNC->glBindRenderbuffer(GL_RENDERBUFFER, msrbo);
		GLFUNC->glBindRenderbuffer(GL_RENDERBUFFER, 0);

		GLFUNC->glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	GLMultiSampleFrameBufferObject::~GLMultiSampleFrameBufferObject()
	{
		GLFUNC->glBindRenderbuffer(GL_RENDERBUFFER, 0);
		GLFUNC->glDeleteRenderbuffers(1, &msrbo);
	}

	void GLMultiSampleFrameBufferObject::bind()
	{
		GLFUNC->glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		if (GLFUNC->glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			LOG(FATAL) << GLFUNC->glCheckFramebufferStatus(GL_FRAMEBUFFER);
	}

	void GLMultiSampleFrameBufferObject::resize(int height, int width)
	{
		GLFrameBufferObject::resize(height, width);
		GLFUNC->glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		mstex->resize(width, height);
		mstex->release();
		mstex->bindToFbo(fbo, GL_COLOR_ATTACHMENT0);

		GLFUNC->glBindRenderbuffer(GL_RENDERBUFFER, msrbo);
		GLFUNC->glRenderbufferStorageMultisample(GL_RENDERBUFFER, nsample, GL_DEPTH32F_STENCIL8, width, height);
		GLFUNC->glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, msrbo);
		GLFUNC->glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}

	std::shared_ptr<mcl::GLAbstractTexture> GLMultiSampleFrameBufferObject::texture(int idx /*= 0*/)
	{
		return mstex;
	}

	void GLMultiSampleFrameBufferObject::copyToFbo(GLuint afbo)
	{
		GLFUNC->glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
		GLFUNC->glBindFramebuffer(GL_DRAW_FRAMEBUFFER, afbo);
		GLFUNC->glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		GLFUNC->glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
		GLFUNC->glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	}

	GLFrameBufferObject::GLFrameBufferObject()
	{
		GLFUNC->glGenFramebuffers(1, &fbo);
	}

	GLFrameBufferObject::~GLFrameBufferObject()
	{
		//½â°ó
		GLFUNC->glBindFramebuffer(GL_FRAMEBUFFER, 0);
		GLFUNC->glDeleteFramebuffers(1, &fbo);
	}

	void GLColorFrameBufferObject::clear(PaintInfomation* info)
	{
		GLFUNC->glClearColor(info->clearColor.x(), info->clearColor.y(), info->clearColor.z(), 1);
		GLFUNC->glClear(GL_COLOR_BUFFER_BIT);
	}

	void GLFrameBufferObject::resize(int height, int width)
	{
		h = height;
		w = width;
	}

	GLShadowMapFrameBufferObject::GLShadowMapFrameBufferObject()
		:GLFrameBufferObject()
	{
		GLFUNC->glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		tex = std::make_shared<GLTextureCubeMap>(GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT);
		tex->setFilter(GL_LINEAR, GL_LINEAR);
		tex->setWrap(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

		//colortex = std::make_shared<GLTextureCubeMap>(GL_RGBA8, GL_RGB, GL_UNSIGNED_BYTE);

		GLFUNC->glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	GLShadowMapFrameBufferObject::~GLShadowMapFrameBufferObject()
	{
	}

	void GLShadowMapFrameBufferObject::bind()
	{
		GLFUNC->glEnable(GL_DEPTH_TEST);
		GLFUNC->glDrawBuffer(GL_NONE);
		GLFUNC->glReadBuffer(GL_NONE);
		GLFUNC->glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		GLFUNC->glDrawBuffer(GL_NONE);
		GLFUNC->glReadBuffer(GL_NONE);
	}

	void GLShadowMapFrameBufferObject::clear(PaintInfomation* info)
	{
		GLFUNC->glClear(GL_DEPTH_BUFFER_BIT);
	}

	void GLShadowMapFrameBufferObject::resize(int height, int width)
	{
		GLFrameBufferObject::resize(height, width);
		GLFUNC->glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		
		tex->resize(width, height);
		tex->bindToFbo(fbo, GL_DEPTH_ATTACHMENT);

		GLFUNC->glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	std::shared_ptr<mcl::GLAbstractTexture> GLShadowMapFrameBufferObject::texture(int idx /*= 0*/)
	{
		return tex;
	}

	GLMtrFrameBufferObject::GLMtrFrameBufferObject(int nsample)
		:nsample(nsample)
	{
		if (nsample > 1)
			GLFUNC->glGenFramebuffers(1, &interFbo);
		GLFUNC->glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		for (int i = 0; i < nTargetType; i++) {
			if(nsample > 1)
				targetTex.emplace_back(std::make_shared<GLTextureMultiSample>(targetFromats[i], nsample));
			outputTex.emplace_back(std::make_shared<GLTexture2D>(targetFromats[i], targetBaseFromats[i], targetUnitFromats[i]));
			outputTex[i]->setFilter(GL_NEAREST, GL_NEAREST);
			outputTex[i]->setWrap(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
		}

		GLFUNC->glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	GLMtrFrameBufferObject::~GLMtrFrameBufferObject()
	{
		if (nsample > 1)
			GLFUNC->glDeleteFramebuffers(1, &interFbo);
	}

	void GLMtrFrameBufferObject::bind()
	{
		GLFUNC->glEnable(GL_DEPTH_TEST);
		GLFUNC->glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		GLuint drawbuffers[nTargetType-1];
		for (int i = 0; i < nTargetType-1; i++) {
			drawbuffers[i] = GL_COLOR_ATTACHMENT0 + i;
		}
		GLFUNC->glDrawBuffers(nTargetType - 1, drawbuffers);
		if (GLFUNC->glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			LOG(FATAL) << GLFUNC->glCheckFramebufferStatus(GL_FRAMEBUFFER);
	}

	void GLMtrFrameBufferObject::clear(PaintInfomation* info)
	{
		GLFUNC->glClearColor(0, 0, 0, 1);
		GLenum buffers[3] = { GL_COLOR_ATTACHMENT0 + ALBEDO, GL_COLOR_ATTACHMENT0 + WORLD_POS, GL_COLOR_ATTACHMENT0 + NORMAL };
		GLFUNC->glDrawBuffers(3, buffers);
		GLFUNC->glClear(GL_COLOR_BUFFER_BIT);

		GLFUNC->glClearColor(1, 1, 1, 1);
		GLFUNC->glClear(GL_DEPTH_BUFFER_BIT);

		GLFUNC->glClearColor(info->emptyPrimIdColor.x(), info->emptyPrimIdColor.y(), info->emptyPrimIdColor.z(), 0);
		GLFUNC->glDrawBuffer(GL_COLOR_ATTACHMENT0 + PRIMID);
		GLFUNC->glClear(GL_COLOR_BUFFER_BIT);

		GLuint drawbuffers[nTargetType - 1];
		for (int i = 0; i < nTargetType - 1; i++) {
			drawbuffers[i] = GL_COLOR_ATTACHMENT0 + i;
		}
		GLFUNC->glDrawBuffers(nTargetType - 1, drawbuffers);
	}

	void GLMtrFrameBufferObject::resize(int height, int width)
	{
		GLFrameBufferObject::resize(height, width);
		GLFUNC->glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		if (nsample > 1) {
			for (int i = 0; i < nTargetType - 1; i++) {
				targetTex[i]->resize(width, height);
				targetTex[i]->bindToFbo(fbo, GL_COLOR_ATTACHMENT0 + i);
			}
			targetTex[DEPTH]->resize(width, height);
			targetTex[DEPTH]->bindToFbo(fbo, GL_DEPTH_ATTACHMENT);

			GLFUNC->glBindFramebuffer(GL_FRAMEBUFFER, interFbo);
		}
		GLuint outputfbo = nsample == 1 ? fbo : interFbo;
		for (int i = 0; i < nTargetType - 1; i++) {
			outputTex[i]->resize(width, height);
			outputTex[i]->bindToFbo(outputfbo, GL_COLOR_ATTACHMENT0 + i);
		}
		outputTex[DEPTH]->resize(width, height);
		outputTex[DEPTH]->bindToFbo(outputfbo, GL_DEPTH_ATTACHMENT);

		GLFUNC->glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	std::shared_ptr<mcl::GLAbstractTexture> GLMtrFrameBufferObject::texture(int idx /*= 0*/)
	{
		if (nsample == 1)
			return outputTex[idx];
		return targetTex[idx];
	}

	std::vector<std::shared_ptr<GLAbstractTexture>> GLMtrFrameBufferObject::transferedTextureId()
	{
		if (nsample == 1)
			return std::vector<std::shared_ptr<GLAbstractTexture>>(outputTex.begin(), outputTex.end());
		GLFUNC->glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
		GLFUNC->glBindFramebuffer(GL_DRAW_FRAMEBUFFER, interFbo);

		for (int i = 0; i < nTargetType - 1; i++) {
			GLFUNC->glReadBuffer(GL_COLOR_ATTACHMENT0 + i);
			GLFUNC->glDrawBuffer(GL_COLOR_ATTACHMENT0 + i);
			GLFUNC->glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		}

		GLFUNC->glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
		GLFUNC->glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
		GLFUNC->glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

		std::vector<std::shared_ptr<GLAbstractTexture>> ret(outputTex.begin(), outputTex.end());
		return ret;
	}

	std::array<GLuint, GLMtrFrameBufferObject::nTargetType> GLMtrFrameBufferObject::targetFromats = {
		GL_RGB16F, //COLOR
		GL_RGB16F, //ALBEDO
		GL_RGB32F,//WORLD POS
		GL_RGB32F,//NORMAL
		GL_RGB8, //PRIMID
		GL_DEPTH_COMPONENT, //DEPTH
	};

	std::array<GLuint, mcl::GLMtrFrameBufferObject::nTargetType> GLMtrFrameBufferObject::targetBaseFromats = {
		GL_RGB, //COLOR
		GL_RGB, //ALBEDO
		GL_RGB,//WORLD POS
		GL_RGB,//NORMAL
		GL_RGB, //PRIMID
		GL_DEPTH_COMPONENT, //DEPTH
	};

	std::array<GLuint, mcl::GLMtrFrameBufferObject::nTargetType> GLMtrFrameBufferObject::targetUnitFromats = {
		GL_FLOAT, //COLOR
		GL_FLOAT, //ALBEDO
		GL_FLOAT,//WORLD POS
		GL_FLOAT,//NORMAL
		GL_UNSIGNED_BYTE, //PRIMID
		GL_FLOAT, //DEPTH
	};

}