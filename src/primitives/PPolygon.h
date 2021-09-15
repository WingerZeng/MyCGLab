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

struct DrawSingleObjInfo{ //���Ƶ����������Ϣ��������һ�������б���������
	DrawSingleObjInfo(GLenum atype, int aoffset, int asize): type(atype), offset(aoffset), size(asize) {};
	DrawSingleObjInfo() = default;
	GLenum type;
	int offset;
	int size;
};

Normal3f calPolygonNormal(const std::vector<int>& lp, const std::vector<Point3f>& pts);

int tessPolygon(const std::vector<std::vector<PType3f>>& lps, std::vector<Float>* tessPts,std::vector<DrawSingleObjInfo>* drawInfo);

/**
 * @brief ������������ɢΪ�ɻ������ݣ�tessPts��������У�offsets���ÿ��ͼ�εĻ��������붥��ƫ��
 */
int tessPolygons(const std::vector<std::vector<std::vector<PType3f>>>& plgs, std::vector<Float>* tessPts, std::vector<DrawSingleObjInfo>* drawInfo);
/*Render Interface of solid*/
//std::vector<std::shared_ptr<PPolygon>> solidToPolygons(bpSolid* solid);
}
