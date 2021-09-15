#include "CadCore.h"
#include "glog/logging.h"
//for the definition of operations method of element list
#define BP_ELEMENT_OP_DEF(ClassName,TypeName,VarName) \
	void bp##ClassName::add##TypeName(bp##TypeName* elem){ \
		if (VarName) \
			VarName->push_back(elem);\
		else VarName = elem;\
	}\
	int bp##ClassName::del##TypeName(bp##TypeName* elem){\
		if(VarName == elem) VarName = elem->Prev()?elem->Prev():elem->Next();\
		for(auto it = VarName->begin();it!=VarName->end();it++){\
			if((*it) == elem) {\
				elem->del(); \
				return 0;\
			}\
		}\
		return -1;\
	}\
	bool bp##ClassName::find##TypeName(bp##TypeName* elem){\
		if(elem && VarName){\
			return VarName->findInList(elem);\
		}\
		else return false;\
	}\
	bp##TypeName* bp##ClassName::get##TypeName(){\
		return VarName;\
	}


namespace mcl{
	bpSolid* mvfs(const Point3f& coord, bpLoop** lp, bpVertex** vtx)
	{
		bpSolid* newSolid;
		bpFace* newFace;
		bpLoop* newLoop;
		bpVertex* newVtx;

		newSolid = new bpSolid;
		newFace = new bpFace;
		newLoop = new bpLoop;
		newVtx = new bpVertex(coord);

		newSolid->addFace(newFace);
		newSolid->addVertex(newVtx);
		newFace->fsolid = newSolid;
		newFace->floops = newLoop;
		newLoop->lface = newFace;

		if (lp) *lp = newLoop;
		if (vtx) *vtx = newVtx;

		return newSolid;
	}

	bpHalfEdge* mev(bpVertex* v1, bpLoop* lp, const Point3f& coord)
	{
		if (!v1 || !lp) {
			assert(0);
			return nullptr;
		}

		bpHalfEdge* he1, *he2;
		bpEdge* eg;
		bpVertex* v2;
		v2 = new bpVertex(coord);

		he1 = new bpHalfEdge;
		he2 = new bpHalfEdge;
		eg = new bpEdge;

		eg->firstHalf = he1;
		he1->adj = he2;
		he1->eloop = lp;
		he1->edge = eg;

		he2->adj = he1;
		he2->eloop = lp;
		he2->edge = eg;

		he1->beginVtx = v1;
		he2->beginVtx = v2;

		v2->firstEdge = he2;

		he1->nxt = he2;
		he2->pev = he1;

		if (lp->firstEg) {
			v1->firstEdge = he1;

			bpHalfEdge* ep = lp->findVertexPevEdge(v1);
			CHECK(ep);
			if (!ep) {
				delete he1;
				delete he2;
				delete eg;
				delete v2;
				return nullptr;
			}
			bpHalfEdge* en = ep->nxt;
			ep->nxt = he1;
			he1->pev = ep;
			he2->nxt = en;
			en->pev = he2;
		}
		else
		{
			he1->pev = he2;
			he2->nxt = he1;
			lp->firstEg = he1;
		}
		auto solid = getSolid(lp);
		solid->addEdge(eg);
		solid->addVertex(v2);
		return he1;
	}

	mcl::bpFace* mef(bpVertex* v1, bpVertex* v2, bpLoop* lp)
	{
		bpHalfEdge* pe1 = lp->findVertexPevEdge(v1);
		bpHalfEdge* pe2 = lp->findVertexPevEdge(v2);
		return mef(pe1, pe2, lp);
	}

	mcl::bpFace* mef(bpHalfEdge* pe1, bpHalfEdge* pe2, bpLoop*lp)
	{
		if (!pe1 || !pe2 || !lp) {
			assert(0);
			return nullptr;
		}
		if (pe1 == pe2) {
			assert(0);
			return nullptr;
		}
		if (!lp->edgeExists(pe1) || !lp->edgeExists(pe2)) {
			assert(0);
			return nullptr;
		}
		bpVertex* v1 = pe1->getEndVtx();
		bpVertex* v2 = pe2->getEndVtx();

		bpEdge* eg = new bpEdge;

		bpHalfEdge* ne1 = new bpHalfEdge;
		bpHalfEdge* ne2 = new bpHalfEdge;
		ne1->adj = ne2;
		ne2->adj = ne1;
		ne1->beginVtx = v1;
		ne2->beginVtx = v2;
		ne1->edge = eg;
		ne2->edge = eg;
		ne1->pev = pe1;
		ne2->pev = pe2;
		ne1->nxt = pe2->nxt;
		ne2->nxt = pe1->nxt;

		eg->firstHalf = ne1;

		pe1->nxt->pev = ne2;
		pe1->nxt = ne1;
		pe2->nxt->pev = ne1;
		pe2->nxt = ne2;

		bpFace* fc = new bpFace;
		bpLoop* nlp = new bpLoop;
		nlp->lface = fc;
		nlp->firstEg = ne1;
		lp->firstEg = ne2;
		ne2->eloop = lp;

		nlp->firstEg->eloop = nlp;
		for (bpHalfEdge* he = nlp->firstEg->nxt; he != nlp->firstEg; he = he->nxt) {
			he->eloop = nlp;
		}

		fc->addLoop(nlp);

		bpSolid* solid = getSolid(lp);
		solid->addEdge(eg);
		solid->addFace(fc);
		fc->fsolid = solid;

		return fc;
	}

	mcl::bpFace* kemr(bpLoop* lp, bpHalfEdge* he1)
	{
		if (!lp || !he1) {
			assert(0);
			return nullptr;
		}
		if (!(lp->edgeExists(he1) && lp->edgeExists(he1->adj))) {
			assert(0);
			return nullptr;
		}
		bpHalfEdge* he2 = he1->adj;
		bpHalfEdge* outerPevHe = he1->pev;
		bpHalfEdge* innerNxtHe = he1->nxt;
		bpHalfEdge* innerPevHe = he2->pev;
		bpHalfEdge* outerNxtHe = he2->nxt;

		outerPevHe->nxt = outerNxtHe;
		outerNxtHe->pev = outerPevHe;
		innerPevHe->nxt = innerNxtHe;
		innerNxtHe->pev = innerPevHe;

		bpLoop* nlp = new bpLoop;
		nlp->lface = lp->lface;
		nlp->firstEg = innerNxtHe;
		lp->firstEg = outerNxtHe;
		lp->lface->addLoop(nlp);

		//修改edge的loop信息
		bpHalfEdge* temphe = innerNxtHe;
		do 
		{
			temphe->eloop = nlp;
			temphe = temphe->nxt;
		} while (temphe!= innerNxtHe);

		getSolid(lp)->delEdge(he1->Edge());
		delete he1->edge;
		delete he1;
		delete he2;

		return lp->lface;
	}

	int kfmrh(bpFace* f1, bpFace* f2)
	{
		if (getSolid(f1) != getSolid(f2)) {
			assert(0);
			return -1;
		}
		if (f2->getLoop()->size() > 1) {
			assert(0);
			return -1;
		} 
		f1->addLoop(f2->getLoop());
		f2->getLoop()->lface = f1;
		getSolid(f2)->delFace(f2);
		delete f2;
		return 0;
	}

	int sweep(bpSolid* solid, bpFace** face, Vector3f dir, Float dist)
	{
		if (!solid || !face) return -1;
		if (!solid->findFace(*face)) return -2;
		if (dist == 0.0)  return -3;
		if (dir.length() == 0.0) return -4;
		if (!((*face)->getLoop())) return -5;

		bpFace* bottomFace = *face;
		//bool first = true;
		dir.normalize();
		Vector3f tran = dir * dist;
		for (auto lp = (*face)->getLoop()->begin(); lp != (*face)->getLoop()->end(); lp++) {

			bpHalfEdge* he = (*lp)->getFirstHalfEdge();
			//bpHalfEdge* firstHe = he;
			bpHalfEdge* lastHe = he->Pev();
			mev(he->getBeginVtx(), *lp, he->getBeginVtx()->getCoord() + tran);
			bpVertex* nowPt = he->Pev()->BeginVtx();
			bpVertex* firstPt = nowPt;

			int kkk = 0;
			while (1) {
				kkk++;
				bpHalfEdge* nextHe = he->Nxt();
				nowPt = mev(nowPt, he->Eloop(), he->getEndVtx()->getCoord() + tran)->getEndVtx();
				mef(he->getEndVtx(),nowPt, he->Eloop());
				he = nextHe;
				if (he == lastHe) break;
			}

			mef(firstPt, nowPt, he->Eloop());
			//bpFace* newFace = 
			//if (first) {
			//	bottomFace = newFace;
			//	first = false;
			//}
			//else {
			//	kfmrh(bottomFace, newFace); //因为所有内loop原本就在一个face中，通过合理使用mef，将所有地面loop保留在了原来的面中
			//}
		}
		*face = bottomFace;
		return 0;
	}

	mcl::bpSolid* createFaceFromLoop(const std::vector<std::vector<Point3f>>& lps_, bpFace** pface, bpFace** prface)
	{
		auto lps = lps_;
		for (auto it = lps.begin(); it != lps.end();) {
			if ((*it).size()<=1) it = lps.erase(it);
			else ++it;
		}
		if (lps.empty()) return nullptr;

		//For Loop 1
		bpLoop* lp;
		bpVertex* vtx;
		bpSolid* solid = mvfs(lps[0][0], &lp, &vtx);
		bpVertex* tempVtx = vtx;
		bpHalfEdge* tempHe;
		for (int i = 1; i < lps[0].size(); i++) {
			tempHe = mev(tempVtx, lp, lps[0][i]);
			tempVtx = tempHe->getEndVtx();
		}
		bpFace* face =lp->Lface();
		bpFace* rface = mef(vtx,tempVtx,lp);  //The reversed face

		bpVertex* firstVtx = tempVtx;
		for (int i = 1; i < lps.size(); i++) {
			//bpVertex* tempVtx2 = firstVtx;
			bpHalfEdge* innerFirstHe;
			bpHalfEdge* firstHe;
			bool first = true;
			bool second = true;
			for (int j = 0; j < lps[i].size(); j++) {
				if (first) {
					firstHe = mev(tempVtx, lp, lps[i][j]);
					tempVtx = firstHe->getEndVtx();
					first = false;
				}
				else if (second) {
					bpHalfEdge* temphe = mev(tempVtx, lp, lps[i][j]);
					tempVtx = temphe->getEndVtx();
					innerFirstHe = temphe->Adj();
					second = false;
				}
				else{
					tempVtx = mev(tempVtx, lp, lps[i][j])->getEndVtx();
				}
			}
			bpFace* newface = mef(innerFirstHe, tempVtx->FirstEdge()->Pev(), lp);
			kemr(firstHe->Eloop(), firstHe);
			kfmrh(rface, newface); //Add loop into the reversed face
			tempVtx = firstVtx;
		}
		if(pface) *pface = face;
		if(prface) *prface = rface;
		return solid;
	}

	BP_ELEMENT_OP_DEF(Solid, Face, sfaces);
	BP_ELEMENT_OP_DEF(Solid, Edge, sedges);
	BP_ELEMENT_OP_DEF(Solid, Vertex, svertics);

	BP_ELEMENT_OP_DEF(Face, Loop, floops);

	void bpLoop::printEdgesDebug()
	{
		bpHalfEdge* he = firstEg;
		DLOG(INFO) << "LOOP: " << this;
		do {
			DLOG(INFO) << he;
			he = he->nxt;
		} while (he != firstEg);
	}

	void bpLoop::printEloopDebug()
	{
		bpHalfEdge* he = firstEg;
		DLOG(INFO) << "LOOP: " << this;
		do {
			DLOG(INFO) << he->eloop;
			he = he->nxt;
		} while (he != firstEg);
	}

	void bpLoop::getVertices(std::vector<bpVertex*>& vs)
	{
		bpHalfEdge* he = firstEg;
		do {
			vs.push_back(he->getBeginVtx());
			he = he->nxt;
		} while (he != firstEg);
	}

}

