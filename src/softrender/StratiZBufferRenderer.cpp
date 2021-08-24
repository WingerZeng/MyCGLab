#include "StratiZBufferRenderer.h"
#include "utilities.h"
#include "realtime/Scene.h"
#include "camera.h"
#include "primitives/PPolygonMesh.h"
#include "Primitive.h"

namespace mcl {
	StratiZBuffer::StratiZBuffer(int size)
	{
		DCHECK(size > 0);
		totsize = size;
		nlayer = 0;
		while (size) {
			size = size >> 1;
			nlayer++;
		}

		size = 1;
		for (int i = 0; i < nlayer; i++) {
			layers.emplace_back(size*size);
			for (auto& cell : layers.back()) {
				cell.maxdepth = MaxFloat;
				cell.mindepth = MaxFloat;
				cell.info.z = SoftRenderer::maxDepth;
				cell.mask = (1 << 4) - 1;
			}
			size <<= 1;
		}

	}


	bool StratiZBuffer::isCull(const Bound3f& bound)
	{
		Bound2i ibd(Bound3i::Bound3(bound));
		double mindepth = bound.pMin().z();
		double maxdepth = bound.pMax().z();
		Point2i nodeTodo(Point2i(0, 0));
		//遍历直到找到最小包含节点
		int flayer = 0;
		for (flayer = 0; flayer < nlayer-1; flayer++) {
			bool find = false;
			for (int i = 0; i < 4; i++) {
				Point2i childpos;
				child(nodeTodo, i, childpos);
				//子节点包含对象，则继续遍历子节点
				if (cellBound(flayer + 1, childpos.x(), childpos.y()).include(ibd)) {
					nodeTodo = childpos;
					find = true;
					break;
				}
			}
			//如果四个子节点都不包含对象，说明父节点为最小包含节点
			if (!find) {
				break;
			}
		}

		//从最小包含节点开始遍历
		std::vector<Point2i> nodesTodo{ nodeTodo };
		for (int layer = flayer; layer < nlayer; layer++) {
			bool success = true;
			std::vector<Point2i> nextNodes;
			nextNodes.reserve(4);
			for (const auto& node : nodesTodo) {
				//如果最大深度小于该节点最小深度，则肯定需要更新
				if (maxdepth < cell(layer, node).mindepth) {
					return false;
				}

				//如果有任何一个节点最大深度大于目标最小深度，则测试失败
				//如果该节点被目标完全包含，并且判定失败，则说明不能剔除；否则说明该内部节点判定成功
				if (ibd.include(cellBound(layer, node))) {
					if (mindepth < cell(layer, node).maxdepth) {
						return false;
					}
				}
				//否则，若判定失败，则继续判定子节点，否则说明该内部根节点判定成功
				else {
					if (mindepth < cell(layer, node).maxdepth) {
						success = false;

						for (int i = 0; i < 4; i++) {
							Point2i nxtchild;
							child(node, i, nxtchild);
							if (cellBound(layer + 1, nxtchild).overlap(ibd)) {
								nextNodes.emplace_back(nxtchild);
							}
						}
					}
				}
			}
			if (success) 
				return true;
			if (nextNodes.empty()) return false;
			nodesTodo = std::move(nextNodes);
		}

		return false;
	}

	

	mcl::Bound2i StratiZBuffer::cellBound(int layer, const Float& x, const Float& y)
	{
		int span = totsize / size(layer);
		return Bound2i(Point2i(span * x, span * y), Point2i(span * (x + 1) - 1, span * (y + 1) - 1));
	}

	mcl::Bound2i StratiZBuffer::cellBound(int layer, const Point2i& pos)
	{
		int span = totsize / size(layer);
		Point2i temp = pos * span;
		return Bound2i(temp, temp + span - 1);
	}

	void StratiZBuffer::father(const Point2i& pos, Point2i& fatherpos)
	{
		fatherpos = pos / 2;
	}

	bool StratiZBuffer::updateDepth(const Point2i& childpos, Float depth)
	{
		int curlayer = nlayer - 1;
		Point2i pos = childpos;

		//如果全分辨率zbuffer的深度值更小，则不更新
		if (cell(curlayer, pos).maxdepth <= depth)
			return false;
		//否则更新单元深度值
		cell(curlayer, pos).maxdepth = depth;
		cell(curlayer, pos).mindepth = depth;

		bool updatemax = true;
		bool updatemin = true;

		//迭代更新所有层级的zbuffer
		Point2i fatherPos;
		for (curlayer; curlayer > 0; --curlayer) {
			father(pos, fatherPos);
			Cell& fatherCell = cell(curlayer - 1, fatherPos);
			Cell& childCell = cell(curlayer, pos);

			//父单元深度值取子单元深度值中最大的
			if (updatemax) {
				int index = indexOfChild(pos);
				fatherCell.updateChild(index);
				//通过mask来判断父节点是否需要更新，该方法能够大幅提速
				if (fatherCell.needUpdateMax()) {
					Float maxDepth = -MaxFloat;
					for (int i = 0; i < 4; i++) {
						Point2i tempchild;
						child(fatherPos, i, tempchild);
						Float tempdepth = cell(curlayer, tempchild).maxdepth;
						//将所有等于最大深度的子节点加入mask
						if (tempdepth > maxDepth) {
							maxDepth = tempdepth;
							fatherCell.resetMask();
							fatherCell.maskChild(i);
						}
						else if (tempdepth == maxDepth) {
							fatherCell.maskChild(i);
						}
					}
					fatherCell.maxdepth = maxDepth;
				}
				else updatemax = false;
			}
			if (updatemin) {
				if(fatherCell.mindepth <= depth) updatemin = false;
				else fatherCell.mindepth = depth;
			}
			if (updatemin || updatemax) {
				pos = std::move(fatherPos);
			}
			else {
				break;
			}
		}
		return true;
	}

	StratiZBufferRenderer::StratiZBufferRenderer(Scene* scene, int px, int py)
		:SoftRenderer(scene, px, py)
	{
		DCHECK(px == py);
	}



	int StratiZBufferRenderer::paint(PaintInfomation* info /*= nullptr*/)
	{
		sceneUpdated(Scene::ALL);
		isPainting = true;

		Clock<ZBUFFER_STRATIFY>::clear();
		Clock<ZBUFFER_STRATIFY_CULL>::clear();
		Clock<ZBUFFER_STRATIFY_SHADER>::clear();
		Clock<ZBUFFER_STRATIFY>::start();

		Transform viewTrans(scene_->getCamera().getViewMatrix());
		Transform projTrans(scene_->getCamera().getProjMatrix());
		Transform totTrans = film_->deviceToScreen()*projTrans * viewTrans;
		Transform invTotTran = totTrans.inverse();

		Vector3f screenXinWorld = invTotTran(Vector3f(1, 0, 0));
		Vector3f screenZinWorld = invTotTran(Vector3f(0, 0, 1));

		TransformInfo traninfo = { invTotTran ,screenXinWorld ,screenZinWorld };

		int npy = film_->getSize().y();
		int npx = film_->getSize().x();

		StratiZBuffer zbuffer(npx);

		std::vector<std::unique_ptr<MyPolygon>> polygons;

		/* 构造多边形数据 */
		const auto& prims = scene_->getPrimitives();
		Bound2i screenBound(Point2i(0, 0), Point2i(npx, npy));
		polygons.reserve(prims.size());
		for (const auto& prim : prims) {
			if (prim.second->dynamic_tag() == PPolygonMesh::tag()) {
				auto pplg = std::dynamic_pointer_cast<PPolygonMesh>(prim.second);
				const auto& pts = pplg->getPts();
				const auto& plgs = pplg->getPlgs();

				Primitive* pprim = prim.second.get();
				for (const auto& plg : plgs) {
					polygons.emplace_back(new MyPolygon);

					int ret = createRasPolygon(&(polygons.back()->data), pprim, totTrans, invTotTran, screenBound, plg.lps_[0], pts);

					if (ret < 0) {
						paintFinished();
						return -1;
					}

					if (ret > 0) {
						polygons.pop_back();
						continue;
					}

					RasPolygon& curplg = polygons.back()->data;
					Bound3f& curBd = polygons.back()->bound;
					for (auto it = curplg.edges.begin(); it != curplg.edges.end(); it++) {
						RasPlgNode& n1 = (*it)->node[0];
						RasPlgNode& n2 = (*it)->node[1];
						curBd.unionPt(Point3f(n1.scoord.x(), n1.scoord.y(), n1.depth));
						curBd.unionPt(Point3f(n2.scoord.x(), n2.scoord.y(), n2.depth));
					}
				}
			}
		}

		int painted = 0;

		Clock<ZBUFFER_STRATIFY_CULL>::start();
		//逐多边形判断是否剔除，否则光栅化并且存入缓冲
		for (const auto& plg : polygons) {
			if (!zbuffer.isCull(plg->bound)) {
				int ret;
				ret = rasterPolygon(plg->data, Bound2i(Bound2f::Bound2(plg->bound)), zbuffer, traninfo);
				painted++;
				DCHECK(!ret);
			}
		}
		Clock<ZBUFFER_STRATIFY_CULL>::stop();
		
		Clock<ZBUFFER_STRATIFY_SHADER>::start();
		//消隐结束后，用缓冲中的信息进行最后的绘制
		for (int x = 0; x < npx; x++) {
			for (int y = 0; y < npy; y++) {
				fragShader(zbuffer.cell(x,y).info, x, y);
			}
		}
		Clock<ZBUFFER_STRATIFY_SHADER>::stop();

		Clock<ZBUFFER_STRATIFY>::stop();
		paintFinished();
		return 0;
	}

	int StratiZBufferRenderer::rasterPolygon(RasPolygon& polygon, const Bound2i& bound, StratiZBuffer& zbuffer, const TransformInfo& traninfo)
	{
		FragInfo info;
		info.normal = polygon.normal;
		info.color = polygon.prim->color();

		//假设多边形不存在两边相交而无交点的情况，在该情况下，只有当新增活化边时，才需要重新排序
		bool needSort = true;

		for (int y = bound.pMin().y(); y <= bound.pMax().y(); y++) {
			//更新活化边
			while (getPlgMinY(&polygon) == y) {
				polygon.activeEdges.splice(polygon.activeEdges.end(), polygon.edges, polygon.edges.begin());
				needSort = true;
			}

			if (needSort) {
				polygon.activeEdges.sort([](const std::unique_ptr<RasPlgEdge>& e1, const std::unique_ptr<RasPlgEdge>& e2) {
					return e1->curX < e2->curX;
					});
				needSort = false;
			}
			int nae = polygon.activeEdges.size();
			auto it = polygon.activeEdges.begin();

			if (nae % 2 == 1) {
				info.color = Color3f(2, 0, 0);
				auto anormal = traninfo.invTotTran(Normal3f(0, 0, -1));
				anormal.normalize();
				info.normal = anormal;
				//return -2; //无法配对边对
				for (int i = 0; i < nae; i++) {
					const auto& e = *(it);
					info.worldPos = traninfo.invTotTran(Point3f(0,0,0));
					info.z = -MaxFloat;
					zbuffer.updateDepth(Point2i(e->curX, y), info.z);
					zbuffer.cell(e->curX, y).info = info;
				}
			}

			//配对边对，将区间信息存入buffer
			for (int i = 0; i < nae - 1; i += 2) {
				const auto& e1 = *(it++);
				const auto& e2 = *(it++);
				RasBiEdge be(e1.get(), e2.get());
				info.worldPos = traninfo.invTotTran(Point3f(Float(be.egs[0]->curX), Float(y), Float(info.z)));
				info.z = be.egs[0]->curZ;
				//为了防止端点处由于浮点误差导致失真，此处采用四舍五入
				for (int x = be.egs[0]->curX; x < be.egs[1]->curX ; x++) {
					if (zbuffer.updateDepth(Point2i(x, y), info.z)) {
						zbuffer.cell(x, y).info = info;
					}
					info.z += be.dzdx();
					info.worldPos = info.worldPos + traninfo.screenxInWorld * 1 + traninfo.screenzInWorld * be.dzdx();
				}
			}

			//移除过期边对
			for (auto it = polygon.activeEdges.begin(); it != polygon.activeEdges.end();) {
				//移除过期活化边
				if ((*it)->getLowerBound() == y) {
					it = polygon.activeEdges.erase(it);
				}
				//更新已有活化边
				else {
					(*it)->moveToNxtLine();
					it++;
				}
			}
		}

		return 0;
	}

}