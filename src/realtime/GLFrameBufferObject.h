#pragma once
#include "mcl.h"
#include "types.h"
namespace mcl {
	class GLFrameBufferObject {
	public:
		GLFrameBufferObject();
		virtual ~GLFrameBufferObject();

		virtual void bind() = 0;

		virtual void resize(int height, int width);

		virtual int textureId() = 0;

		GLuint fboId() { return fbo; };

	protected:
		GLuint fbo;
		int w, h;
	};

	class GLColorFrameBufferObject :public GLFrameBufferObject
	{
	public:
		GLColorFrameBufferObject();
		~GLColorFrameBufferObject();

		void bind() override;

		void resize(int height, int width) override;

		int textureId()  override { return tbo; }

	private:
		GLuint rbo;
		GLuint tbo;
	};

	class GLMultiSampleFrameBufferObject :public GLFrameBufferObject
	{
	public:
		GLMultiSampleFrameBufferObject(int nsample);
		~GLMultiSampleFrameBufferObject();;

		void bind() override;

		void resize(int height, int width) override;

		int textureId() override;

		void copyToFbo(GLuint fbo);

	private:
		int nsample;
		GLuint msrbo;
		GLuint mstbo;
	};

	class GLShadowMapFrameBufferObject : public GLFrameBufferObject
	{
	public:
		GLShadowMapFrameBufferObject();
		~GLShadowMapFrameBufferObject();;

		void bind() override;

		void resize(int height, int width) override;

		int textureId() override;

	private:
		GLuint cubetbo;
	};
}

