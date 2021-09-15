#include "Subdivide.h"
#include <set>
#include <vector>
#include <memory>
#include "ui/MainWindow.h"
#include "ItemManager.h"
#include "Scene.h"
#include "PPolygonMesh.h"
#include "PTriMesh.h"
namespace mcl{
	class SubdivPoint;
	class SubdivHalfEdge;
	class SubdivPolygon;

	struct SubdivPoint
	{
		SubdivPoint(const Point3f& pt, const SubdivHalfEdge* eg=nullptr)
			:p(pt), firstEdge(eg) {}
		SubdivPoint() = default;
		inline int getDegree() const;
		Point3f p;
		const SubdivHalfEdge* firstEdge=nullptr;
		int subPoint = -1; //细分子顶点，用于catmull细分

	};

	struct SubdivHalfEdge
	{
		SubdivHalfEdge(int pt1, int pt2, int face)
			:pt{ pt1,pt2 },fc(face) {}
		SubdivHalfEdge() = default;
		bool operator<(const SubdivHalfEdge& rhs) const{
			if (rhs.pt[0] != pt[0]) return pt[0] < rhs.pt[0];
			return pt[1] < rhs.pt[1];
		}
		int pt[2] = { -1 ,-1 };  //Edge point from pt[0] to pt[1];
		mutable int fc= -1;
		mutable const SubdivHalfEdge* adj = nullptr,* pev = nullptr,* nxt = nullptr;
		//#PERF5 这里两个变量在每个算法中只用其一，有空间效率问题
		mutable const SubdivHalfEdge* subEdge=nullptr; //细分产生的对应子半边
		mutable int subEdgePoint = -1; //细分产生的边点，用于catmull细分
	};

	struct SubdivFace
	{
		std::unique_ptr<const SubdivHalfEdge*[]> lps; //整数数组，每个单元记录每个lp首边
		int nLp;
		Point3f centroid;
		SubdivFace* subFace=nullptr; //细分产生的对应子面;
		int subFacePoint = -1; //子面点，用于catmull细分
	};

	struct SubdivPolygonMesh
	{
		int addPoint(Point3f pt) {
			points.push_back(SubdivPoint(pt));
			return points.size() - 1;
		}

		//SubdivPolygonMesh(int nPlgs, const int* plgLoopNums, const int *loopIndicesNums, const int *indices, int nPts, const Point3f* pts);
		SubdivPolygonMesh(std::shared_ptr<PPolygonMesh> plgmesh);
		std::vector<std::shared_ptr<mcl::Primitive>> createPrimitives();
		SubdivPolygonMesh() = default;
		std::set<SubdivHalfEdge> edges;
		std::vector<SubdivFace> faces;
		std::vector<SubdivPoint> points;
	};

#if 0
	//构造多边形网格拓扑
	SubdivPolygonMesh::SubdivPolygonMesh(int nPlgs, const int* plgLoopNums, const int *loopIndicesNums, const int *indices, int nPts, const Point3f* pts)
	{
		points.resize(nPts);
		for (int i = 0; i < nPts; i++) {
			points[i] = SubdivPoint(pts[i]);
		}
		int lpOffset = 0;
		int idxOffset = 0;
		//遍历每一多边形
		for (int i = 0; i < nPlgs; i++) {
			faces.push_back(SubdivFace());
			SubdivFace& curFace = faces.back();
			int curFaceIdx = faces.size()-1;
			const int &nLp = plgLoopNums[i];
			curFace.nLp = nLp;
			curFace.lps.reset(new const SubdivHalfEdge*[nLp]);
			//用于计算质心
			int ptCount = 0;
			Point3f ptSum(0, 0, 0);
			//遍历每一Loop
			for (int lp = 0; lp < nLp; lp++) {
				const int &nLpIdx = loopIndicesNums[lpOffset+lp];
				DCHECK(nLpIdx >= 3);
				//构造边拓扑
				int lastP = indices[idxOffset + nLpIdx - 1]; //第一个点与最后一个点相连
				const SubdivHalfEdge* lastEdge = nullptr;
				const SubdivHalfEdge* firstEdge = nullptr;
				for (int idx = 0; idx < nLpIdx; idx++) {
					int curP = indices[idxOffset + idx];
					ptCount++;
					ptSum += points[curP].p;
					DCHECK(edges.find(SubdivHalfEdge(lastP, curP, -1)) == edges.end());
					//加入边
					edges.insert(SubdivHalfEdge(lastP,curP, curFaceIdx));
					const SubdivHalfEdge* curEdge = &(*edges.find(SubdivHalfEdge(lastP, curP, curFaceIdx)));
					//给点分配首边
					if (!points[lastP].firstEdge) points[lastP].firstEdge = curEdge;
					//给loop分配首边
					//配对相邻边
					auto it = edges.find(SubdivHalfEdge(curP, lastP,-1));
					if (it != edges.end()) {
						it->adj = curEdge;
						curEdge->adj = &(*it);
					}
					//配对前后边
					if (lastEdge) {
						lastEdge->nxt = curEdge;
						curEdge->pev = lastEdge;
					}
					lastEdge = curEdge;
					if (!firstEdge) {
						firstEdge = curEdge;
						curFace.lps[lp] = curEdge;
					}
					lastP = curP;
				}
				//配对第一边与最后一边
				firstEdge->pev = lastEdge;
				lastEdge->nxt = firstEdge;

				idxOffset += nLpIdx;
			}
			//计算质心
			curFace.centroid = ptSum / ptCount;
			lpOffset += nLp;
		}
	}
#endif

	int SubdivPoint::getDegree() const {
		int count = 0;
		if (!firstEdge) return 0;
		const SubdivHalfEdge* he = firstEdge;
		const SubdivHalfEdge* fhe = he;
		do {
			count++;
			he = he->adj->nxt;
		} while (he != firstEdge);
		return count;
	}

	SubdivPolygonMesh::SubdivPolygonMesh(std::shared_ptr<PPolygonMesh> plgmesh)
	{
		int nPts = plgmesh->getPts().size();
		points.resize(nPts);
		for (int i = 0; i < nPts; i++) {
			points[i] = SubdivPoint(plgmesh->getPts()[i]);
		}
		//遍历每一多边形
		for (int i = 0; i < plgmesh->getPlgs().size(); i++) {
			faces.push_back(SubdivFace());
			SubdivFace& curFace = faces.back();
			int curFaceIdx = faces.size() - 1;
			const int &nLp = plgmesh->getPlgs()[i].lps_.size();
			curFace.nLp = nLp;
			curFace.lps.reset(new const SubdivHalfEdge*[nLp]);
			//用于计算质心
			int ptCount = 0;
			Point3f ptSum(0, 0, 0);
			//遍历每一Loop
			for (int lp = 0; lp < nLp; lp++) {
				const auto& curLp = plgmesh->getPlgs()[i].lps_[lp];
				const int &nLpIdx = curLp.size();
				DCHECK(nLpIdx >= 3);
				//构造边拓扑
				int lastP = curLp[curLp.size()-1]; //第一个点与最后一个点相连
				const SubdivHalfEdge* lastEdge = nullptr;
				const SubdivHalfEdge* firstEdge = nullptr;
				for (int idx = 0; idx < nLpIdx; idx++) {
					int curP = curLp[idx];
					ptCount++;
					ptSum += points[curP].p;
					DCHECK(edges.find(SubdivHalfEdge(lastP, curP, -1)) == edges.end());
					//加入边
					edges.insert(SubdivHalfEdge(lastP, curP, curFaceIdx));
					const SubdivHalfEdge* curEdge = &(*edges.find(SubdivHalfEdge(lastP, curP, curFaceIdx)));
					//给点分配首边
					if (!points[lastP].firstEdge) points[lastP].firstEdge = curEdge;
					//给loop分配首边
					//配对相邻边
					auto it = edges.find(SubdivHalfEdge(curP, lastP, -1));
					if (it != edges.end()) {
						it->adj = curEdge;
						curEdge->adj = &(*it);
					}
					//配对前后边
					if (lastEdge) {
						lastEdge->nxt = curEdge;
						curEdge->pev = lastEdge;
					}
					lastEdge = curEdge;
					if (!firstEdge) {
						firstEdge = curEdge;
						curFace.lps[lp] = curEdge;
					}
					lastP = curP;
				}
				//配对第一边与最后一边
				firstEdge->pev = lastEdge;
				lastEdge->nxt = firstEdge;
			}
			//计算质心
			curFace.centroid = ptSum / ptCount;
		}
	}

	std::vector<std::shared_ptr<mcl::Primitive>> SubdivPolygonMesh::createPrimitives()
	{
		std::vector<Point3f>  pts;
		std::vector<PPolygonMesh::Polygon> plgs;
		for (const auto& pt : points) {
			pts.push_back(pt.p);
		}
		for (const auto& fc : faces) {
			plgs.emplace_back();
			for (int i = 0; i < fc.nLp; i++) {
				plgs.back().lps_.emplace_back();
				const SubdivHalfEdge* he = fc.lps[i];
				auto firstHe = he;
				do {
					plgs.back().lps_.back().push_back(he->pt[0]);
					he = he->nxt;
				} while (firstHe != he);
			}
		}
		auto ret = std::make_shared<PPolygonMesh>(plgs, pts);
		return { ret };
	}

	int dooSabinSubdivPolygonMesh(int id, int nlevels)
	{
		std::shared_ptr<PPolygonMesh> plgs;
		MAIPTR->getItemMng()->getItem(id,plgs);
		if (!plgs) return -1;
		auto output = dooSabinSubdivide(nlevels,plgs);
		MAIPTR->getItemMng()->delItem(plgs);
		MAIPTR->getItemMng()->addItem(output); 
		//MAIPTR->getScene()->update();
		return 0;
	}

	int catmullClarkSubdivPolygonMesh(int id, int nlevels)
	{
		std::shared_ptr<PPolygonMesh> plgs;
		MAIPTR->getItemMng()->getItem(id, plgs);
		if (!plgs) return -1;
		auto output = catmullClarkSubdivide(nlevels, plgs);
		MAIPTR->getItemMng()->delItem(plgs);
		MAIPTR->getItemMng()->addItem(output);
		//MAIPTR->getScene()->update();
		return 0;
	}

	int dooSabinSubdivTriangles(int id, int nlevels)
	{
		std::shared_ptr<PTriMesh> tris;
		MAIPTR->getItemMng()->getItem(id, tris);
		if (!tris) return -1;
		auto output = dooSabinSubdivideTri(nlevels, tris);
		MAIPTR->getItemMng()->delItem(tris);
		MAIPTR->getItemMng()->addItem(output);
		//MAIPTR->getScene()->update();
		return 0;
	}

	int catmullClarkSubdivTriangles(int id, int nlevels)
	{
		std::shared_ptr<PTriMesh> tris;
		MAIPTR->getItemMng()->getItem(id, tris);
		if (!tris) return -1;
		auto output = catmullClarkSubdivideTri(nlevels, tris);
		MAIPTR->getItemMng()->delItem(tris);
		MAIPTR->getItemMng()->addItem(output);
		//MAIPTR->getScene()->update();
		return 0;
	}

	int loopSubdivideTriangles(int id, int nlevels, bool toLimit)
	{
		std::shared_ptr<PTriMesh> tris;
		MAIPTR->getItemMng()->getItem(id, tris);
		if (!tris) return -1;
		auto output = loopSubdivideTri(nlevels, tris, toLimit);
		MAIPTR->getItemMng()->delItem(tris);
		MAIPTR->getItemMng()->addItem(output);
		//MAIPTR->getScene()->update();
		return 0;
	}

	std::vector<std::shared_ptr<mcl::Primitive>> dooSabinSubdivide(int nLevels, std::shared_ptr<PPolygonMesh> plgMesh)
	{
		std::unique_ptr<SubdivPolygonMesh> mesh(new SubdivPolygonMesh(plgMesh));
		//进行nLevel次细分
		for (int i = 0; i < nLevels; i++) {
			std::unique_ptr<SubdivPolygonMesh> subMesh(new SubdivPolygonMesh);
			//预分配数组空间
			subMesh->faces.reserve(mesh->faces.size() * 2);
			subMesh->points.reserve(mesh->points.size());

			//首先生成所有面内部的细分面，点和半边
			for (auto& face : mesh->faces) {
				//生成新面
				subMesh->faces.emplace_back();
				int curFaceIdx = subMesh->faces.size()-1;
				SubdivFace& curFace = subMesh->faces.back();
				face.subFace = &curFace;
				curFace.nLp = face.nLp;
				curFace.lps.reset(new const SubdivHalfEdge *[curFace.nLp]);
				for (int lp = 0; lp < face.nLp; lp++) {
					const SubdivHalfEdge* edge = face.lps[lp];
					const SubdivHalfEdge* firstEdge = edge;
					int firstPt = subMesh->points.size();
					bool first = true;
					const SubdivHalfEdge* firstSubEdge = nullptr;
					const SubdivHalfEdge* lastSubEdge = nullptr;
					//生成细分节点与半边
					do {
						const Point3f& pt = mesh->points[edge->pt[1]].p;	//顶点
						const Point3f& pevPt = (mesh->points[edge->pt[0]].p + mesh->points[edge->pt[1]].p)/2;	//边点
						const Point3f& nxtPt = (mesh->points[edge->pt[1]].p + mesh->points[edge->nxt->pt[1]].p) / 2;	//边点
						Point3f avgPt = (pt + pevPt + nxtPt + face.centroid) / 4;	//面点
						subMesh->points.push_back(SubdivPoint(avgPt));
						if (first) first = false;
						else { //生成半边
							subMesh->edges.insert(SubdivHalfEdge(subMesh->points.size() - 2, subMesh->points.size() - 1, curFaceIdx));
							const SubdivHalfEdge* curSubEdge = &(*subMesh->edges.find(SubdivHalfEdge(subMesh->points.size() - 2, subMesh->points.size() - 1, curFaceIdx)));
							//绑定到父边
							edge->subEdge = curSubEdge;
							subMesh->points[subMesh->points.size() - 2].firstEdge = curSubEdge;
							//分配loop的首边
							curFace.lps[lp] = curSubEdge;
							//绑定半边前后关系
							if (!firstSubEdge) {
								firstSubEdge = curSubEdge;
							}
							else {
								lastSubEdge->nxt = curSubEdge;
								curSubEdge->pev = lastSubEdge;
							}
							lastSubEdge = curSubEdge;
						}
						edge = edge->nxt;
					} while (edge != firstEdge);
					//连接首末节点
					subMesh->edges.insert(SubdivHalfEdge(subMesh->points.size()-1, firstPt, curFaceIdx));
					const SubdivHalfEdge* curSubEdge = &(*subMesh->edges.find(SubdivHalfEdge(subMesh->points.size() - 1, firstPt, curFaceIdx)));
					curSubEdge->nxt = firstSubEdge;
					firstSubEdge->pev = curSubEdge;
					curSubEdge->pev = lastSubEdge;
					lastSubEdge->nxt = curSubEdge;
					firstEdge->subEdge = curSubEdge; 
					subMesh->points[subMesh->points.size() - 1].firstEdge = curSubEdge;
				}
			}

			//遍历原始所有的原始网格点，生成所有脊子面与点子面，连接拓扑关系
			for (int pt = 0; pt < mesh->points.size(); pt++) {
				const SubdivHalfEdge* firstEdge = mesh->points[pt].firstEdge;
				const SubdivHalfEdge* edge = mesh->points[pt].firstEdge;
				//生成顶点子面
				subMesh->faces.emplace_back();
				int vtxSubFaceIdx = subMesh->faces.size()-1;
				subMesh->faces[vtxSubFaceIdx].nLp = 1;
				subMesh->faces[vtxSubFaceIdx].lps.reset(new const SubdivHalfEdge *[1]);
				subMesh->faces[vtxSubFaceIdx].lps[0] = nullptr;

				const SubdivHalfEdge* lastVtxSubEdge = nullptr;
				const SubdivHalfEdge* firstVtxSubEdge = nullptr;
				//遍历一个原始网格点周围的边
				do {
					const SubdivHalfEdge* subedge = edge->subEdge;
					if (!subedge->adj) { //该子边对应的脊面没有连接，连接脊面
						const SubdivHalfEdge* oppoSubEdge = edge->adj->subEdge;

						subMesh->faces.emplace_back();
						SubdivFace& newSubFace = subMesh->faces.back();
						newSubFace.nLp = 1;
						newSubFace.lps.reset(new const SubdivHalfEdge *[1]);

						//生成脊面四个边，并且连接拓扑
						subMesh->edges.insert(SubdivHalfEdge(subedge->pt[1], subedge->pt[0], subMesh->faces.size() - 1));
						const SubdivHalfEdge* edgeAdj = &*subMesh->edges.find(SubdivHalfEdge(subedge->pt[1], subedge->pt[0], subMesh->faces.size() - 1));
						edgeAdj->adj = subedge;
						subedge->adj = edgeAdj;

						subMesh->edges.insert(SubdivHalfEdge(oppoSubEdge->pt[1], oppoSubEdge->pt[0], subMesh->faces.size() - 1));
						const SubdivHalfEdge* oppoEdgeAdj = &*subMesh->edges.find(SubdivHalfEdge(oppoSubEdge->pt[1], oppoSubEdge->pt[0], subMesh->faces.size() - 1));
						oppoEdgeAdj->adj = oppoSubEdge;
						oppoSubEdge->adj = oppoEdgeAdj;

						subMesh->edges.insert(SubdivHalfEdge(subedge->pt[0], oppoSubEdge->pt[1], subMesh->faces.size() - 1));
						const SubdivHalfEdge* edgeAdjNxt = &*subMesh->edges.find(SubdivHalfEdge(subedge->pt[0], oppoSubEdge->pt[1], subMesh->faces.size() - 1));

						subMesh->edges.insert(SubdivHalfEdge(oppoSubEdge->pt[0], subedge->pt[1], subMesh->faces.size() - 1));
						const SubdivHalfEdge* oppoEdgeAdjNxt = &*subMesh->edges.find(SubdivHalfEdge(oppoSubEdge->pt[0], subedge->pt[1], subMesh->faces.size() - 1));

						edgeAdj->nxt = edgeAdjNxt;
						edgeAdjNxt->pev = edgeAdj;

						oppoEdgeAdj->pev = edgeAdjNxt;
						edgeAdjNxt->nxt = oppoEdgeAdj;

						edgeAdj->pev = oppoEdgeAdjNxt;
						oppoEdgeAdjNxt->nxt = edgeAdj;

						oppoEdgeAdj->nxt = oppoEdgeAdjNxt;
						oppoEdgeAdjNxt->pev = oppoEdgeAdj;

						newSubFace.lps[0] = edgeAdj;
					}
					const SubdivHalfEdge* adjEdge = edge->subEdge->adj->nxt; //与顶点面的边相邻的边

					subMesh->edges.insert(SubdivHalfEdge(adjEdge->pt[1], adjEdge->pt[0], vtxSubFaceIdx));
					//生成顶点子面的一条边
					const SubdivHalfEdge* vtxSubEdge = &*subMesh->edges.find(SubdivHalfEdge(adjEdge->pt[1], adjEdge->pt[0], vtxSubFaceIdx));
					vtxSubEdge->adj = adjEdge;
					adjEdge->adj = vtxSubEdge;

					//连接拓扑关系
					if (!firstVtxSubEdge) firstVtxSubEdge = vtxSubEdge;
					else {
						lastVtxSubEdge->nxt = vtxSubEdge;
						vtxSubEdge->pev = lastVtxSubEdge;
					}
					lastVtxSubEdge = vtxSubEdge;
					//移动到下一条顶点边
					edge = edge->pev->adj;
				} while (edge != firstEdge);
				subMesh->faces[vtxSubFaceIdx].lps[0] = firstVtxSubEdge;

				lastVtxSubEdge->nxt = firstVtxSubEdge;
				firstVtxSubEdge->pev = lastVtxSubEdge;
			}

			//计算所有子面的质心
			//以下这段是bug代码，可以制造出石头效果
			//for (auto& face : subMesh->faces) {
			//	Point3f ptSum(0, 0, 0);
			//	int ptCount=0;
			//	for (int i = 0; i < face.nLp; i++) {
			//		auto edge = face.lps[i];
			//		auto firstEdge = edge;
			//		do {
			//			ptSum += subMesh->points[edge->pt[0]].p;
			//			ptCount++;
			//		} while (edge != firstEdge);
			//	}
			//	face.centroid = ptSum / ptCount;
			//}
			for (auto& face : subMesh->faces) {
				Point3f ptSum(0, 0, 0);
				int ptCount = 0;
				for (int i = 0; i < face.nLp; i++) {
					auto edge = face.lps[i];
					auto firstEdge = edge;
					do {
						ptSum += subMesh->points[edge->pt[0]].p;
						ptCount++;
						edge = edge->nxt;
					} while (edge != firstEdge);
				}
				face.centroid = ptSum / ptCount;
			}

			//父mesh替换为子mesh
			mesh = std::move(subMesh);
		}

		//#TODO2 将mesh移到极限点
		auto ret = mesh->createPrimitives();

		for (const auto& prim : ret) {
			prim->copyAttribute(plgMesh);
		}

		return ret;
	}

	std::vector<std::shared_ptr<mcl::Primitive>> catmullClarkSubdivide(int nLevels, std::shared_ptr<PPolygonMesh> plgMesh)
	{
		std::unique_ptr<SubdivPolygonMesh> mesh(new SubdivPolygonMesh(plgMesh));

		for (int i = 0; i < nLevels; i++) {
			std::unique_ptr<SubdivPolygonMesh> subMesh(new SubdivPolygonMesh);
			//预分配数组空间
			subMesh->faces.reserve(mesh->faces.size() * 3);
			subMesh->points.reserve(mesh->points.size() * 2);

			//计算面点
			for (auto& face : mesh->faces) {
				face.subFacePoint = subMesh->points.size();
				subMesh->addPoint(face.centroid);
			}

			//计算边点
			for (const auto& he : mesh->edges) {
				if (he.subEdgePoint == -1) {
					//获取面点
					const Point3f& fcpt1 = mesh->faces[he.fc].centroid; 
					const Point3f& fcpt2 = mesh->faces[he.adj->fc].centroid;
					Point3f edgept = (fcpt1 + fcpt2 + mesh->points[he.pt[0]].p + mesh->points[he.pt[1]].p) / 4;
					he.subEdgePoint=subMesh->addPoint(edgept);
					he.adj->subEdgePoint = he.subEdgePoint;
				}
			}

			//计算顶点
			for (auto& pt : mesh->points) {
				const SubdivHalfEdge* he = pt.firstEdge;
				const SubdivHalfEdge* firstHe = he;
				Point3f sumOfFacePt(0, 0, 0);
				Point3f sumOfEdgePt(0, 0, 0);
				int count = 0;
				do {
					sumOfEdgePt += subMesh->points[he->subEdgePoint].p;
					sumOfFacePt += subMesh->points[mesh->faces[he->fc].subFacePoint].p;
					count++;
					he = he->adj->nxt;
				} while (he!=firstHe);

				int n = pt.getDegree();
				pt.subPoint = subMesh->addPoint(((sumOfFacePt + 2 * sumOfEdgePt) / count + (n - 3)*pt.p) / n);
			}

			//为每个原始面，生成子面
			for (auto&fc: mesh->faces) {
				for (int i = 0; i < fc.nLp; i++) {
					const SubdivHalfEdge* he = fc.lps[i]; 
					const SubdivHalfEdge* firsthe = he;

					do 
					{
						int ep1 = he->subEdgePoint;
						int ep2 = he->nxt->subEdgePoint;
						int pp = mesh->points[he->pt[1]].subPoint;
						int fp = mesh->faces[he->fc].subFacePoint;

						subMesh->faces.emplace_back();
						SubdivFace& newFace = subMesh->faces.back();
						newFace.nLp = 1;

						subMesh->edges.insert(SubdivHalfEdge(fp, ep1, subMesh->faces.size() - 1));
						const SubdivHalfEdge* inhe1 = &*subMesh->edges.find(SubdivHalfEdge(fp, ep1, subMesh->faces.size() - 1));

						subMesh->edges.insert(SubdivHalfEdge(ep1, pp, subMesh->faces.size() - 1));
						const SubdivHalfEdge* outhe1 = &*subMesh->edges.find(SubdivHalfEdge(ep1, pp, subMesh->faces.size() - 1));

						subMesh->edges.insert(SubdivHalfEdge(pp, ep2, subMesh->faces.size() - 1));
						const SubdivHalfEdge* inhe2 = &*subMesh->edges.find(SubdivHalfEdge(pp, ep2, subMesh->faces.size() - 1));

						subMesh->edges.insert(SubdivHalfEdge(ep2, fp, subMesh->faces.size() - 1));
						const SubdivHalfEdge* outhe2 = &*subMesh->edges.find(SubdivHalfEdge(ep2, fp, subMesh->faces.size() - 1));

						//建立拓扑关系
						if (!subMesh->points[fp].firstEdge) subMesh->points[fp].firstEdge = inhe1;
						if (!subMesh->points[ep1].firstEdge) subMesh->points[ep1].firstEdge = outhe1;
						if (!subMesh->points[pp].firstEdge) subMesh->points[pp].firstEdge = inhe2;
						if (!subMesh->points[ep2].firstEdge) subMesh->points[ep2].firstEdge = outhe2;

						inhe1->nxt = outhe1;
						outhe1->pev = inhe1;

						outhe1->nxt = inhe2;
						inhe2->pev = outhe1;

						inhe2->nxt = outhe2;
						outhe2->pev = inhe2;

						outhe2->nxt = inhe1;
						inhe1->pev = outhe2;

						newFace.lps.reset(new const SubdivHalfEdge*[1]{ inhe1 });

						//计算质心
						const Point3f&	pep1 = subMesh->points[ep1].p;
						const Point3f&	pep2 = subMesh->points[ep2].p;
						const Point3f&	ppp = subMesh->points[pp].p;
						const Point3f&	pfp = subMesh->points[fp].p;
						newFace.centroid = (pep1 + pep2 + ppp + pfp) / 4;

						he = he->nxt;
					} while (he!=firsthe);
				}
			}

			//建立边的adj关系
			for (auto it = subMesh->edges.begin(); it != subMesh->edges.end();it++){
				if(it->adj) continue;
				auto adjit = subMesh->edges.find(SubdivHalfEdge(it->pt[1], it->pt[0], -1));
				DCHECK(adjit != subMesh->edges.end());
				it->adj = &*adjit;
				adjit->adj = &*it;
			}
			
			mesh = std::move(subMesh);
		}
		//#TODO2 将mesh移到极限点
		auto ret = mesh->createPrimitives();
		for (const auto& prim : ret) {
			prim->copyAttribute(plgMesh);
		}
		return ret;
	}

	std::vector<std::shared_ptr<mcl::Primitive>> dooSabinSubdivideTri(int nLevels, std::shared_ptr<PTriMesh> triMesh)
	{
		std::shared_ptr<PPolygonMesh> plgmesh = std::make_shared<PPolygonMesh>(triMesh);
		if (!plgmesh) return std::vector<std::shared_ptr<mcl::Primitive>>();
		return dooSabinSubdivide(nLevels, plgmesh);
	}

	std::vector<std::shared_ptr<mcl::Primitive>> catmullClarkSubdivideTri(int nLevels, std::shared_ptr<PTriMesh> triMesh)
	{
		std::shared_ptr<PPolygonMesh> plgmesh = std::make_shared<PPolygonMesh>(triMesh);
		if (!plgmesh) return std::vector<std::shared_ptr<mcl::Primitive>>();
		return catmullClarkSubdivide(nLevels, plgmesh);
	}

	//--------------------------------------------------
	//
	// Loop subdivide method definition
	//
	//--------------------------------------------------

	inline int pevIdx(int idx) { return (idx + 2) % 3; }
	inline int nxtIdx(int idx) { return (idx + 1) % 3; }

	/*Structures for loop subdivide*/
	struct LPVertex;
	struct LPFace; 
	struct LPEdge;
	struct LPTopology;

	typedef unsigned int handle_lpface;
	typedef unsigned int handle_lpvertex;
#define handle_null (unsigned int(-1))
	
	struct LPVertex
	{
		LPVertex(Point3f coord = Point3f(0, 0, 0))
			:pt(coord) {}
		bool isBoundary(const LPTopology& tp) const;
		int valence(const LPTopology& tp) const; //获取点的度
		bool isRegular(const LPTopology& tp) const;
		void getVtxAround(LPTopology& tp, std::unique_ptr<Point3f[]>& pts, int& n);
		handle_lpvertex child(LPTopology& tp) const;
		int boundAdjVertices(LPTopology& tp , handle_lpvertex& nxtv, handle_lpvertex& pevv) const; //获取边界点的相邻边界点

		Point3f pt;
		handle_lpface firstFc;
		handle_lpvertex index = -1;
	};

	//边作为中介来建立面之间的相邻关系
	struct LPEdge {
		LPEdge(handle_lpvertex v1 = handle_null, handle_lpvertex v2 = handle_null) :v{ std::min(v1,v2),std::max(v2,v1) } {}; //将顶点指针升序排列，保证构造时参数顺序无关性
		int indexInFace(LPFace* face) const;
		bool operator<(const LPEdge& rhs) const {
			if (v[0] != rhs.v[0]) return v[0] < rhs.v[0];
			return v[1] < rhs.v[1];
		}
		void connectFaces(LPTopology& tp) const; //在fc成员两个元素都被赋值的情况下，连接fc中两个面
		handle_lpvertex child(LPTopology& tp) const;
		mutable int index;
		handle_lpvertex v[2];
		mutable handle_lpface fc[2];
	};

	struct LPFace
	{
		LPFace(handle_lpvertex v0 = handle_null, handle_lpvertex v1 = handle_null, handle_lpvertex v2 = handle_null)
			:v{ v0,v1,v2 }, adj{ handle_null,handle_null,handle_null }{}
		int vidx(handle_lpvertex vtx) const;
		handle_lpface nxtFace(handle_lpvertex vtx) const; //返回某个顶点对应的相邻面
		handle_lpface pevFace(handle_lpvertex vtx) const; //返回某个顶点对应的前一个面
		handle_lpvertex nxtVtx(handle_lpvertex vtx) const; //返回某个顶点上一个顶点
		handle_lpvertex pevVtx(handle_lpvertex vtx) const; //返回某个顶点下一个顶点

		handle_lpface adj[3]; //face adj[i] shares vertex v[i] v[(i+1)%3] with this face
		handle_lpvertex v[3];
	};

	struct LPTopology
	{
		LPTopology(const std::vector<Point3f>& pts, const std::vector<int>& indices);
		LPTopology() = default;
		void setupVtxIndex();
		void setupEdgeIndex();
		const LPEdge* getEdge(handle_lpvertex v0, handle_lpvertex v1); //寻找边，若不存在该边，则创建并返回
		void addEdgeForFace(handle_lpface face, const LPEdge& edge); //为面face添加新边edge，并分配edge的fc属性，如果edge已经存在，则连接face与edge的另外一个面
		std::shared_ptr<PTriMesh> toTriMesh();
		int nfc, npt;
		std::unique_ptr<LPFace[]> fcs;
		std::unique_ptr<LPVertex[]> vtxs;
		std::set<LPEdge> edges;
	};

	LPTopology::LPTopology(const std::vector<Point3f>& pts, const std::vector<int>& indices)
	{
		nfc = indices.size() / 3;
		npt = pts.size();
		fcs.reset(new LPFace[nfc]);
		vtxs.reset(new LPVertex[pts.size()]);
		for (int i = 0; i < npt;i++) {
			vtxs[i].pt = pts[i];
		}

		setupVtxIndex();

		for (int i = 0; i < nfc; i++) {
			fcs[i].v[0] = indices[3 * i];
			fcs[i].v[1] = indices[3 * i + 1];
			fcs[i].v[2] = indices[3 * i + 2];
			vtxs[indices[3 * i]].firstFc = i;
			vtxs[indices[3 * i + 1]].firstFc = i;
			vtxs[indices[3 * i + 2]].firstFc = i;
		}

		for (handle_lpface fc = 0; fc < nfc; fc++) {
			for (int i = 0; i < 3; i++) {
				handle_lpvertex v0 = fcs[fc].v[i];
				handle_lpvertex v1 = fcs[fc].v[nxtIdx(i)];
				LPEdge edge(v0, v1);
				auto it = edges.find(edge);
				if (it == edges.end()) {
					//没有找到相邻面，新建边
					edge.fc[0] = fc;
					edges.insert(edge);
				}
				else {
					//找到相邻面，维护相邻面信息
					LPFace* adjface = &fcs[it->fc[0]];
					adjface->adj[it->indexInFace(adjface)] = fc;
					fcs[fc].adj[i] = it->fc[0];
					it->fc[1] = fc;
				}
			}
		}
		setupEdgeIndex();
	}

	inline bool LPVertex::isBoundary(const LPTopology& tp) const
	{
		handle_lpface fc = firstFc;
		bool isbd = false;
		do
		{
			fc = tp.fcs[fc].nxtFace(index);
			if (fc == handle_null) {  //如果遍历到null的三角相邻面，说明遇到边界
				isbd = true;
				break;
			}
		} while (fc != firstFc);
		return isbd;
	}

	inline int LPVertex::valence(const LPTopology& tp) const
	{
		handle_lpface fc = firstFc;
		bool isbd = false;
		int count = 0;
		do
		{
			++count;
			fc = tp.fcs[fc].nxtFace(index);
			if (fc == handle_null) {  //如果遍历到null的三角相邻面，说明遇到边界
				isbd = true;
				break;
			}
		} while (fc != firstFc);
		if (!isbd) return count;
		//碰到边界后，向另外一个方向进行计数
		fc = tp.fcs[firstFc].pevFace(index);
		while (fc != handle_null) {
			count++;
			fc = tp.fcs[fc].pevFace(index);
		}
		return count + 1;  //边界点的度数为相邻面数+1
	}

	void LPVertex::getVtxAround(LPTopology& tp, std::unique_ptr<Point3f[]>& pts, int& n)
	{
		n = valence(tp);
		pts.reset(new Point3f[n]);
		int count = 0;
		if (isBoundary(tp)) {
			handle_lpface fc = firstFc;
			do {
				handle_lpface nxtfc = tp.fcs[fc].pevFace(index);
				if (nxtfc == handle_null) {  //如果遍历到null的三角相邻面，说明遇到边界
					break;
				}
				fc = nxtfc;
			} while (1);
			
			pts[count] = tp.vtxs[tp.fcs[fc].pevVtx(index)].pt;
			++count;

			while (fc != handle_null) {
				fc = tp.fcs[fc].nxtFace(index);
				pts[count] = tp.vtxs[tp.fcs[fc].nxtVtx(index)].pt;
				++count;
			}
		}
		else {
			handle_lpface fc = firstFc;
			do
			{
				++count;
				pts[count] = tp.vtxs[tp.fcs[fc].nxtVtx(index)].pt;
				fc = tp.fcs[fc].nxtFace(index);
			} while (fc != firstFc);
		}
	}

	mcl::handle_lpvertex LPVertex::child(LPTopology& tp) const
	{
		return index;
	}

	inline int LPVertex::boundAdjVertices(LPTopology& tp, handle_lpvertex& nxtv, handle_lpvertex& pevv) const
	{
		handle_lpface fc = firstFc;
		bool isbd = false;
		do {
			handle_lpface nxtfc = tp.fcs[fc].nxtFace(index);
			if (nxtfc == handle_null) {  //如果遍历到null的三角相邻面，说明遇到边界
				isbd = true;
				break;
			}
			fc = nxtfc;
		} while (fc != firstFc);
		DCHECK(isbd);
		nxtv = tp.fcs[fc].nxtVtx(index);

		fc = firstFc;
		isbd = false;
		do {
			handle_lpface nxtfc = tp.fcs[fc].pevFace(index);
			if (nxtfc == handle_null) {  //如果遍历到null的三角相邻面，说明遇到边界
				isbd = true;
				break;
			}
			fc = nxtfc;
		} while (fc != firstFc);
		DCHECK(isbd);
		pevv = tp.fcs[fc].pevVtx(index);

		return 0;
	}

	void LPTopology::setupVtxIndex()
	{
		for (int i = 0; i < npt; i++) {
			vtxs[i].index = i;
		}
	}

	void LPTopology::setupEdgeIndex()
	{
		//设置LPEdge的index成员
		int count = 0;
		for (auto& edge : edges)
		{
			edge.index = count;
			++count;
		}
	}

	const mcl::LPEdge* LPTopology::getEdge(handle_lpvertex v0, handle_lpvertex v1)
	{
		auto it = edges.find(LPEdge(v0, v1));
		if (it == edges.end()) {
			edges.insert(LPEdge(v0, v1));
			return &*edges.find(LPEdge(v0, v1));
		}
		return &*it;
	}

	void LPTopology::addEdgeForFace(handle_lpface face, const LPEdge& edge)
	{
		auto it = edges.find(edge);
		if (it == edges.end()) {
			edges.insert(edge);
			edges.find(edge)->fc[0] = face;
		}
		else {
			it->fc[1] = face;
			it->connectFaces(*this);
		}
	}

	std::shared_ptr<mcl::PTriMesh> LPTopology::toTriMesh()
	{
		std::vector<Point3f> ptvec;
		std::vector<int> trivec;
		for (int i = 0; i < npt;i++) {
			ptvec.push_back(vtxs[i].pt);
		}
		for (int i = 0; i < nfc; i++) {
			trivec.push_back(fcs[i].v[0]);
			trivec.push_back(fcs[i].v[1]);
			trivec.push_back(fcs[i].v[2]);
		}
		return std::make_shared<PTriMesh>(trivec, ptvec);
	}

	inline int LPEdge::indexInFace(LPFace* face) const {
		int v1 = face->vidx(v[0]);
		int v2 = face->vidx(v[1]);
		if (v1 == pevIdx(v2)) return v1;
		DCHECK(v1 == nxtIdx(v2));
		return v2;
	}

	inline void LPEdge::connectFaces(LPTopology& tp) const
	{
		DCHECK(fc[0] != handle_null && fc[1] != handle_null);
		LPFace* fc0 = &tp.fcs[fc[0]];
		LPFace* fc1 = &tp.fcs[fc[1]];
		fc0->adj[indexInFace(fc0)] = fc[1];
		fc1->adj[indexInFace(fc1)] = fc[0];
		return;
	}

	mcl::handle_lpvertex LPEdge::child(LPTopology& tp) const
	{
		//子边点在子顶点之后，按父边顺序排列
		return tp.npt + index;
	}

	inline int LPFace::vidx(handle_lpvertex vtx) const {
		if (v[0] == vtx) return 0;
		if (v[1] == vtx) return 1;
		DCHECK(v[2] == vtx);
		return 2;
	}

	inline mcl::handle_lpface LPFace::nxtFace(handle_lpvertex vtx) const
	{
		return adj[vidx(vtx)];
	}

	inline mcl::handle_lpface LPFace::pevFace(handle_lpvertex vtx) const
	{
		return adj[pevIdx(vidx(vtx))];
	}

	inline mcl::handle_lpvertex LPFace::nxtVtx(handle_lpvertex vtx) const
	{
		return v[nxtIdx(vidx(vtx))];
	}

	inline mcl::handle_lpvertex LPFace::pevVtx(handle_lpvertex vtx) const
	{
		return v[pevIdx(vidx(vtx))];
	}

	inline constexpr Float boundVtxBeta() {
		return 0.125;
	}

	inline Float innerVtxBeta(int valence) {
		if (valence == 3) return (3. / 16.);
		else return (3. / 8. / valence);
	}

	inline Float limitInnerVtxBeta(int valence) {
		return 1 / (valence + 3. / (8. * innerVtxBeta(valence)));
	}

	inline constexpr Float limitBoundVtxBeta() {
		return 0.2;
	}

	std::vector<std::shared_ptr<mcl::Primitive>> loopSubdivideTri(int nLevels, std::shared_ptr<PTriMesh> triMesh, bool toLimit)
	{
		std::unique_ptr<LPTopology> msh(new LPTopology(Point3f::fromFloatVec(triMesh->getPts()), triMesh->getIndices()));
		Transform localtrans_ = triMesh->localTransform();

		for (int ilevel = 0; ilevel < nLevels; ilevel++) {
			std::unique_ptr<LPTopology> newmsh(new LPTopology);
			newmsh->npt = msh->npt + msh->edges.size();
			newmsh->nfc = msh->nfc * 4;
			newmsh->fcs.reset(new LPFace[newmsh->nfc]);
			newmsh->vtxs.reset(new LPVertex[newmsh->npt]);

			newmsh->setupVtxIndex();

			/*生成所有顶点*/
			//处理子顶点
			for (handle_lpvertex ipt = 0; ipt < msh->npt; ipt++) {
				LPVertex* pt = &msh->vtxs[ipt];
				//处理边界点
				if (pt->isBoundary(*msh)) {
					handle_lpvertex v0;
					handle_lpvertex v1;
					pt->boundAdjVertices(*msh, v0, v1);
					newmsh->vtxs[pt->child(*msh)].pt = (1 - 2 * boundVtxBeta())*pt->pt + boundVtxBeta()*(msh->vtxs[v0].pt + msh->vtxs[v1].pt);
				}
				//处理内部点
				else {
					double beta = innerVtxBeta(pt->valence(*msh));
					handle_lpface fc = pt->firstFc;
					Point3f ptsum(0,0,0);
					int count = 0;
					do
					{
						ptsum += msh->vtxs[msh->fcs[fc].nxtVtx(pt->index)].pt;
						++count;
						fc = msh->fcs[fc].nxtFace(pt->index);
					} while (fc != pt->firstFc);
					newmsh->vtxs[pt->child(*msh)].pt = (1 - count * beta) * pt->pt + beta * ptsum;
				}
			}
			//处理子边点
			for (const auto& edge : msh->edges) {
				//边界边
				if (msh->vtxs[edge.v[0]].isBoundary(*msh)) {
					DCHECK(msh->vtxs[edge.v[1]].isBoundary(*msh));
					Point3f subPt = (msh->vtxs[edge.v[0]].pt + msh->vtxs[edge.v[1]].pt) / 2;
					newmsh->vtxs[edge.child(*msh)].pt = subPt;
				}
				//内部边
				else
				{
					LPFace* face1 = &msh->fcs[edge.fc[0]];
					LPFace* face2 = &msh->fcs[edge.fc[1]];
					handle_lpvertex v[4];
					v[0] = edge.v[0];
					v[1] = edge.v[1];
					v[2] = face1->v[pevIdx(edge.indexInFace(face1))];
					v[3] = face2->v[pevIdx(edge.indexInFace(face2))];
					Point3f subPt = (msh->vtxs[v[2]].pt + msh->vtxs[v[3]].pt) / 8;
					subPt += (msh->vtxs[v[0]].pt + msh->vtxs[v[1]].pt) * 3 / 8;
					newmsh->vtxs[edge.child(*msh)].pt = subPt;
				}
			}
			/*生成顶点结束*/

			for (int ifc = 0; ifc < msh->nfc; ifc++) {
				LPFace* pfc = &msh->fcs[ifc];
				handle_lpface subf[4];
				//确定子面的索引, subf[3]是中心子面
				for (int i = 0; i < 4; i++) {
					subf[i] = ifc * 4 + i;
				}
				//设置子顶点的face属性
				for (int i = 0; i < 3; i++) {
					newmsh->vtxs[msh->vtxs[pfc->v[i]].child(*msh)].firstFc = subf[i];
				}
				//获取父面的三条边
				const LPEdge* eg[3];
				for (int i = 0; i < 3; i++) {
					eg[i] = &*msh->edges.find(LPEdge(pfc->v[i], pfc->v[nxtIdx(i)]));
				}
				//创建中央边
				const LPEdge* subeg[3];
				for (int i = 0; i < 3; i++) {
					subeg[i] = newmsh->getEdge(eg[i]->child(*msh), eg[pevIdx(i)]->child(*msh));
					subeg[i]->fc[0] = subf[i];
					subeg[i]->fc[1] = subf[3];
				}
				//设置中央面
				for (int i = 0; i < 3; i++) {
					handle_lpvertex ep = eg[i]->child(*msh); //第i个边点
					newmsh->fcs[subf[3]].v[i] = ep;
					newmsh->vtxs[ep].firstFc = subf[3];
					//子面的相邻关系
					newmsh->fcs[subf[3]].adj[pevIdx(i)] = subf[i];
					newmsh->fcs[subf[i]].adj[1] = subf[3];
				}
				//设置角上3个面
				for (int i = 0; i < 3; i++) {
					LPFace* fc = &newmsh->fcs[subf[i]];
					fc->v[0] = msh->vtxs[pfc->v[i]].child(*msh);
					fc->v[1] = eg[i]->child(*msh);
					fc->v[2] = eg[pevIdx(i)]->child(*msh);

					newmsh->addEdgeForFace(subf[i], LPEdge(fc->v[0], fc->v[1]));
					newmsh->addEdgeForFace(subf[i], LPEdge(fc->v[0], fc->v[2]));
				}
			}

			newmsh->setupEdgeIndex();
			msh = std::move(newmsh);
		}

		//计算极限点属性
		if (toLimit) {
			std::vector<Point3f> limitPt(msh->npt); //暂存极限点位置
			std::vector<Normal3f> normals(msh->npt); //各个点的极限法向
			for (int i = 0; i < msh->npt; i++) {
				LPVertex* vtx = &msh->vtxs[i];
				Vector3f T, S; //切向，副切向
				if (vtx->isBoundary(*msh)) {
					//计算边界点极限位置
					handle_lpvertex v0, v1;
					vtx->boundAdjVertices(*msh, v0, v1);
					limitPt[i] = vtx->pt * (1 - 2 * limitBoundVtxBeta()) + limitBoundVtxBeta() * (msh->vtxs[v0].pt + msh->vtxs[v1].pt);

					//计算边界点极限法向
					int n;
					std::unique_ptr<Point3f[]> pts;
					vtx->getVtxAround(*msh, pts, n);
					S = pts[n - 1] - pts[0];
					//根据点度数不同，有不同的权重
					if (n == 2) {
						T = Vector3f(vtx->pt * -2 + pts[0] + pts[1]);
					}
					if (n == 3) {
						T = Vector3f(vtx->pt * -1 + pts[1]);
					}
					if (n == 4) {
						T = Vector3f(vtx->pt * -2 + (pts[0] + pts[3])*-1 + (pts[1] + pts[2]) * 2);
					}
					else {
						Float theta = PI / (n - 1);
						Float sinval = std::sin(theta);
						Float cosval = std::cos(theta);
						T = Vector3f((pts[0] + pts[n - 1]) * sinval);
						for (int j = 1; j < n - 1; j++) {
							T += Vector3f(pts[i] * ((2 * cosval - 2)*std::sin(theta*i)));
						}
					}
				}
				else {
					int n = vtx->valence(*msh);
					double beta = limitInnerVtxBeta(n);
					handle_lpface fc = vtx->firstFc;
					Point3f ptsum(0, 0, 0);
					int count = 0;
					do
					{
						const Point3f& pt = msh->vtxs[msh->fcs[fc].nxtVtx(vtx->index)].pt;
						ptsum += pt;
						S += Vector3f(std::cos(2 * PI*count / n) * pt);
						T += Vector3f(std::sin(2 * PI*count / n) * pt);
						++count;
						fc = msh->fcs[fc].nxtFace(vtx->index);
					} while (fc != vtx->firstFc);
					limitPt[i] = (1 - count * beta) * vtx->pt + beta * ptsum;
				}
				normals[i] = Normal3f(S.cross(T));
			}

			//#TODO3 将点的极限法向赋予PTriMesh，在绘制时用该法向实现光滑化效果

			for (int i = 0; i < msh->npt; i++) {
				msh->vtxs[i].pt = limitPt[i];
			}
		}
		auto ret = msh->toTriMesh();
		ret->copyAttribute(triMesh);
		return std::vector<std::shared_ptr<mcl::Primitive>>{ret};
	}
#undef  handle_error
}