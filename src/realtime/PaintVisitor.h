#pragma once
#include "mcl.h"
#include "types.h"
namespace mcl {
class PaintVisitor{
public:
	virtual ~PaintVisitor() {};

	virtual int paintTris(PaintInfomation* info, PTriMesh* tri);

	virtual int paintPoint(PaintInfomation* info, PPoint* point);

	virtual int paintPolygons(PaintInfomation* info, PPolygonMesh* polygon);

	virtual int paintLines(PaintInfomation* info, PLines* lines);
protected:
	PaintVisitor() {};
};

}

