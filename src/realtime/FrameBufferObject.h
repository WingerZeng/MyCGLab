#pragma once
#include "mcl.h"
#include "types.h"
#include "QOpenGLFunctions_4_5_Core"
namespace mcl {
	class FrameBufferObject:public QOpenGLFunctions_4_5_Core
	{
	public:
		FrameBufferObject();
		~FrameBufferObject();

		void bind();

		void resize(int height, int width);

		int textureId() { return tbo; }

	private:
		GLuint fbo;
		GLuint rbo;
		GLuint tbo;
	};
}

