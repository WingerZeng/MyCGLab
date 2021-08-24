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

class PPolygon : public GeometryPrimitive
{
	RTTI_CLASS(PPolygon)
public:
	PPolygon(std::vector<PType3f> pts) //one loop
		:lps_{ pts } {
		setColor(Color3f{0.6,0.6,0.6});
	};
	PPolygon(std::vector<std::vector<PType3f>> lps) //multi loop
		:lps_(lps) {
		setColor(Color3f{ 0.6,0.6,0.6 });
	};
	~PPolygon();
	virtual void initialize() override;
	virtual void paint(PaintInfomation* info) override;
	bool checkNormal(Vec3f& normal); //Judge the normal of polygon
	static PPolygon* currentTessPolygon;

private:
	bool readyToDraw = false;

	std::vector<std::vector<PType3f>> lps_; // #PERF1 这个成员可以改成指针，用完释放
	std::vector<Float> boundPts_;
	std::vector<DrawSingleObjInfo> boundDrawInfo_; //记录每条边线在boundPts中的offset
	std::vector<Float> tessPts_;
	std::vector<DrawSingleObjInfo> drawInfo_;

	std::shared_ptr<QOpenGLBuffer > vbo;
	std::shared_ptr<QOpenGLVertexArrayObject > vao;

	std::shared_ptr<QOpenGLBuffer> linevbo;
	std::shared_ptr<QOpenGLVertexArrayObject> linevao;

	Vector3f normal_;
};

Normal3f calPolygonNormal(const std::vector<int>& lp, const std::vector<Point3f>& pts);

int tessPolygon(const std::vector<std::vector<PType3f>>& lps, std::vector<Float>* tessPts,std::vector<DrawSingleObjInfo>* drawInfo);

/**
 * @brief 将多个多边形离散为可绘制数据，tessPts输出点序列，offsets输出每个图形的绘制类型与顶点偏移
 */
int tessPolygons(const std::vector<std::vector<std::vector<PType3f>>>& plgs, std::vector<Float>* tessPts, std::vector<DrawSingleObjInfo>* drawInfo);
/*Render Interface of solid*/
std::vector<std::shared_ptr<PPolygon>> solidToPolygons(bpSolid* solid);
}
