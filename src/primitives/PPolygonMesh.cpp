#include "PPolygonMesh.h"
#include "PPolygon.h"
#include "shaders.h"
#include "PaintInformation.h"
#include "Light.h"

#include "PTriMesh.h"
#include "realtime/Scene.h"
#include "ui/MainWindow.h"
#include "ItemManager.h"
namespace mcl{
	
	PPolygonMesh::PPolygonMesh(std::shared_ptr<PTriMesh> trimesh)
	{
		pts_ = Point3f::fromFloatVec(trimesh->getPts());
		for (int i = 0; i < trimesh->getIndices().size()/3; i++) {
			const auto& indices = trimesh->getIndices();
			plgs_.emplace_back();
			plgs_.back().lps_.push_back(std::vector<int>{ static_cast<int>(indices[3 * i]), static_cast<int>(indices[3 * i + 1]), static_cast<int>(indices[3 * i + 2])});
		}

		setColor(Vector3f{ 0.6,0.6,0.6 });
		buildTessPts();
	}

	PPolygonMesh::~PPolygonMesh()
	{
		if(vbo)vbo->destroy();
		if(vao)vao->destroy();
		if(linevbo)linevbo->destroy();
	}

	int PPolygonMesh::toTriangleMesh(std::vector<int>& tris, std::vector<Point3f>& pts)
	{
		std::map<Point3f, int> ptoi;
		pts.clear();
		tris.clear();

		//通过容差判断点是否在点集中，不是则新建点，返回索引
		auto getPtIdx = [&ptoi, &pts](const Point3f& pt) {
			int idx;
			auto it = ptoi.find(pt);
			if (it == ptoi.end()) {
				pts.push_back(pt);
				idx = ptoi[pt] = pts.size() - 1;
			}
			else
			{
				DCHECK((pt - it->first).length() < MachineEpsilon*1e2);
				idx = it->second;
			}
			return idx;
		};

		auto getTessPt = [this](int idx) {
			return Point3f(this->tessPts_[idx * 3], tessPts_[idx * 3 + 1], tessPts_[idx * 3 + 2]);
		};

		for (const auto& info : drawInfo_) {
			switch (info.type)
			{
			case GL_TRIANGLES:
			{
				for (int i = 0; i < info.size; i++) {
					tris.push_back(getPtIdx(getTessPt(info.offset + i)));
				}
				break;
			}

			case GL_TRIANGLE_FAN:
			{
				int firstPt = getPtIdx(getTessPt(info.offset));
				int secondPt = getPtIdx(getTessPt(info.offset + 1));
				for (int i = 2; i < info.size; i++) {
					int thirdPt = getPtIdx(getTessPt(info.offset + i));
					tris.push_back(firstPt);
					tris.push_back(secondPt);
					tris.push_back(thirdPt);
					secondPt = thirdPt;
				}
				break;
			}

			case GL_TRIANGLE_STRIP:
			{
				bool odd = true;
				int firstPt = getPtIdx(getTessPt(info.offset));
				int secondPt = getPtIdx(getTessPt(info.offset + 1));
				for (int i = 2; i < info.size; i++) {
					int thirdPt = getPtIdx(getTessPt(info.offset + i));
					if (odd) {
						tris.push_back(firstPt);
						tris.push_back(secondPt);
						tris.push_back(thirdPt);
					}
					else {
						tris.push_back(firstPt);
						tris.push_back(thirdPt);
						tris.push_back(secondPt);
					}
					firstPt = secondPt;
					secondPt = thirdPt;
					odd = !odd;
					secondPt = thirdPt;
				}
				break;
			}

			default:
				CHECK(0);
				break;
			}
		}
		return 0;
	}

	void PPolygonMesh::initializeGL()
	{
		this->initializeOpenGLFunctions();

		if (tessPts_.empty()) 
			return;

		vao = std::make_shared<QOpenGLVertexArrayObject>();
		vbo = std::make_shared<QOpenGLBuffer>();

		vao->create();
		vao->bind();
		vbo->create();
		vbo->bind();
		vbo->allocate(&tessPts_[0], tessPts_.size() * sizeof(Float));

		//生成边界点
		for (const auto& plg : plgs_) {
			for (const auto& lp : plg.lps_) {
				boundDrawInfo_.push_back(DrawSingleObjInfo(GL_LINE_LOOP,boundPts_.size() / 3,lp.size()));
				for (const auto& ptIdx : lp) {
					boundPts_.push_back(pts_[ptIdx].x());
					boundPts_.push_back(pts_[ptIdx].y());
					boundPts_.push_back(pts_[ptIdx].z());
				}
			}
		}

		linevbo = std::make_shared<QOpenGLBuffer>();
		linevbo->create();
		linevbo->bind();
		linevbo->allocate(&boundPts_[0], boundPts_.size() * sizeof(Float));
	}

	std::unique_ptr<mcl::Primitive> PPolygonMesh::clone()
	{
		return std::make_unique<PPolygonMesh>(plgs_, pts_);
	}

	int PPolygonMesh::buildTessPts()
	{
		std::vector<std::vector<std::vector<Point3f>>> plgPts;
		for (const auto& plg : plgs_) {
			plgPts.emplace_back();
			for (const auto& lp : plg.lps_) {
				plgPts.back().emplace_back();
				for (const auto& ptIdx : lp) {
					plgPts.back().back().push_back(pts_[ptIdx]);
				}
			}
		}

		//将多边形离散为面片
		return tessPolygons(plgPts, &tessPts_, &drawInfo_);
	}

	int triangulatePolygonMesh(int id)
	{
		std::shared_ptr<PPolygonMesh> plg;
		MAIPTR->getItemMng()->getItem(id,plg);
		if (!plg) return -1;
		std::vector<int> tris;
		std::vector<Point3f> pts;
		plg->toTriangleMesh(tris, pts);
		auto tri = std::make_shared<PTriMesh>(tris, pts);
		tri->copyAttribute(plg);
		MAIPTR->getItemMng()->addItem(tri);
		MAIPTR->getItemMng()->delItem(plg);
		return 0;
	}

}