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
		//����ֱ���ҵ���С�����ڵ�
		int flayer = 0;
		for (flayer = 0; flayer < nlayer-1; flayer++) {
			bool find = false;
			for (int i = 0; i < 4; i++) {
				Point2i childpos;
				child(nodeTodo, i, childpos);
				//�ӽڵ������������������ӽڵ�
				if (cellBound(flayer + 1, childpos.x(), childpos.y()).include(ibd)) {
					nodeTodo = childpos;
					find = true;
					break;
				}
			}
			//����ĸ��ӽڵ㶼����������˵�����ڵ�Ϊ��С�����ڵ�
			if (!find) {
				break;
			}
		}

		//����С�����ڵ㿪ʼ����
		std::vector<Point2i> nodesTodo{ nodeTodo };
		for (int layer = flayer; layer < nlayer; layer++) {
			bool success = true;
			std::vector<Point2i> nextNodes;
			nextNodes.reserve(4);
			for (const auto& node : nodesTodo) {
				//���������С�ڸýڵ���С��ȣ���϶���Ҫ����
				if (maxdepth < cell(layer, node).mindepth) {
					return false;
				}

				//������κ�һ���ڵ������ȴ���Ŀ����С��ȣ������ʧ��
				//����ýڵ㱻Ŀ����ȫ�����������ж�ʧ�ܣ���˵�������޳�������˵�����ڲ��ڵ��ж��ɹ�
				if (ibd.include(cellBound(layer, node))) {
					if (mindepth < cell(layer, node).maxdepth) {
						return false;
					}
				}
				//�������ж�ʧ�ܣ�������ж��ӽڵ㣬����˵�����ڲ����ڵ��ж��ɹ�
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

		//���ȫ�ֱ���zbuffer�����ֵ��С���򲻸���
		if (cell(curlayer, pos).maxdepth <= depth)
			return false;
		//������µ�Ԫ���ֵ
		cell(curlayer, pos).maxdepth = depth;
		cell(curlayer, pos).mindepth = depth;

		bool updatemax = true;
		bool updatemin = true;

		//�����������в㼶��zbuffer
		Point2i fatherPos;
		for (curlayer; curlayer > 0; --curlayer) {
			father(pos, fatherPos);
			Cell& fatherCell = cell(curlayer - 1, fatherPos);
			Cell& childCell = cell(curlayer, pos);

			//����Ԫ���ֵȡ�ӵ�Ԫ���ֵ������
			if (updatemax) {
				int index = indexOfChild(pos);
				fatherCell.updateChild(index);
				//ͨ��mask���жϸ��ڵ��Ƿ���Ҫ���£��÷����ܹ��������
				if (fatherCell.needUpdateMax()) {
					Float maxDepth = -MaxFloat;
					for (int i = 0; i < 4; i++) {
						Point2i tempchild;
						child(fatherPos, i, tempchild);
						Float tempdepth = cell(curlayer, tempchild).maxdepth;
						//�����е��������ȵ��ӽڵ����mask
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

		/* ������������ */
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
		//�������ж��Ƿ��޳��������դ�����Ҵ��뻺��
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
		//�����������û����е���Ϣ�������Ļ���
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

		//�������β����������ཻ���޽����������ڸ�����£�ֻ�е��������ʱ������Ҫ��������
		bool needSort = true;

		for (int y = bound.pMin().y(); y <= bound.pMax().y(); y++) {
			//���»��
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
				//return -2; //�޷���Ա߶�
				for (int i = 0; i < nae; i++) {
					const auto& e = *(it);
					info.worldPos = traninfo.invTotTran(Point3f(0,0,0));
					info.z = -MaxFloat;
					zbuffer.updateDepth(Point2i(e->curX, y), info.z);
					zbuffer.cell(e->curX, y).info = info;
				}
			}

			//��Ա߶ԣ���������Ϣ����buffer
			for (int i = 0; i < nae - 1; i += 2) {
				const auto& e1 = *(it++);
				const auto& e2 = *(it++);
				RasBiEdge be(e1.get(), e2.get());
				info.worldPos = traninfo.invTotTran(Point3f(Float(be.egs[0]->curX), Float(y), Float(info.z)));
				info.z = be.egs[0]->curZ;
				//Ϊ�˷�ֹ�˵㴦���ڸ�������ʧ�棬�˴�������������
				for (int x = be.egs[0]->curX; x < be.egs[1]->curX ; x++) {
					if (zbuffer.updateDepth(Point2i(x, y), info.z)) {
						zbuffer.cell(x, y).info = info;
					}
					info.z += be.dzdx();
					info.worldPos = info.worldPos + traninfo.screenxInWorld * 1 + traninfo.screenzInWorld * be.dzdx();
				}
			}

			//�Ƴ����ڱ߶�
			for (auto it = polygon.activeEdges.begin(); it != polygon.activeEdges.end();) {
				//�Ƴ����ڻ��
				if ((*it)->getLowerBound() == y) {
					it = polygon.activeEdges.erase(it);
				}
				//�������л��
				else {
					(*it)->moveToNxtLine();
					it++;
				}
			}
		}

		return 0;
	}

}