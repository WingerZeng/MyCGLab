#pragma once
#include <array>
#include "mcl.h"
#include "types.h"
namespace mcl {
	class GLFrameBufferObject {
	public:
		GLFrameBufferObject();
		virtual ~GLFrameBufferObject();

		virtual void bind() = 0;

		virtual void resize(int height, int width);

		virtual std::shared_ptr<GLAbstractTexture> texture(int idx=0) = 0;

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

		std::shared_ptr<GLAbstractTexture> texture(int idx = 0) override;

	private:
		GLuint rbo;
		std::shared_ptr<GLTexture2D> tex;
	};

	class GLMultiSampleFrameBufferObject :public GLFrameBufferObject
	{
	public:
		GLMultiSampleFrameBufferObject(int nsample);
		~GLMultiSampleFrameBufferObject();

		void bind() override;

		void resize(int height, int width) override;

		std::shared_ptr<GLAbstractTexture> texture(int idx = 0) override;

		void copyToFbo(GLuint fbo);

	private:
		int nsample;
		GLuint msrbo;
		std::shared_ptr<GLTextureMultiSample> mstex;
	};

	class GLShadowMapFrameBufferObject : public GLFrameBufferObject
	{
	public:
		GLShadowMapFrameBufferObject();
		~GLShadowMapFrameBufferObject();;

		void bind() override;

		void resize(int height, int width) override;

		std::shared_ptr<GLAbstractTexture> texture(int idx = 0) override;

	private:
		std::shared_ptr<GLTextureCubeMap> tex;
	};

	class GLMtrFrameBufferObject : public GLFrameBufferObject
	{

	public:
		GLMtrFrameBufferObject(int nsample);
		~GLMtrFrameBufferObject();

		virtual void bind() override;

		virtual void resize(int height, int width) override;

		std::shared_ptr<GLAbstractTexture> texture(int idx = 0) override;

		std::vector<std::shared_ptr<GLAbstractTexture>> transferedTextureId();

		enum TargetType
		{
			COLOR = 0,
			ALBEDO,
			WORLD_POS,
			NORMAL,
			DEPTH,
		};
		static const int nTargetType = 5;

		static std::array<GLuint, nTargetType> targetFromats;
		static std::array<GLuint, nTargetType> targetBaseFromats;
		static std::array<GLuint, nTargetType> targetUnitFromats;

	private:
		int nsample;
		std::vector<std::shared_ptr<GLTextureMultiSample>> targetTex;
		std::vector<std::shared_ptr<GLTexture2D>> outputTex;
		GLuint interFbo;
	};
}

