#include "CadInterface.h"
#include <QTreeWidgetItem>
#include "CadCore.h"
#include "MainWindow.h"
#include "realtime/Scene.h"
#include "primitives/PPolygon.h"
#include "Primitive.h"
#include "primitives/PPolygonMesh.h"
#include "primitives/PPoint.h"
#include "primitives/PLines.h"
#include "ItemManager.h"
namespace mcl{

	CadInterface::CadInterface()
	{
		connect(this, SIGNAL(elementChanged(ELEMEMNT_TYPE, ElemHandle)), this, SLOT(updatePrimitives(ELEMEMNT_TYPE, ElemHandle)));
		isCleaning = false;
	}

	int CadInterface::mvfs(const Point3f& coord)
	{
		bpLoop* nlp;
		bpVertex* nvtx;
		bpSolid* nsld = mcl::mvfs(coord, &nlp,&nvtx);
		addFace(nlp->Lface());
		addLoop(nlp);
		addVertex(nvtx);
		int nsid = addSolid(nsld);

		updateSolid(nsid);
		emit elementChanged(SOLID, nsid);
		return nsid;
	}

	CadInterface::HalfEdgeHandle CadInterface::mev(VertexHandle vertex, LoopHandle lp, const Point3f& coord)
	{
		bpLoop* plp = findLoop(lp);
		bpVertex* pVtx = findVertex(vertex);
		if (!plp || !pVtx) return ErrorHandle;
		bpHalfEdge* nhe = mcl::mev(pVtx, plp, coord);
		addVertex(nhe->getEndVtx());
		addHalfEdge(nhe->Adj());
		int heid = addHalfEdge(nhe);

		updateLoop(lp);
		emit elementChanged(LOOP, lp);
		return heid;
	}

	mcl::CadInterface::FaceHandle CadInterface::mef(VertexHandle v1, VertexHandle v2, LoopHandle lp)
	{
		bpVertex* pv1 = findVertex(v1);
		bpVertex* pv2 = findVertex(v2);
		bpLoop* plp = findLoop(lp);
		if (!pv1 || !pv2 || !plp) return ErrorHandle;
		int fcid = addFace(mcl::mef(pv1, pv2, plp));
		addHalfEdge(plp->getFirstHalfEdge());
		addHalfEdge(plp->getFirstHalfEdge()->Adj());
		addLoop(plp->getFirstHalfEdge()->Adj()->Eloop());

		SolidHandle sld = findSolid(mcl::getSolid(plp));
		updateSolid(sld);
		emit elementChanged(SOLID, sld);
		return fcid;
	}

	int CadInterface::kfmrh(FaceHandle f1, FaceHandle f2)
	{
		bpFace* pf1 = findFace(f1);
		bpFace* pf2 = findFace(f2);
		SolidHandle sld1 = findSolid(mcl::getSolid(pf1));
		SolidHandle sld2 = findSolid(mcl::getSolid(pf2));
		int ret = mcl::kfmrh(pf1, pf2);

		CHECK(!ret);
		delFace(f2);
		if (sld1 == sld2) {
			updateSolid(sld1);
			emit elementChanged(SOLID, sld1);
		}
		else {
			updateSolid(sld2);
			updateFace(f1);
			emit elementChanged(SOLID, sld2);
			emit elementChanged(FACE, f1);
		}
		return ret;
	}

	mcl::CadInterface::FaceHandle CadInterface::kemr(LoopHandle lp, HalfEdgeHandle he)
	{
		bpLoop* plp = findLoop(lp);
		bpHalfEdge* phe = findHalfEdge(he);
		HalfEdgeHandle he1 = he;
		HalfEdgeHandle he2 = findHalfEdge(phe->Adj());
		CHECK(plp&&phe);
		FaceHandle fcid = findFace(mcl::kemr(plp, phe));
		if (fcid != ErrorHandle) {
			delHalfEdge(he1);
			delHalfEdge(he2);
			updateFace(fcid);
			emit elementChanged(FACE,fcid);
		}
		return fcid;
	}

	int CadInterface::sweep(SolidHandle solid, FaceHandle face, Vector3f dir, Float dist)
	{
		bpSolid* psld = findSolid(solid);
		bpFace* pfc = findFace(face);
		CHECK(psld&&pfc);
		int ret = mcl::sweep(psld, &pfc, dir, dist);
		updateSolid(solid);
		emit elementChanged(SOLID, solid);
		return ret;
	}

	void CadInterface::updateSolid(SolidHandle solid)
	{
		bpSolid* psld = findSolid(solid);
		CHECK(solid!=ErrorHandle);

		//update tree widget items
		QTreeWidgetItem* sldItem;
		auto it = handle2Item.find(solid);
		if (it == handle2Item.end()) {
			sldItem = new QTreeWidgetItem({ "Solid",QString::number(solid) });
			MAIPTR->getCadTreeWidget()->addTopLevelItem(sldItem);
			handle2Item[solid] = sldItem;
		}
		else {
			sldItem = it->second;
			cleanTreeItem(sldItem,1);
		}

		for (auto it = psld->getFace()->begin(); it != psld->getFace()->end(); it++) {
			FaceHandle fcid;
			fcid = findFace(*it);
			if (fcid == ErrorHandle) fcid=addFace(*it);
			updateFace(fcid);
		}
		for (auto it = psld->getVertex()->begin(); it != psld->getVertex()->end(); it++) {
			VertexHandle vid;
			vid = findVertex(*it);
			if (vid == ErrorHandle) vid = addVertex(*it);
		}
	}

	void CadInterface::updateFace(FaceHandle face)
	{
		bpFace* pfc = findFace(face);
		CHECK(pfc);

		//update tree widget items
		QTreeWidgetItem* faceItem;
		auto it = handle2Item.find(face);
		if (it == handle2Item.end()) {
			faceItem = new QTreeWidgetItem({ "Face",QString::number(face) });
			handle2Item[findSolid(mcl::getSolid(pfc))]->addChild(faceItem);
			handle2Item[face] = faceItem;
		}
		else {
			faceItem = it->second;
			cleanTreeItem(faceItem,1);
		}

		for (auto it = pfc->getLoop()->begin(); it != pfc->getLoop()->end(); it++) {
			LoopHandle lp;
			lp = findLoop(*it);
			if (lp == ErrorHandle) lp = addLoop(*it);
			updateLoop(lp);
		}
	}

	void CadInterface::updateLoop(LoopHandle loop)
	{
		bpLoop* plp = findLoop(loop);
		CHECK(plp);

		//update tree widget items
		QTreeWidgetItem* lpitem;
		auto it = handle2Item.find(loop);
		if (it == handle2Item.end()) {
			lpitem = new QTreeWidgetItem({ "Loop",QString::number(loop) });
			handle2Item[findFace(plp->Lface())]->addChild(lpitem);
			handle2Item[loop] = lpitem;
		}
		else {
			lpitem = it->second;
			cleanTreeItem(lpitem,1);
		}

		auto he = plp->getFirstHalfEdge();
		if (!plp->getFirstHalfEdge()) { //如果loop没有半边，说明loop是mvfs生成的
			QTreeWidgetItem* vitem = new QTreeWidgetItem({ "Vertex",QString::number(findVertex(mcl::getSolid(plp)->getVertex())) });
			handle2Item[findVertex(mcl::getSolid(plp)->getVertex())] = vitem;
			handle2Item[loop]->addChild(vitem);
			return;
		}

		auto fhe = he;
		do 
		{
			HalfEdgeHandle heid;
			heid = findHalfEdge(he);
			if (heid == ErrorHandle) heid = addHalfEdge(he);
			VertexHandle vid;
			vid = findVertex(he->getBeginVtx());
			if (vid == ErrorHandle) vid = addVertex(he->getBeginVtx());

			QTreeWidgetItem* heitem = new QTreeWidgetItem({ "HalfEdge",QString::number(heid) });
			handle2Item[heid] = heitem;
			handle2Item[loop]->addChild(heitem);
			QTreeWidgetItem* vitem = new QTreeWidgetItem({ "Vertex",QString::number(vid) });
			//handle2Item[vid] = vitem; //一个vertex对应多个item，所以暂时不做映射
			handle2Item[loop]->addChild(vitem);
			he = he->Nxt();
		} while (he != fhe);
	}

	void CadInterface::cleanTreeItem(QTreeWidgetItem* item, bool atRoot)
	{
		if (atRoot) isCleaning = true;
		ElemHandle elemid = item->text(1).toInt();
		if (!atRoot) { 
			if(handle2Item.find(elemid) != handle2Item.end()) handle2Item.erase(elemid);
			item->setSelected(false);

			auto it = cadSelectedPrims.find(item);
			if (it != cadSelectedPrims.end()) {
				MAIPTR->getScene()->delPrimitives(it->second);
				cadSelectedPrims.erase(it);
			}

			delElement(item->text(0), item->text(1).toInt());
		}
		for (int i = item->childCount() - 1; i >=0 ; i--) {
			cleanTreeItem(item->child(i));
		}
		if (!atRoot)
			delete item;
		if (atRoot) isCleaning = false;
	}

	void CadInterface::cleanSelectedPrimitive()
	{
		for(const auto& vec: cadSelectedPrims)
		MAIPTR->getScene()->delPrimitives(vec.second);
		cadSelectedPrims.clear();
	}

	void CadInterface::delElement(QString typeStr, ElemHandle elem)
	{
		if (typeStr == "Solid")
			delSolid(elem);
		if (typeStr == "Face")
			delFace(elem);
		if (typeStr == "Loop")
			delLoop(elem);
		if (typeStr == "Vertex")
			delVertex(elem);
		if (typeStr == "HalfEdge")
			delHalfEdge(elem);
	}

	void CadInterface::itemSelectionChanged() // #PERF2 目前是选择item改变后，先清空选择Primitive，再构建之，不能增量构建
	{
		if (isCleaning) return;
		auto tree = MAIPTR->getCadTreeWidget();
		cleanSelectedPrimitive();
		auto list = tree->selectedItems();
		for (const auto& item : list) {
			ElemHandle elem = item2Handle(item);
			QString type = item->text(0);
			if (type == "Solid")
				elemSolidSelectionChanged(item, 1);
			if (type == "Face")
				elemFaceSelectionChanged(item, 1);
			if (type == "Loop")
				elemLoopSelectionChanged(item, 1);
			if (type == "Vertex")
				elemVertexSelectionChanged(item, 1);
			if (type == "HalfEdge")
				elemHalfEdgeSelectionChanged(item, 1);
		}
		MAIPTR->getScene()->update();
		MAIPTR->getScene()->repaint();
	}

#define DEF_ELEM_SELECTION_CHANGE_FUNC(Type) \
	void CadInterface::elem##Type##SelectionChanged(QTreeWidgetItem* item, bool selected){ \
		auto it = cadSelectedPrims.find(item);		\
		\
		if (!selected) {\
			if (it == cadSelectedPrims.end()) return;\
			MAIPTR->getScene()->delPrimitives(it->second);\
			cadSelectedPrims.erase(it);\
		} \
		else{ \
			if (it != cadSelectedPrims.end()) return;\
			auto prims = ##Type##ToPrimitives(item2Handle(item));\
			for(auto& prim:prims) prim->setSelected(true);\
			cadSelectedPrims[item] = prims;\
			MAIPTR->getScene()->addPrimitives(prims);\
		}\
	}

	DEF_ELEM_SELECTION_CHANGE_FUNC(Solid);
	DEF_ELEM_SELECTION_CHANGE_FUNC(Face);
	DEF_ELEM_SELECTION_CHANGE_FUNC(Loop);
	DEF_ELEM_SELECTION_CHANGE_FUNC(HalfEdge);
	DEF_ELEM_SELECTION_CHANGE_FUNC(Vertex);

#undef DEF_ELEM_SELECTION_CHANGE_FUNC

	void CadInterface::updatePrimitives(ELEMEMNT_TYPE type, ElemHandle elem)
	{
		SolidHandle sld;
		sld = getSolid(type, elem);
		auto it = cadPrimMap.find(sld);
		if (it != cadPrimMap.end()) {
			for (const auto& prim : it->second) {
				MAIPTR->getScene()->delPrimitive(prim->id());
			}
		}
		cadPrimMap[sld] = SolidToPrimitives(sld);
		for (const auto& prim : cadPrimMap[sld]) {
			MAIPTR->getScene()->addPrimitives(cadPrimMap[sld]);
		}

		//每次更新，重置选择
		MAIPTR->cadTreeUnselectedAll();
		//MAIPTR->getScene()->update();
	}

	mcl::CadInterface::SolidHandle CadInterface::getSolid(ELEMEMNT_TYPE type, ElemHandle elem)
	{
		switch (type)
		{
		case mcl::CadInterface::SOLID:
			return elem;
			break;
		case mcl::CadInterface::FACE:
			return findSolid(mcl::getSolid(findFace(elem)));
			break;
		case mcl::CadInterface::LOOP:
			return findSolid(mcl::getSolid(findLoop(elem)));
			break;
		case mcl::CadInterface::HALFEDGE:
			CHECK(0);
			break;
		case mcl::CadInterface::VERTEX:
			CHECK(0);
			break;
		default:
			CHECK(0);
			break;
		}
		return ErrorHandle;
	}

	mcl::CadInterface::ElemHandle CadInterface::item2Handle(QTreeWidgetItem* item)
	{
		return item->text(1).toInt();
	}

	mcl::CadInterface::SolidHandle CadInterface::createFaceFromLoop(const std::vector<std::vector<Point3f>>& lps)
	{
		if (lps.empty()) return ErrorHandle;
		bpSolid* sld = mcl::createFaceFromLoop(lps);
		if (!sld) return ErrorHandle;
		SolidHandle sldid = addSolid(sld);
		updateSolid(sldid);
		emit elementChanged(SOLID, sldid);
		return sldid;
	}


	void CadInterface::removeSolid(SolidHandle sld)
	{
		auto it = cadPrimMap.find(sld);
		if (it != cadPrimMap.end()) {
			for (const auto& prim : it->second) {
				MAIPTR->getScene()->delPrimitive(prim->id());
			}
			cadPrimMap.erase(it);
		}

		QTreeWidgetItem* sldItem = handle2Item.find(sld)->second;
		cleanTreeItem(sldItem, 1);
		MAIPTR->getCadTreeWidget()->takeTopLevelItem(MAIPTR->getCadTreeWidget()->indexOfTopLevelItem(sldItem));
		delSolid(sld);
		//MAIPTR->getScene()->update();
	}

	int CadInterface::solidToPolygonMesh(SolidHandle sld)
	{
		std::vector<Point3f> pts;
		std::vector<PPolygonMesh::Polygon> plgs;
		std::map<bpVertex*, int> ptid;
		int count = 0;

		bpSolid* psld = findSolid(sld);

		for (auto it = psld->getVertex()->begin(); it != psld->getVertex()->end(); it++) {
			bpVertex* pt = *it;
			pts.push_back(pt->getCoord());
			if (ptid.find(pt) == ptid.end()) {
				ptid[pt] = count;
				++count;
			}
		}
		
		for (auto it = psld->getFace()->begin(); it != psld->getFace()->end(); it++) {
			bpFace* fc = *it;
			plgs.emplace_back();
			for (auto lpit = fc->Floops()->begin(); lpit != fc->Floops()->end(); lpit++) {
				plgs.back().lps_.emplace_back();
				auto& lp = plgs.back().lps_.back();
				std::vector<bpVertex*> vtcs;
				(*lpit)->getVertices(vtcs);
				for (const auto& vt : vtcs) {
					DCHECK(ptid.find(vt) != ptid.end());
					lp.push_back(ptid[vt]);
				}
			}
		}

		std::shared_ptr<mcl::PPolygonMesh> plg(new PPolygonMesh(plgs, pts));
		MAIPTR->getItemMng()->addItem(plg);
		removeSolid(sld);

		return 0;
	}

	std::vector<mcl::CadInterface::HalfEdgeHandle> CadInterface::getHalfEdgesOfLoop(LoopHandle lp)
	{
		bpLoop* plp = findLoop(lp);
		CHECK(plp);
		bpHalfEdge* fhe = plp->getFirstHalfEdge();
		std::vector<mcl::CadInterface::HalfEdgeHandle> result;
		bpHalfEdge* he = fhe;
		do {
			result.push_back(findHalfEdge(he));
			CHECK(result.back() != ErrorHandle);
			he = he->Nxt();
		} while (he != fhe);
		return result;
	}

	std::vector<std::shared_ptr<mcl::Primitive>> CadInterface::SolidToPrimitives(SolidHandle solidid)
	{
		auto solid = findSolid(solidid);
		bpFace* fc = solid->getFace();
		std::vector<std::shared_ptr<mcl::Primitive>> plgs;
		for (auto it = fc->begin(); it != fc->end(); it++) {
			auto newps = FaceToPrimitives(findFace(*it));
			plgs.insert(plgs.end(), newps.begin(), newps.end());
		}
		return plgs;
	}

	std::vector<std::shared_ptr<mcl::Primitive>> CadInterface::FaceToPrimitives(FaceHandle faceid)
	{
		bpFace* fc = findFace(faceid);
		std::vector<std::shared_ptr<mcl::Primitive>> plgs;
		std::vector<std::vector<int>> lps;
		std::vector<Point3f> pts;
		for (auto lpit = fc->Floops()->begin(); lpit != fc->Floops()->end(); lpit++) {
			bpHalfEdge* he = (*lpit)->getFirstHalfEdge();
			bpHalfEdge* firstHe = he;
			if (!he) {
				auto newps = LoopToPrimitives(findLoop(*lpit));
				plgs.insert(plgs.end(),newps.begin(),newps.end());
				continue;
			}
			lps.emplace_back();
			do
			{
				lps.back().push_back(pts.size());
				pts.push_back(he->getBeginVtx()->getCoord());
				he = he->Nxt();
			} while (he != firstHe);
		}
		if (lps.size()) plgs.emplace_back(new mcl::PPolygonMesh(std::vector<PPolygonMesh::Polygon>{PPolygonMesh::Polygon{lps}}, pts));
		return plgs;
	}

	std::vector<std::shared_ptr<mcl::Primitive>> CadInterface::HalfEdgeToPrimitives(HalfEdgeHandle heid)
	{
		auto he = findHalfEdge(heid);
		std::vector<Point3f> pts;
		pts.push_back(he->getBeginVtx()->getCoord());
		pts.push_back(he->getEndVtx()->getCoord());
		return { std::shared_ptr<mcl::Primitive>(new PLines(pts,false)) };
	}

	std::vector<std::shared_ptr<mcl::Primitive>> CadInterface::LoopToPrimitives(LoopHandle lpid)
	{
		auto lp = findLoop(lpid);
		auto he = lp->getFirstHalfEdge();

		if (!he) {
			return { std::shared_ptr<mcl::Primitive>(new PPoint(mcl::getSolid(lp)->getVertex()->getCoord())) }; // loop没有边，说明是mvfs生成的
		}

		auto fhe = he;
		std::vector<Point3f> pts;

		do {
			pts.push_back(he->getBeginVtx()->getCoord());
			he = he->Nxt();
		} while (fhe != he);
		pts.push_back(he->getBeginVtx()->getCoord());
		return { std::shared_ptr<mcl::Primitive>(new PLines(pts,true)) };
	}

	std::vector<std::shared_ptr<mcl::Primitive>> CadInterface::VertexToPrimitives(VertexHandle vid)
	{
		auto v = findVertex(vid);
		return { std::shared_ptr<mcl::Primitive>(new PPoint(v->getCoord())) };
	}

}