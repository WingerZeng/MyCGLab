#pragma once
#include "Primitive.h"
#include <QtGui/QOpenGLVertexArrayObject>
#include <QtGui/QOpenGLBuffer>
#include "core/mcl.h"
#include "core/types.h"
#include <map>
#include <utility>
namespace mcl {
void CALLBACK vertexCallback(GLvoid * vertex);
void CALLBACK beginCallback(GLenum type);
void CALLBACK endCallback();
void CALLBACK errorCallback(GLenum errorCode);
class PaintInfomation;
class bpSolid;

struct DrawSingleObjInfo{ //绘制单个对象的信息，用于在一个数组中保存多个对象
	DrawSingleObjInfo(GLenum atype, int aoffset, int asize): type(atype), offset(aoffset), size(asize) {};
	DrawSingleObjInfo() = default;
	GLenum type;
	int offset;
	int size;
};

Normal3f calPolygonNormal(const std::vector<int>& lp, const std::vector<Point3f>& pts);

int tessPolygon(const std::vector<std::vector<PType3f>>& lps, std::vector<Float>* tessPts,std::vector<DrawSingleObjInfo>* drawInfo);

/**
 * @brief 将多个多边形离散为可绘制数据，tessPts输出点序列，offsets输出每个图形的绘制类型与顶点偏移
 */
int tessPolygons(const std::vector<std::vector<std::vector<PType3f>>>& plgs, std::vector<Float>* tessPts, std::vector<DrawSingleObjInfo>* drawInfo);
/*Render Interface of solid*/
//std::vector<std::shared_ptr<PPolygon>> solidToPolygons(bpSolid* solid);
}
