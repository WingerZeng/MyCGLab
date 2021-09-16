#pragma once
#include "mcl.h"
#include "types.h"
#include "QOpenGLFunctions_4_3_Core"
namespace mcl {
	class GLFrameBufferObject:public QOpenGLFunctions_4_3_Core
	{
	public:
		GLFrameBufferObject();
		~GLFrameBufferObject();

		void bind();

		void resize(int height, int width);

		int textureId() { return tbo; }

		GLuint fboId() { return fbo; }

	private:
		GLuint fbo;
		GLuint rbo;
		GLuint tbo;
		int w, h;
	};

	class GLMultiSampleFrameBufferObject :public QOpenGLFunctions_4_3_Core
	{
	public:
		GLMultiSampleFrameBufferObject(int nsample);
		~GLMultiSampleFrameBufferObject();;

		void bind();

		void resize(int height, int width);

		int msTextureId();

		void copyToFbo(GLuint fbo);

		GLuint fboId() { return msfbo; }

	private:
		int nsample;
		GLuint msfbo;
		GLuint msrbo;
		GLuint mstbo;
		int w, h;
	};
}

