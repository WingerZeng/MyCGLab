#include "GLFrameBufferObject.h"
#include "GLFunctions.h"
//#TEST
#include "MainWindow.h"
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
		GLFUNC->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

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
		GLFUNC->glGenFramebuffers(1, &fbo);
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
		GLFUNC->glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, nsample, GL_RGB, width, height, GL_TRUE);
		GLFUNC->glBindRenderbuffer(GL_RENDERBUFFER, msrbo);
		GLFUNC->glRenderbufferStorageMultisample(GL_RENDERBUFFER, nsample, GL_DEPTH32F_STENCIL8, width, height);
		GLFUNC->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, mstbo, 0);
		GLFUNC->glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, msrbo);
		GLFUNC->glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
		GLFUNC->glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}

	int GLMultiSampleFrameBufferObject::textureId()
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

	int GLShadowMapFrameBufferObject::textureId()
	{
		return cubetbo;
	}

}