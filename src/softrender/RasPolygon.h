#pragma once
#include "mcl.h"
#include "types.h"

namespace mcl {
	/*************************
	�������ڹ�դ���Ķ��������
	************************/

	struct RasPlgNode;
	struct RasPlgEdge;
	struct RasPolygon;
	struct RasBiEdge;

	struct RasPlgNode { //����νڵ�
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
		Point2i scoord; //��Ļ����ϵ����
		Float depth;	//���
		Point3f wcoord; //��������ϵ����
	};
	struct RasPolygon
	{
		std::list<std::unique_ptr<RasPlgEdge>> edges;
		std::list<std::unique_ptr<RasPlgEdge>> activeEdges;
		Primitive* prim;
		int miny, maxy; //��¼����ε����y
		Normal3f normal;
	};
	struct RasPlgEdge
	{
		RasPlgEdge(const RasPlgNode& n1, const RasPlgNode& n2) {
			if (n1.upperThan(n2)) {  // yֵС��Ϊnode[0]
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
		void moveToNxtLine() { //�ƶ�����һ��ɨ���ߣ�����xλ��
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
		Float dxdy; //б��
		Float dzdy;
		Float curX;
		Float curZ;
	};
	struct RasBiEdge //�߶�
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

	/* �������غ��� */
	inline int getPlgMinY(const RasPolygon* plg); //���ض������δʹ�õıߵ��Ͻ�
	inline void freePolygon(RasPolygon* plg); //���������
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