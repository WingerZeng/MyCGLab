#pragma once
#include "mcl.h"
#include "types.h"
#include "PaintVisitor.h"
namespace mcl {
	class DefaultPaintVisitor: public PaintVisitor
	{
	public:
		DefaultPaintVisitor() {};

		virtual int paintTris(PaintInfomation* info, PTriMesh* tri) override;


		virtual int paintPoint(PaintInfomation* info, PPoint* point) override;


		virtual int paintPolygons(PaintInfomation* info, PPolygonMesh* polygon) override;


		virtual int paintLines(PaintInfomation* info, PLines* lines) override;

	private:
		void setPointUniform(PaintInfomation* info);
		void setLineUniform(PaintInfomation* info);
	};

	class DeferredMtrPaintVisitor : public PaintVisitor
	{
	public:
		DeferredMtrPaintVisitor() {};

		virtual int paintTris(PaintInfomation* info, PTriMesh* tri) override;
	};

	class ForwardPaintVisitor : public PaintVisitor
	{
	public:
		ForwardPaintVisitor() {};
	};
}

