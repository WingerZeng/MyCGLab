#include "GLFrameBufferObject.h"
#include "GLFunctions.h"
#include "Scene.h"
namespace mcl {
	
	GLColorFrameBufferObject::GLColorFrameBufferObject()
	{
		GLFUNC->glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		GLFUNC->glGenTextures(1, &tbo); 
		GLFUNC->glBindTexture(GL_TEXTURE_2D, tbo);
		GLFUNC->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		GLFUNC->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		GLFUNC->glBindTexture(GL_TEXTURE_2D, 0);

		GLFUNC->glGenRenderbuffers(1, &rbo);
		GLFUNC->glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		GLFUNC->glBindRenderbuffer(GL_RENDERBUFFER, 0);

		GLFUNC->glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	GLColorFrameBufferObject::~GLColorFrameBufferObject()
	{	
		GLFUNC->glBindTexture(GL_TEXTURE_2D, 0);
		GLFUNC->glDeleteTextures(1, &tbo);
		GLFUNC->glBindRenderbuffer(GL_RENDERBUFFER, 0);
		GLFUNC->glDeleteRenderbuffers(1, &rbo);
	}

	void GLColorFrameBufferObject::bind()
	{
		GLFUNC->glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		if (GLFUNC->glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			LOG(FATAL) << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!";
	}

	void GLColorFrameBufferObject::resize(int height, int width)
	{
		GLFrameBufferObject::resize(height, width);
		GLFUNC->glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		GLFUNC->glBindTexture(GL_TEXTURE_2D, tbo);
		GLFUNC->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);

		GLFUNC->glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		GLFUNC->glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH32F_STENCIL8, width, height);

		GLFUNC->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tbo, 0);
		GLFUNC->glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
		GLFUNC->glBindTexture(GL_TEXTURE_2D, 0);
		GLFUNC->glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}

	GLMultiSampleFrameBufferObject::GLMultiSampleFrameBufferObject(int nsample)
		:nsample(nsample)
	{
		GLFUNC->glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		GLFUNC->glGenTextures(1, &mstbo);
		GLFUNC->glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, mstbo);
		//glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		GLFUNC->glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

		GLFUNC->glGenRenderbuffers(1, &msrbo);
		GLFUNC->glBindRenderbuffer(GL_RENDERBUFFER, msrbo);
		GLFUNC->glBindRenderbuffer(GL_RENDERBUFFER, 0);

		GLFUNC->glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	GLMultiSampleFrameBufferObject::~GLMultiSampleFrameBufferObject()
	{
		GLFUNC->glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
		GLFUNC->glDeleteTextures(1, &mstbo);
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
		GLFUNC->glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, mstbo);
		GLFUNC->glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, nsample, GL_RGB16F, width, height, GL_TRUE);
		GLFUNC->glBindRenderbuffer(GL_RENDERBUFFER, msrbo);
		GLFUNC->glRenderbufferStorageMultisample(GL_RENDERBUFFER, nsample, GL_DEPTH32F_STENCIL8, width, height);
		GLFUNC->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, mstbo, 0);
		GLFUNC->glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, msrbo);
		GLFUNC->glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
		GLFUNC->glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}

	int GLMultiSampleFrameBufferObject::textureId(int idx/*=0*/)
{
		return mstbo;
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

	void GLFrameBufferObject::resize(int height, int width)
	{
		h = height;
		w = width;
	}

	GLShadowMapFrameBufferObject::GLShadowMapFrameBufferObject()
		:GLFrameBufferObject()
	{
		GLFUNC->glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		GLFUNC->glGenTextures(1, &cubetbo);
		GLFUNC->glBindTexture(GL_TEXTURE_CUBE_MAP, cubetbo); 
		GLFUNC->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		GLFUNC->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		GLFUNC->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		GLFUNC->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		GLFUNC->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		GLFUNC->glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		GLFUNC->glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	GLShadowMapFrameBufferObject::~GLShadowMapFrameBufferObject()
	{
		GLFUNC->glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		GLFUNC->glDeleteTextures(1, &cubetbo);
	}

	void GLShadowMapFrameBufferObject::bind()
	{
		GLFUNC->glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		GLFUNC->glDrawBuffer(GL_NONE);
		GLFUNC->glReadBuffer(GL_NONE);
	}

	void GLShadowMapFrameBufferObject::resize(int height, int width)
	{
		GLFrameBufferObject::resize(height, width);
		GLFUNC->glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		GLFUNC->glBindTexture(GL_TEXTURE_CUBE_MAP, cubetbo);
		for (GLuint i = 0; i < 6; i++) {
			GLFUNC->glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		}
		GLFUNC->glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, cubetbo, 0);
		GLFUNC->glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

		GLFUNC->glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	int GLShadowMapFrameBufferObject::textureId(int idx/*=0*/)
{
		return cubetbo;
	}

	GLMtrFrameBufferObject::GLMtrFrameBufferObject(int nsample)
		:nsample(nsample)
	{
		GLFUNC->glGenFramebuffers(1, &interFbo);
		GLFUNC->glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		
		GLFUNC->glGenTextures(nTargetType, targetTex);
		GLFUNC->glGenTextures(nTargetType, outputTex);
		for (int i = 0; i < nTargetType; i++) {
			GLFUNC->glBindTexture(GL_TEXTURE_2D, outputTex[i]);
			GLFUNC->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			GLFUNC->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			GLFUNC->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			GLFUNC->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			GLFUNC->glBindTexture(GL_TEXTURE_2D, 0);
		}

		GLFUNC->glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	GLMtrFrameBufferObject::~GLMtrFrameBufferObject()
	{
		GLFUNC->glBindTexture(GL_TEXTURE_2D, 0);
		GLFUNC->glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
		GLFUNC->glDeleteTextures(nTargetType, targetTex);
		GLFUNC->glDeleteFramebuffers(1, &interFbo);
		GLFUNC->glDeleteTextures(nTargetType, outputTex);
	}

	void GLMtrFrameBufferObject::bind()
	{
		GLFUNC->glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		GLuint drawbuffers[nTargetType-1];
		for (int i = 0; i < nTargetType-1; i++) {
			drawbuffers[i] = GL_COLOR_ATTACHMENT0 + i;
		}
		GLFUNC->glDrawBuffers(nTargetType - 1, drawbuffers);
		if (GLFUNC->glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			LOG(FATAL) << GLFUNC->glCheckFramebufferStatus(GL_FRAMEBUFFER);
	}

	void GLMtrFrameBufferObject::resize(int height, int width)
	{
		GLFrameBufferObject::resize(height, width);
		GLFUNC->glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		for (int i = 0; i < nTargetType-1; i++) {
			GLFUNC->glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, targetTex[i]);
			GLFUNC->glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, nsample, targetFromats[i], width, height, GL_TRUE);
			GLFUNC->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+i, GL_TEXTURE_2D_MULTISAMPLE, targetTex[i], 0);
		}
		GLFUNC->glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, targetTex[DEPTH]);
		GLFUNC->glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, nsample, targetFromats[DEPTH], width, height, GL_TRUE);
		GLFUNC->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, targetTex[DEPTH], 0);

		GLFUNC->glBindFramebuffer(GL_FRAMEBUFFER, interFbo);
		for (int i = 0; i < nTargetType - 1; i++) {
			GLFUNC->glBindTexture(GL_TEXTURE_2D, outputTex[i]);
			GLFUNC->glTexImage2D(GL_TEXTURE_2D, 0, targetFromats[i], width, height, 0, targetBaseFromats[i], targetUnitFromats[i], NULL);
			GLFUNC->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, outputTex[i], 0);
		}
		GLFUNC->glBindTexture(GL_TEXTURE_2D, outputTex[DEPTH]);
		GLFUNC->glTexImage2D(GL_TEXTURE_2D, 0, targetFromats[DEPTH], width, height, 0, targetBaseFromats[DEPTH], targetUnitFromats[DEPTH], NULL);
		GLFUNC->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, outputTex[DEPTH], 0);

		GLFUNC->glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
		GLFUNC->glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	int GLMtrFrameBufferObject::textureId(int idx/*=0*/)
{
		return targetTex[idx];
	}

	std::vector<GLuint> GLMtrFrameBufferObject::transferedTextureId()
	{
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
		return std::vector<GLuint>(outputTex, outputTex + nTargetType);
	}

	std::array<GLuint, GLMtrFrameBufferObject::nTargetType> GLMtrFrameBufferObject::targetFromats = {
		GL_RGB16F, //COLOR
		GL_RGB16F, //ALBEDO
		GL_RGB32F,//WORLD POS
		GL_RGB32F,//NORMAL
		GL_DEPTH_COMPONENT, //DEPTH
	};

	std::array<GLuint, mcl::GLMtrFrameBufferObject::nTargetType> GLMtrFrameBufferObject::targetBaseFromats = {
		GL_RGB, //COLOR
		GL_RGB, //ALBEDO
		GL_RGB,//WORLD POS
		GL_RGB,//NORMAL
		GL_DEPTH_COMPONENT, //DEPTH
	};

	std::array<GLuint, mcl::GLMtrFrameBufferObject::nTargetType> GLMtrFrameBufferObject::targetUnitFromats = {
		GL_FLOAT, //COLOR
		GL_FLOAT, //ALBEDO
		GL_FLOAT,//WORLD POS
		GL_FLOAT,//NORMAL
		GL_FLOAT, //DEPTH
	};

}