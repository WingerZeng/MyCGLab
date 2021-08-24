#include "StratiZBufferOctreeRenderer.h"
#include "utilities.h"
#include "Primitive.h"
#include "primitives/PPolygonMesh.h"
namespace mcl{
	
	StratiZBufferOctreeRenderer::StratiZBufferOctreeRenderer(Scene* scene, int px, int py)
		:StratiZBufferRenderer(scene, px, py),tree(5,4)
	{

	}

	int StratiZBufferOctreeRenderer::paint(PaintInfomation* info /*= nullptr*/)
	{
		sceneUpdated(Scene::ALL);
		isPainting = true;
		Clock<ZBUFFER_STRATIFY_OCTREE>::clear();
		Clock<ZBUFFER_STRATIFY_OCTREE_CULL>::clear();
		Clock<ZBUFFER_STRATIFY_OCTREE_SHADER>::clear();
		Clock<ZBUFFER_STRATIFY_OCTREE>::start();

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
					polygons.back()->isDrawn = false;
					//计算屏幕空间包围盒
					Bound3f& curSBd = polygons.back()->sbound;
					for (auto it = curplg.edges.begin(); it != curplg.edges.end(); it++) {
						RasPlgNode& n1 = (*it)->node[0];
						RasPlgNode& n2 = (*it)->node[1];
						curSBd.unionPt(Point3f(n1.scoord.x(), n1.scoord.y(), n1.depth));
						curSBd.unionPt(Point3f(n2.scoord.x(), n2.scoord.y(), n2.depth));
					}
					//计算世界空间包围盒
					Bound3f& curWBd = polygons.back()->wbound;
					for (const auto& pt : plg.lps_[0]) {
						curWBd.unionPt(pprim->localTransform()(pts[pt]));
					}
				}
			}
		}

		//建立八叉树
		for (const auto& plg : polygons) {
			tree.addElement(plg.get());
		}
		tree.createTree();

		//制定新的八叉树子节点遍历顺序（按包围盒中点的屏幕空间深度，从前至后遍历）
		Bound3f tempbd(Point3f(0), Point3f(1));
		struct midpt
		{
			int index;
			Point3f pt;
			Float screenz;
		};
		std::vector<midpt> midpts;
		for (int i = 0; i < 8; i++) {
			midpts.push_back({ i,tempbd.octreeSplit(i).mid(),0 });
			midpts.back().screenz = totTrans(midpts.back().pt).z();
		}
		auto cmp = [&totTrans](const midpt& lhs, const midpt& rhs) {
			return lhs.screenz < rhs.screenz;
		};
		std::sort(midpts.begin(), midpts.end(), cmp);
		auto NewChildIndex = [&midpts](int index) {
			return midpts[index].index;
		};

		// 深度优先遍历八叉树，进行消隐
		Clock<ZBUFFER_STRATIFY_OCTREE_CULL>::start();
		struct MyNode
		{
			typename ocnode_t* node;
			int nextIndex;
			bool culled;
		};
		std::vector<MyNode> snode;
		snode.push_back({ tree.getRoot(),0,false });

		int painted = 0;

		while (!snode.empty()) {
			ocnode_t* curnode = snode.back().node;
			if (snode.back().nextIndex == 8) {
				snode.pop_back();
				continue;
			}

			//如果八叉树节点被剔除，则不遍历该节点，只有第一次遇到该节点时才判断剔除
			if (!snode.back().culled && zbuffer.isCull(totTrans(curnode->bound()))){
				snode.pop_back();
				snode.back().culled = true;
				continue;
			}

			//如为内部节点，则继续遍历
			if (curnode->isInner()) {
				//#PERF1 把8个子节点共有的元素放到对应的内部节点中，如果节点为非空，需要进行绘制
				ocnode_t* childnode = curnode->child(NewChildIndex(snode.back().nextIndex));
				snode.back().nextIndex++;
				snode.push_back({ childnode,0,false });
			}

			//如果为叶节点，则绘制其中内容
			else if (curnode->isLeaf()) {
				auto plgs = tree.query(*curnode);
				for (const auto& plgindex : plgs) {
					const auto& plg = tree.getObj(plgindex);
					//判断多边形是否被重复绘制
					if (plg->isDrawn) continue;
					//绘制多边形到缓冲
					painted++;
					int ret = rasterPolygon(plg->data, Bound2i(Bound2f::Bound2(plg->sbound)), zbuffer, traninfo);
					DCHECK(!ret);
					plg->isDrawn = true;
				}
				snode.pop_back();
			}

			//空节点情况
			else {
				snode.pop_back();
			}
		}
		Clock<ZBUFFER_STRATIFY_OCTREE_CULL>::stop();

		Clock<ZBUFFER_STRATIFY_OCTREE_SHADER>::start();
		//消隐结束后，用缓冲中的信息进行最后的绘制
		for (int x = 0; x < npx; x++) {
			for (int y = 0; y < npy; y++) {
				fragShader(zbuffer.cell(x, y).info, x, y);
			}
		}
		Clock<ZBUFFER_STRATIFY_OCTREE_SHADER>::stop();

		Clock<ZBUFFER_STRATIFY_OCTREE>::stop();

		paintFinished();
		tree.clear();

		return 0;
	}

}