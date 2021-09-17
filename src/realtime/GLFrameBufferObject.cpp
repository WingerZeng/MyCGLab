#include "GLFrameBufferObject.h"
namespace mcl {
	
	GLFrameBufferObject::GLFrameBufferObject()
	{
		initializeOpenGLFunctions();
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		
		glGenTextures(1, &tbo); 
		glBindTexture(GL_TEXTURE_2D, tbo);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);

		glGenRenderbuffers(1, &rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	GLFrameBufferObject::~GLFrameBufferObject()
	{	
		//½â°ó
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDeleteFramebuffers(1, &fbo);
		glBindTexture(GL_TEXTURE_2D, 0);
		glDeleteTextures(1, &tbo);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glDeleteRenderbuffers(1, &rbo);
	}

	void GLFrameBufferObject::bind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			LOG(FATAL) << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!";
	}

	void GLFrameBufferObject::resize(int height, int width)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glBindTexture(GL_TEXTURE_2D, tbo);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH32F_STENCIL8, width, height);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tbo, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		w = width;
		h = height;
	}

	GLMultiSampleFrameBufferObject::GLMultiSampleFrameBufferObject(int nsample)
		:nsample(nsample)
	{
		initializeOpenGLFunctions();
		glGenFramebuffers(1, &msfbo);
		glBindFramebuffer(GL_FRAMEBUFFER, msfbo);

		glGenTextures(1, &mstbo);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, mstbo);
		//glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

		glGenRenderbuffers(1, &msrbo);
		glBindRenderbuffer(GL_RENDERBUFFER, msrbo);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	GLMultiSampleFrameBufferObject::~GLMultiSampleFrameBufferObject()
	{
		//½â°ó
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDeleteFramebuffers(1, &msfbo);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
		glDeleteTextures(1, &mstbo);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glDeleteRenderbuffers(1, &msrbo);
	}

	void GLMultiSampleFrameBufferObject::bind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, msfbo);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			LOG(FATAL) << glCheckFramebufferStatus(GL_FRAMEBUFFER);
	}

	void GLMultiSampleFrameBufferObject::resize(int height, int width)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, msfbo);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, mstbo);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, nsample, GL_RGB, width, height, GL_TRUE);
		glBindRenderbuffer(GL_RENDERBUFFER, msrbo);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, nsample, GL_DEPTH32F_STENCIL8, width, height);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, mstbo, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, msrbo);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		w = width;
		h = height;
	}

	int GLMultiSampleFrameBufferObject::msTextureId()
	{
		return mstbo;
	}

	void GLMultiSampleFrameBufferObject::copyToFbo(GLuint fbo)
	{
		glBindFramebuffer(GL_READ_FRAMEBUFFER, msfbo);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
		glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	}

}