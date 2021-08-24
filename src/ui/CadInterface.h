#pragma once
#include "mcl.h"
#include "types.h"
#include <QObject>
#include "glog/logging.h"
#include "CadCore.h"

class QTreeWidgetItem;

namespace mcl {

#define DEF_ELEMENT_OPERATION(Type)\
	bp##Type* find##Type(##Type##Handle elem){\
		std::map<int,bp##Type*>& mp = itop_##Type;\
		auto it = mp.find(elem);\
		if(it == mp.end()) return nullptr;\
		return it->second;\
	}\
	##Type##Handle add##Type(bp##Type* elem){\
		CHECK(elem);\
		std::map<bp##Type*, int>& mp = ptoi_##Type;\
		CHECK(mp.find(elem) == mp.end()); /*Already exists*/ \
		itop_##Type[curId] = elem;\
		ptoi_##Type[elem] = curId;\
		++curId;\
		return curId - 1; \
	}\
	##Type##Handle find##Type(bp##Type* elem){\
		std::map<bp##Type*,int>& mp = ptoi_##Type;\
		auto it = mp.find(elem);\
		if(it == mp.end()) return ErrorHandle;\
		return it->second;\
	}\
	void del##Type(ElemHandle elem){\
		std::map<int,bp##Type*>& mp = itop_##Type;\
		auto it = mp.find(elem);\
		if(it != mp.end()) return;\
		bp##Type* pt= it->second;\
		mp.erase(it);\
		ptoi_##Type.erase(pt);\
	}

#define DEF_GET_CHILD(Type,CType)\
	std::vector<##CType##Handle> get##CType##sOf##Type##(##Type##Handle elem) {\
		bp##Type##* pelem = find##Type##(elem);\
		CHECK(elem);\
		std::vector<##CType##Handle> result;\
		for (auto it = pelem->get##CType##()->begin(); it != pelem->get##CType##()->end(); it++) {\
			result.push_back(find##CType##(*it));\
		}\
		return result;\
	}

	class CadInterface:public QObject //#TODO 析构后删除solid
	{
		Q_OBJECT
	public:
		CadInterface();

		static const int ErrorHandle = -1;
		typedef int ElemHandle;
		typedef ElemHandle SolidHandle;
		typedef ElemHandle FaceHandle;
		typedef ElemHandle LoopHandle;
		typedef ElemHandle HalfEdgeHandle;
		typedef ElemHandle VertexHandle;

		SolidHandle mvfs(const Point3f& coord);
		HalfEdgeHandle mev(VertexHandle vertex, LoopHandle lp, const Point3f& coord);
		FaceHandle mef(VertexHandle v1, VertexHandle v2, LoopHandle lp);
		int kfmrh(FaceHandle f1, FaceHandle f2);
		FaceHandle kemr(LoopHandle lp, HalfEdgeHandle he);
		int sweep(SolidHandle solid, FaceHandle face, Vector3f dir, Float dist);
		SolidHandle createFaceFromLoop(const std::vector<std::vector<Point3f>>& lps);

		/**
		 * @brief 清空一个Solid的所有信息
		 */
		void removeSolid(SolidHandle sld);

		/**
		 * @brief 抛弃solid的大部分拓扑信息，转化为可视化网格
		 */
		int solidToPolygonMesh(SolidHandle sld);

		DEF_ELEMENT_OPERATION(Solid);
		DEF_ELEMENT_OPERATION(Face);
		DEF_ELEMENT_OPERATION(Loop);
		DEF_ELEMENT_OPERATION(HalfEdge);
		DEF_ELEMENT_OPERATION(Vertex);
		
		DEF_GET_CHILD(Solid, Face);
		DEF_GET_CHILD(Solid, Vertex);
		DEF_GET_CHILD(Face, Loop);

		std::vector<HalfEdgeHandle> getHalfEdgesOfLoop(LoopHandle lp);

		enum ELEMEMNT_TYPE
		{
			SOLID = 0,
			FACE,
			LOOP,
			HALFEDGE,
			VERTEX
		};

		/*Render Interface of cad*/
		std::vector<std::shared_ptr<Primitive>> SolidToPrimitives(SolidHandle solid);
		std::vector<std::shared_ptr<Primitive>> FaceToPrimitives(FaceHandle face);
		std::vector<std::shared_ptr<Primitive>> HalfEdgeToPrimitives(HalfEdgeHandle heid);
		std::vector<std::shared_ptr<Primitive>> LoopToPrimitives(LoopHandle lpid);
		std::vector<std::shared_ptr<Primitive>> VertexToPrimitives(VertexHandle vid);

	signals:
		void elementChanged(ELEMEMNT_TYPE type, ElemHandle elem);

	private slots:
		void updateSolid(SolidHandle Solid);
		void updateFace(FaceHandle Face);
		void updateLoop(LoopHandle Loop);
		void cleanTreeItem(QTreeWidgetItem* item,bool atRoot=false);
		void cleanSelectedPrimitive();
		void delElement(QString typeStr, ElemHandle elem);

		void itemSelectionChanged(); 
		void elemSolidSelectionChanged(QTreeWidgetItem* item, bool selected);
		void elemFaceSelectionChanged(QTreeWidgetItem* item, bool selected);
		void elemLoopSelectionChanged(QTreeWidgetItem* item, bool selected);
		void elemHalfEdgeSelectionChanged(QTreeWidgetItem* item, bool selected);
		void elemVertexSelectionChanged(QTreeWidgetItem* item, bool selected);

		void updatePrimitives(ELEMEMNT_TYPE type, ElemHandle elem); // #PERF1 不能每次重建solid

	private:
		SolidHandle getSolid(ELEMEMNT_TYPE type, ElemHandle elem);
		ElemHandle item2Handle(QTreeWidgetItem* item);

		std::vector<bpSolid*> solids;

		std::map<ElemHandle, bpSolid*> itop_Solid;
		std::map<ElemHandle, bpFace*> itop_Face;
		std::map<ElemHandle, bpLoop*> itop_Loop;
		std::map<ElemHandle, bpHalfEdge*> itop_HalfEdge;
		std::map<ElemHandle, bpVertex*> itop_Vertex;

		std::map<bpSolid*, ElemHandle> ptoi_Solid;
		std::map<bpFace*, ElemHandle> ptoi_Face;
		std::map<bpLoop*, ElemHandle> ptoi_Loop;
		std::map<bpHalfEdge*, ElemHandle> ptoi_HalfEdge;
		std::map<bpVertex*, ElemHandle> ptoi_Vertex;

		std::map<ElemHandle, QTreeWidgetItem*> handle2Item;  // #PERF2 只显示TreeWidget中展开的项，不展开的项先不加入TreeWidget
		
		std::map<ElemHandle, std::vector<std::shared_ptr<Primitive>>> cadPrimMap;
		std::map<QTreeWidgetItem*, std::vector<std::shared_ptr<Primitive>>> cadSelectedPrims;
		//#PERF6 一个element是对应一个primitive还是多个?

		unsigned int curId = 0;
		bool isCleaning;
	};

#undef DEF_ELEMENT_OPERATION
}

