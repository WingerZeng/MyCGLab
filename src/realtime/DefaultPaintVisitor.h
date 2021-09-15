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


		virtual int initTris(PTriMesh* tri) override;


		virtual int paintPoint(PaintInfomation* info, PPoint* point) override;


		virtual int initPoint(PPoint* point) override;


		virtual int paintPolygons(PaintInfomation* info, PPolygonMesh* polygon) override;


		virtual int initPolygons(PPolygonMesh* polygon) override;


		virtual int paintLines(PaintInfomation* info, PLines* lines) override;


		virtual int initLines(PLines* lines) override;

	};
}

