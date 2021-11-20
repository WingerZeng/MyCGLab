#pragma once
#include "mcl.h"
#include "types.h"
#include "PostProcPaintVisitor.h"
#include "PaintVisitor.h"
#include "DefaultPaintVisitor.h"
#include "CubeShadowMapPaintVisitor.h"
#include "GLTexture.h"
#include "GLFrameBufferObject.h"
#include "PTriMesh.h"
namespace mcl {
	class Painter
	{
	public:
		virtual ~Painter() {}

		virtual void paint(PaintInfomation* info) = 0;

		virtual void init(PaintInfomation* info){};

		virtual void resize(int w, int h) {};
	};

	class LightsShadowMapPainter: public Painter
	{
	public:
		void paint(PaintInfomation* info) override;
	};

	class MipmapBufferPainter: public Painter
	{
	public:
		MipmapBufferPainter(int maxLevels, int stopSize = 1, GLTextureFormat format = {
				GL_RGB16F, GL_RGB, GL_FLOAT,GL_NEAREST, GL_NEAREST,GL_CLAMP_TO_EDGE,GL_CLAMP_TO_EDGE
			});
		void resize(int w, int h) override;

	protected:
		int stopSize = 1;
		int mipLevel = 0;
		std::vector<std::shared_ptr<AbstractGLFrameBufferObject>> fbos;
	};

	class BloomFilterPainter : public MipmapBufferPainter
	{
	public:
		using MipmapBufferPainter::MipmapBufferPainter;

		virtual void paint(PaintInfomation* info) override;

		virtual void init(PaintInfomation* info) override;
	};

	class HiZFilterPainter :public MipmapBufferPainter
	{
	public:
		HiZFilterPainter(int maxLevels);

		virtual void paint(PaintInfomation* info) override;

		virtual void init(PaintInfomation* info) override;

	};

	class SingleBufferPainter : public Painter
	{
	public:
		SingleBufferPainter(std::shared_ptr<AbstractGLFrameBufferObject> fbo);
		SingleBufferPainter() = default;
		void resize(int w, int h) override;

	protected:
		std::shared_ptr<AbstractGLFrameBufferObject> fbo;
	};
	
	class DeferredMtrPainter : public SingleBufferPainter 
	{
	public:
		DeferredMtrPainter();

		virtual void paint(PaintInfomation* info) override;

		virtual void init(PaintInfomation* info) override;

	};

	template<class T>
	class PostProcPainter : public SingleBufferPainter
	{
	public:
		PostProcPainter(std::shared_ptr<AbstractGLFrameBufferObject> fbo)
			:SingleBufferPainter(fbo)
		{
		}

		virtual void paint(PaintInfomation* info) override {
			glViewport(0, 0, info->width, info->height);
			fbo->bind();
			fbo->clear(info);
			PTriMesh::getBillBoard()->paint(info, Singleton<T>::getSingleton());
			std::shared_ptr<GLTexture2D> tex2D; 
			if (fbo->texture())
				tex2D = std::dynamic_pointer_cast<GLTexture2D>(fbo->texture());
			if (tex2D) {
				setTexture(tex2D);
			}
		}

		virtual void setTexture(std::shared_ptr<GLTexture2D> tex) {
			tex->setFilter(GL_NEAREST, GL_NEAREST);
			tex->setWrap(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
		}
		
	protected:
		using Super = PostProcPainter<T>;
	};

	class DirectLightPainter : public PostProcPainter<DirectLightPaintVisitor>
	{
	public:
		using Super::Super;

		void init(PaintInfomation* info) override;
	};

	class DirectLightFilterPainter : public PostProcPainter<DirectLightFilterPaintVisitor>
	{
	public:
		using Super::Super;

		void init(PaintInfomation* info) override;
	};

	class SsrPainter : public PostProcPainter<SsrPaintVisitor>
	{
	public:
		using Super::Super;

		void init(PaintInfomation* info) override;
	};

	class HiZSsrPainter : public PostProcPainter<HiZSsrPaintVisitor>
	{
	public:
		using Super::Super;

		void init(PaintInfomation* info) override;
	};

	class SsrFilterPainter : public PostProcPainter<SsrFilterPaintVisitor>
	{
	public:
		using Super::Super;

		void init(PaintInfomation* info) override;
	};

	class SsdoPainter : public PostProcPainter<SsdoPaintVisitor>
	{
	public:
		using Super::Super;

		void init(PaintInfomation* info) override;
	};

	class SsdoFilterPainter : public PostProcPainter<SsdoFilterPaintVisitor>
	{
	public:
		using Super::Super;

		void init(PaintInfomation* info) override;

		void setTexture(std::shared_ptr<GLTexture2D> tex) override;
	};

	class ToneMapPainter : public PostProcPainter<ToneMapPaintVisitor>
	{
	public:
		using Super::Super;

		void init(PaintInfomation* info) override;

		void setTexture(std::shared_ptr<GLTexture2D> tex) override;
	};

	class FxaaPainter : public PostProcPainter<FxaaPaintVisitor>
	{
	public:
		using Super::Super;

		void init(PaintInfomation* info) override;
	};
}

