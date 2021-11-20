#pragma once
#include "mcl.h"
#include "types.h"
#include "PaintVisitor.h"
class QOpenGLShaderProgram;
namespace mcl {
	class PostProcPaintVisitor: public PaintVisitor
	{
	public:

		virtual void setUniform(PaintInfomation* info) = 0;

		virtual QOpenGLShaderProgram* getDefaultShader() = 0;

		virtual int paintTris(PaintInfomation* info, PTriMesh* tri) override;
	};

	class ToneMapPaintVisitor : public PostProcPaintVisitor
	{
	public:

		virtual void setUniform(PaintInfomation* info) override;

		virtual QOpenGLShaderProgram* getDefaultShader() override;

	};

	class DirectLightPaintVisitor : public PostProcPaintVisitor
	{
	public:
		virtual void setUniform(PaintInfomation* info) override;

		virtual QOpenGLShaderProgram* getDefaultShader() override;

	};

	class DirectLightFilterPaintVisitor : public PostProcPaintVisitor
	{
	public:
		virtual void setUniform(PaintInfomation* info) override;

		virtual QOpenGLShaderProgram* getDefaultShader() override;

	};

	class SsrPaintVisitor : public PostProcPaintVisitor
	{
	public:
		virtual void setUniform(PaintInfomation* info) override;

		virtual QOpenGLShaderProgram* getDefaultShader() override;

	};

	class HiZSsrPaintVisitor : public PostProcPaintVisitor
	{
	public:
		virtual void setUniform(PaintInfomation* info) override;

		virtual QOpenGLShaderProgram* getDefaultShader() override;
	};

	class SsrFilterPaintVisitor : public PostProcPaintVisitor
	{
	public:
		virtual void setUniform(PaintInfomation* info) override;

		virtual QOpenGLShaderProgram* getDefaultShader() override;

	};

	class SsdoPaintVisitor : public PostProcPaintVisitor
	{
	public:
		virtual void setUniform(PaintInfomation* info) override;
		
		virtual QOpenGLShaderProgram* getDefaultShader() override;

	};

	class SsdoFilterPaintVisitor : public PostProcPaintVisitor
	{
	public:
		virtual void setUniform(PaintInfomation* info) override;

		virtual QOpenGLShaderProgram* getDefaultShader() override;

	};

	class BloomFilterPaintVisitor : public PostProcPaintVisitor
	{
	public:
		virtual void setUniform(PaintInfomation* info) override;

		virtual QOpenGLShaderProgram* getDefaultShader() override;

	};

	class FxaaPaintVisitor: public PostProcPaintVisitor
	{
	public:
		virtual void setUniform(PaintInfomation* info) override;

		virtual QOpenGLShaderProgram* getDefaultShader() override;

	};

	class HiZFilterPaintVisitor : public PostProcPaintVisitor
	{
	public:
		virtual void setUniform(PaintInfomation* info) override;

		virtual QOpenGLShaderProgram* getDefaultShader() override;
	};
}

