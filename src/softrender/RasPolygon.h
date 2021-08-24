#pragma once
#include "mcl.h"
#include "types.h"

namespace mcl {
	/*************************
	定义用于光栅化的多边形数据
	************************/

	struct RasPlgNode;
	struct RasPlgEdge;
	struct RasPolygon;
	struct RasBiEdge;

	struct RasPlgNode { //多边形节点
		RasPlgNode() = default;
		RasPlgNode(Point3f wc, Transform trans)
			:wcoord(wc) {
			wc = trans(wc);
			depth = wc.z();
			scoord.x() = int(wc.x());
			scoord.y() = int(wc.y());
		}
		bool upperThan(const RasPlgNode& rhs) const {
			return scoord.y() < rhs.scoord.y();
		}
		Point2i scoord; //屏幕坐标系坐标
		Float depth;	//深度
		Point3f wcoord; //世界坐标系坐标
	};
	struct RasPolygon
	{
		std::list<std::unique_ptr<RasPlgEdge>> edges;
		std::list<std::unique_ptr<RasPlgEdge>> activeEdges;
		Primitive* prim;
		int miny, maxy; //记录多边形的最大y
		Normal3f normal;
	};
	struct RasPlgEdge
	{
		RasPlgEdge(const RasPlgNode& n1, const RasPlgNode& n2) {
			if (n1.upperThan(n2)) {  // y值小的为node[0]
				node[0] = n1;
				node[1] = n2;
			}
			else {
				node[0] = n2;
				node[1] = n1;
			}
			dxdy = (node[0].scoord.x() - node[1].scoord.x())*1.0f / (node[0].scoord.y() - node[1].scoord.y());
			dzdy = (node[0].depth - node[1].depth) / (node[0].scoord.y() - node[1].scoord.y());
			curX = node[0].scoord.x() + 0.5;
			curZ = node[0].depth;
		}
		int getUpperBound() {
			return node[0].scoord.y();
		};
		int getLowerBound() {
			return node[1].scoord.y();
		};
		void moveToNxtLine() { //移动到下一条扫描线，更新x位置
			curX += dxdy;
			curZ += dzdy;
		}
		Vector2i getScreenVec() {
			return (node[1].scoord - node[0].scoord);
		}
		Vector3f getWorldVec() {
			return node[1].wcoord - node[0].wcoord;
		}
		RasPlgNode node[2];
		Float dxdy; //斜率
		Float dzdy;
		Float curX;
		Float curZ;
	};
	struct RasBiEdge //边对
	{
		RasBiEdge(RasPlgEdge* e1, RasPlgEdge* e2) {
			if (e1->curX < e2->curX) {
				egs[0] = e1;
				egs[1] = e2;
			}
			else {
				egs[1] = e1;
				egs[0] = e2;
			}
			if (egs[0]->curX == egs[1]->curX) dzdx_ = 0;
			else dzdx_ = (egs[0]->curZ - egs[1]->curZ) / (egs[0]->curX - egs[1]->curX);
		}
		Float dzdx() {
			return dzdx_;
		}
		Float dzdx_;
		RasPlgEdge* egs[2];
	};

	/* 多边形相关函数 */
	inline int getPlgMinY(const RasPolygon* plg); //返回多边形中未使用的边的上界
	inline void freePolygon(RasPolygon* plg); //析构多边形
	inline bool isHorizon(const RasPlgNode& n1, const RasPlgNode& n2);
	//inline NodeType getNodeType(PlgNode* node);
	//inline PlgNode* getLowerNode(PlgNode* node);

	inline void freePolygon(RasPolygon* plg)
	{
		delete plg;
	}

	inline int getPlgMinY(const RasPolygon* plg)
	{
		if (plg->edges.empty()) return MaxInt;
		return (*(plg->edges.begin()))->getUpperBound();
	}

	inline bool isHorizon(const RasPlgNode& n1, const RasPlgNode& n2) {
		return n1.scoord.y() == n2.scoord.y();
	}
}