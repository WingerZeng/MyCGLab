#include "SoftRenderer.h"
#include "realtime/Scene.h"
#include "Light.h"
#include "utilities.h"
#include "Primitive.h"
#include "primitives/PPolygon.h"
#include "primitives/PPolygonMesh.h"
#include "primitives/PTriMesh.h"
#include "RasPolygon.h"
namespace mcl {
	SoftRenderer::SoftRenderer(Scene* scene)
		:scene_(scene),film_(new Film(scene->width(),scene->height()))
	{
		bindViewportToScene = true;
		//connect(scene.get(), &Scene::updated, this, [this](Scene::UpdateReason reason) {
		//	this->sceneUpdated(reason);
		//	});
		//sceneUpdated(Scene::ALL);
	}

	SoftRenderer::SoftRenderer(Scene* scene, int px, int py)
		:scene_(scene), film_(new Film(px,py))
	{
		bindViewportToScene = false;
	}

	SoftRenderer::~SoftRenderer()
	{

	}

	int SoftRenderer::paint(PaintInfomation* info /*= nullptr*/)
	{
		sceneUpdated(Scene::ALL);
		isPainting = true;
		Clock<ZBUFFER_SWEEPLINE>::clear();
		Clock<ZBUFFER_SWEEPLINE_CULL>::clear();
		Clock<ZBUFFER_SWEEPLINE_SHADER>::clear();
		Clock<ZBUFFER_SWEEPLINE>::start();
		int nPixels = film_->getSize().x() * film_->getSize().y();

		Transform viewTrans(scene_->getCamera().getViewMatrix());
		Transform projTrans(scene_->getCamera().getProjMatrix());
		Transform totTrans = film_->deviceToScreen()*projTrans * viewTrans;
		Transform invTotTran = totTrans.inverse();

		Vector3f screenxInWorld = invTotTran(Vector3f(1, 0, 0));
		Vector3f screenzInWorld = invTotTran(Vector3f(0, 0, 1));

		int npy = film_->getSize().y();
		int npx = film_->getSize().x();

		/* 构造多边形数据 */
		const auto& prims = scene_->getPrimitives();
		std::list<std::unique_ptr<RasPolygon>> polygons;
		Bound2i screenBound(Point2i(0, 0), Point2i(npx, npy));
		for (const auto& prim : prims) {
			if (prim.second->dynamic_tag() == PPolygonMesh::tag()) {
				auto pplg = std::dynamic_pointer_cast<PPolygonMesh>(prim.second);
				const auto& pts = pplg->getPts();
				const auto& plgs = pplg->getPlgs();

				Primitive* pprim = prim.second.get();
				for (const auto& plg : plgs) {
					polygons.emplace_back(new RasPolygon);

					int ret = createRasPolygon(polygons.back().get(), pprim, totTrans,invTotTran, screenBound, plg.lps_[0],pts);

					if (ret < 0) {
						paintFinished();
						return -1;
					}
					if (ret > 0) {
						polygons.pop_back();
					}
				}
			}
		}
		//按照miny升序排列多边形
		polygons.sort([](std::unique_ptr<RasPolygon> const & lhs, std::unique_ptr<RasPolygon> const & rhs) {
			return getPlgMinY(lhs.get()) < getPlgMinY(rhs.get());
			});

		Clock<ZBUFFER_SWEEPLINE_CULL>::start();
		/* 扫描线算法 */
		std::unique_ptr<FragInfo[]> zbuffer(new FragInfo[npx]);
		std::list<std::unique_ptr<RasPolygon>> activePolygons;
		std::list<std::unique_ptr<RasPolygon>>::iterator activePolygonPos = polygons.begin(); // 记录第一个未活化的多边形位置
		std::set<RasBiEdge*> sbedge; //维护所有边对
		for (int y = 0; y < npy; y++) {
			for (int x = 0; x < npx; x++) {
				zbuffer[x].z = maxDepth;
			}

			//添加新的活化多边形
			while (activePolygonPos!= polygons.end()&&getPlgMinY((*activePolygonPos).get()) == y) {
				activePolygons.push_back(std::move(*activePolygonPos));
				++activePolygonPos;
			}

			//遍历每个活化多边形，逐个绘制
			for (const auto& curPlg : activePolygons) {
				FragInfo info;
				info.normal = curPlg->normal;
				info.color = curPlg->prim->color();
				//更新活化边
				while (getPlgMinY(curPlg.get()) == y) {
					//#TEST 检查splice是否只移除一个
					curPlg->activeEdges.splice(curPlg->activeEdges.end(), curPlg->edges, curPlg->edges.begin());
				}
				curPlg->activeEdges.sort([](const std::unique_ptr<RasPlgEdge>& e1, const std::unique_ptr<RasPlgEdge>& e2) {
					return e1->curX < e2->curX;
					});
				int nae = curPlg->activeEdges.size();
				auto it = curPlg->activeEdges.begin();
				if (nae % 2 == 1) {

					//paintFinished();
					//return -2; //无法配对边对
				}
				//配对边对，将区间信息存入buffer
				for (int i = 0; i < nae - 1; i += 2) {
					const auto& e1 = *(it++);
					const auto& e2 = *(it++);
					RasBiEdge be(e1.get(), e2.get());
					info.z = be.egs[0]->curZ;
					info.worldPos = invTotTran(Point3f(Float(be.egs[0]->curX), Float(y), Float(info.z)));

					for (int x = be.egs[0]->curX; x < be.egs[1]->curX; x++) {
						//info.worldPos = invTotTran(Point3f(Float(x),Float(y),Float(info.z)));
						if (zbuffer[x].z > info.z) zbuffer[x] = info;
						info.z += be.dzdx();
						info.worldPos = info.worldPos + screenxInWorld * 1 + screenzInWorld * be.dzdx();
					}
				}
			}

			//绘制扫描线上的像素
			Clock<ZBUFFER_SWEEPLINE_CULL>::stop();
			Clock<ZBUFFER_SWEEPLINE_SHADER>::start();
			for (int x = 0; x < npx; x++) {
				fragShader(zbuffer[x], x, y);
			}
			Clock<ZBUFFER_SWEEPLINE_SHADER>::stop();
			Clock<ZBUFFER_SWEEPLINE_CULL>::start();

			for (auto it = activePolygons.begin(); it != activePolygons.end();) {
				if ((*it)->maxy == y) {
					it = activePolygons.erase(it);
				}
				else
					++it;
			}

			//增量更新扫描线数据
			for (const auto& curPlg : activePolygons) {
				for (auto it = curPlg->activeEdges.begin(); it != curPlg->activeEdges.end();) {
					//移除过期活化边
					if ((*it)->getLowerBound() == y) {
						it = curPlg->activeEdges.erase(it);
					}
					//更新已有活化边
					else {
						(*it)->moveToNxtLine();
						it++;
					}
				}
			}
		}
		Clock<ZBUFFER_SWEEPLINE_CULL>::stop();

		Clock<ZBUFFER_SWEEPLINE>::stop();
		paintFinished();

		return 0;
	}

	void SoftRenderer::sceneUpdated(Scene::UpdateReason reason)
	{
		if (isPainting) {
			needUpdate = Scene::UpdateReason(needUpdate | reason);
		}
		else {
			if (reason & Scene::CAMERA) {
				invViewMatrix = scene_->getCamera().getViewMatrix();
				invViewMatrix = invViewMatrix.inverse();
			}
			if (reason & Scene::CAMERA) {
				invViewMatrix = scene_->getCamera().getViewMatrix();
				invViewMatrix = invViewMatrix.inverse();
			}
			if (reason & Scene::VIEWPORT && bindViewportToScene) {
				film_.reset(new Film(scene_->width(), scene_->height()));
			}
		}
	}

	void SoftRenderer::fragShader(const FragInfo& info, int px, int py)
	{
		if (info.z == maxDepth) {
			film_->setPixelColor(px, py, bgcolor);
		}
		else {
			//film_->setPixelColor(px, py, (info.normal + Normal3f(1))/2);
			Color3f result(0);
			for (const auto& light : lights) {
				result += light->ambient();
				Vector3f lightvec = Normalize(light->position() - info.worldPos);
				result += light->emission()*(std::clamp<Float>(info.normal.dot(Normal3f(lightvec)), 0, 1));
			}
			result *= info.color;
			if (result == Color3f(0)) {
				std::cout << 1;
			}
			film_->setPixelColor(px, py, result);
		}
	}

	void SoftRenderer::paintFinished()
	{
		isPainting = false;
		if (needUpdate != Scene::NONE) {
			sceneUpdated(needUpdate);
		}
	}

	
	int SoftRenderer::createRasPolygon(RasPolygon* curpolygon,Primitive* prim, const Transform& viewProjMat, const Transform& invTotTrans, const Bound2i& screenBound, const std::vector<int>& lp, const std::vector<PType3f>& pts)
	{
		curpolygon->maxy = screenBound.pMin().y() - 1;
		curpolygon->miny = screenBound.pMax().y() + 1;
		curpolygon->prim = prim;
		//背面剔除
		curpolygon->normal = prim->localTransform()(calPolygonNormal(lp, pts));

		Transform totalTrans = viewProjMat * prim->localTransform();

		if (viewProjMat(curpolygon->normal).z() > 0) {
			return 1;
		}

		//#PERF3 此处多次计算一个点的屏幕空间坐标
		//检查点是否超出屏幕
		//#TODO2 暂时不支持超出屏幕范围的多边形，需要完善裁剪功能
		for (const auto& node : lp) {
			const Point3f& coord = pts[node];
			Point3f tranedNode = totalTrans(coord);
			if (tranedNode.y() < screenBound.pMin().y() || tranedNode.y() >= screenBound.pMax().y()) {
				return -1; //#TODO2 暂时不支持超出屏幕范围的多边形，需要完善裁剪功能
			}
			if (tranedNode.x() < screenBound.pMin().x()  || tranedNode.x() >= screenBound.pMax().x()) {
				return -1;
			}
			curpolygon->maxy = std::max(curpolygon->maxy, int(tranedNode.y()));
			curpolygon->miny = std::min(curpolygon->miny, int(tranedNode.y()));
		}

		//顶点处理的逻辑
		int size = lp.size();
		auto next = [size](int i) {
			return (i + size + 1) % size;
		};
		auto pev = [size](int i) {
			return (i + size - 1) % size;
		};
		auto processNode = [&pts, &lp, &next, &pev, &totalTrans, &invTotTrans](int pevidx, RasPlgNode& pevn, RasPlgNode& nxtn) {
			int nxtidx = next(pevidx);
			nxtn = RasPlgNode(pts[lp[nxtidx]], totalTrans);
			pevn = RasPlgNode(pts[lp[pevidx]], totalTrans);
			if (pevn.scoord.y() < nxtn.scoord.y()) {
				int nnxtidx = next(nxtidx);
				RasPlgNode nnxtn(pts[lp[nnxtidx]], totalTrans);
				while (nxtn.scoord.y() == nnxtn.scoord.y()) {
					nnxtidx = next(nnxtidx);
					nnxtn = RasPlgNode(pts[lp[nnxtidx]], totalTrans);
				}
				//处理非极值点（nxtNode）
				if (nxtn.scoord.y() < nnxtn.scoord.y()) {
					RasPlgEdge tempe(pevn, nxtn);
					Point3f tempv(nxtn.scoord.x(), nxtn.scoord.y(), nxtn.depth);
					tempv += Point3f(-tempe.dxdy, -1, -tempe.dzdy);
					nxtn.scoord = Point2i(tempv.x(), nxtn.scoord.y() - 1);
					nxtn.depth = tempv.z();
					nxtn.wcoord = invTotTrans(tempv);
				}
			}
			else if (pevn.scoord.y() > nxtn.scoord.y()) {
				int ppevtidx = pev(pevidx);
				RasPlgNode ppevn(pts[lp[ppevtidx]], totalTrans);
				while (pevn.scoord.y() == ppevn.scoord.y()) {
					ppevtidx = pev(ppevtidx);
					ppevn = RasPlgNode(pts[lp[ppevtidx]], totalTrans);
				}
				//处理非极值点（pevNode）
				if (ppevn.scoord.y() > pevn.scoord.y()) {
					RasPlgEdge tempe(pevn, nxtn);
					Point3f tempv(pevn.scoord.x(), pevn.scoord.y(), pevn.depth);
					tempv += Point3f(-tempe.dxdy, -1, -tempe.dzdy);
					pevn.scoord = Point2i(int(tempv.x()), pevn.scoord.y()-1);
					pevn.depth = tempv.z();
					pevn.wcoord = invTotTrans(tempv);
				}
			}
			else if (pevn.scoord.y() == nxtn.scoord.y()) //去除原有水平边
				return -1;
			
			//如果非极值点处理后变为水平边，需要特殊处理
			if (pevn.scoord.y() == nxtn.scoord.y()) {
				Point2i newSPt = (pevn.scoord + nxtn.scoord) / 2;
				Point3f newWPt = (pevn.wcoord + nxtn.wcoord) / 2;
				Float newdepth = (pevn.depth + nxtn.depth) / 2;

				pevn.scoord = newSPt;
				DCHECK(pevn.scoord.y() == nxtn.scoord.y() && pevn.scoord.y() == newSPt.y());
				nxtn.scoord = newSPt;

				pevn.wcoord = newWPt;
				nxtn.wcoord = newWPt;

				pevn.depth = newdepth;
				nxtn.depth = newdepth;
			}
			return 0; 
		};

		//创建各边
		for (int i = 0; i < lp.size(); i++) {
			RasPlgNode pevNode, nxtNode;
			int ret = processNode(i, pevNode, nxtNode);
			if (ret) {
				continue; //去除水平边
			}
			curpolygon->edges.emplace_back(new RasPlgEdge(pevNode, nxtNode));
		}

		if (curpolygon->edges.empty()) return 1;
		else {
			//按maxy值升序排列各边 
			curpolygon->edges.sort([](const std::unique_ptr<RasPlgEdge>& e1, const std::unique_ptr<RasPlgEdge>& e2) {
				return e1->getUpperBound() < e2->getUpperBound();
				});
		}

		return 0;
	}

}