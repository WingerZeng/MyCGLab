#pragma once
#include <array>
#include <QOpenGLWidget>
#include "mcl.h"
#include "types.h"

namespace mcl {
	class AbstractGLFrameBufferObject {
	public:
		virtual ~AbstractGLFrameBufferObject();

		virtual void bind() = 0;

		virtual void clear(PaintInfomation* info) = 0;

		virtual void resize(int height, int width) = 0;

		template<class T>
		std::shared_ptr<T> castedTexture(int idx = 0) {
			return std::dynamic_pointer_cast<T>(texture(idx));
		}

		virtual std::shared_ptr<GLAbstractTexture> texture(int idx = 0) = 0;

		virtual GLuint fboId() = 0;
	};

	class GLFrameBufferObject: public AbstractGLFrameBufferObject
	{
	public:
		GLFrameBufferObject();
		virtual ~GLFrameBufferObject();

		virtual void bind() = 0;

		virtual void clear(PaintInfomation* info) = 0;

		virtual void resize(int height, int width);

		virtual std::shared_ptr<GLAbstractTexture> texture(int idx=0) = 0;

		GLuint fboId() { return fbo; };

	protected:
		GLuint fbo;
		int w, h;
	};

	class GLQWidgetFrameBufferObject : public AbstractGLFrameBufferObject
	{
	public:
		GLQWidgetFrameBufferObject(QOpenGLWidget* qwgt);

		virtual void bind();

		virtual void clear(PaintInfomation* info) override;

		virtual void resize(int height, int width) override;

		virtual std::shared_ptr<GLAbstractTexture> texture(int idx = 0) override;

		virtual GLuint fboId() override;

	protected:
		QOpenGLWidget* qwgt;
	};

	class GLColorFrameBufferObject :public GLFrameBufferObject
	{
	public:
		GLColorFrameBufferObject();
		~GLColorFrameBufferObject();

		void bind() override;

		void clear(PaintInfomation* info) override;

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

		void clear(PaintInfomation* info) override;

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

		void clear(PaintInfomation* info) override;

		virtual void resize(int height, int width) override;

		std::shared_ptr<GLAbstractTexture> texture(int idx = 0) override;

		std::vector<std::shared_ptr<GLAbstractTexture>> transferedTextureId();

		enum TargetType
		{
			COLOR = 0,
			ALBEDO,	//ALBEDO的A通道存材质序号
			SPECULAR,
			WORLD_POS,
			NORMAL,
			PRIMID,
			DEPTH,
		};
		static const int nTargetType = 7;

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

