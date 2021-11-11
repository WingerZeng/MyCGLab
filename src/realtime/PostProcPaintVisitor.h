#pragma once
#include "mcl.h"
#include "types.h"
#include "PaintVisitor.h"
namespace mcl {
	class PostProcPaintVisitor: public PaintVisitor
	{
	};

	class ToneMapPaintVisitor : public PostProcPaintVisitor
	{
	public:
		ToneMapPaintVisitor();

		virtual int paintTris(PaintInfomation* info, PTriMesh* tri) override;
	};

	class DirectLightPaintVisitor : public PostProcPaintVisitor
	{
	public:
		virtual int paintTris(PaintInfomation* info, PTriMesh* tri) override;
	};

	class DirectLightFilterPaintVisitor : public PostProcPaintVisitor
	{
	public:
		virtual int paintTris(PaintInfomation* info, PTriMesh* tri) override;
	};

	class SsrPaintVisitor : public PostProcPaintVisitor
	{
	public:
		virtual int paintTris(PaintInfomation* info, PTriMesh* tri) override;
	};

	class SsrPaintFilterVisitor : public PostProcPaintVisitor
	{
	public:
		virtual int paintTris(PaintInfomation* info, PTriMesh* tri) override;
	};

	class SsdoPaintVisitor : public PostProcPaintVisitor
	{
	public:
		virtual int paintTris(PaintInfomation* info, PTriMesh* tri) override;
	};

	class SsdoFilterPaintVisitor : public PostProcPaintVisitor
	{
	public:
		virtual int paintTris(PaintInfomation* info, PTriMesh* tri) override;
	};

	class BloomFilterPaintVisitor : public PostProcPaintVisitor
	{
	public:
		virtual int paintTris(PaintInfomation* info, PTriMesh* tri) override;
	};

	class FxaaPaintVisitor: public PostProcPaintVisitor
	{
	public:
		virtual int paintTris(PaintInfomation* info, PTriMesh* tri) override;
	};
}

