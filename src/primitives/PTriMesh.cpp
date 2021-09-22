#include "PTriMesh.h"
#include "shaders.h"
#include "PaintInformation.h"
#include "geometries/GTriangleMesh.h"
#include "Light.h"
#include "PaintVisitor.h"
#include "Material.h"
#include "GLFunctions.h"
namespace mcl{
	
	PTriMesh::PTriMesh(const std::vector<int>& tris, const std::vector<Point3f>& pts) :tris_(tris)
	{
		setColor(Vector3f{ 0.6,0.6,0.6 });
		for (const auto& pt : pts) {
			pts_.push_back(pt.x());
			pts_.push_back(pt.y());
			pts_.push_back(pt.z());
		}

		std::set<std::pair<int, int>> egs;
		for (int i = 0; i < tris_.size(); i += 3) {
			int eg[3][2] = { {0,1} ,{0,2}, {1,2} };
			for (int j = 0; j < 3; j++) {
				int p1 = tris_[i + eg[j][0]];
				int p2 = tris_[i + eg[j][1]];
				if (p1 < p2)
					std::swap(p1, p2);
				egs.insert(std::make_pair(p1, p2));
			}
		}
		linepts_.reset(new Float[egs.size() * 6]);
		int idx = 0;
		for (auto it = egs.begin(); it != egs.end(); it++) {
			linepts_[idx++] = (pts_[3 * it->first + 0]);
			linepts_[idx++] = (pts_[3 * it->first + 1]);
			linepts_[idx++] = (pts_[3 * it->first + 2]);
			linepts_[idx++] = (pts_[3 * it->second + 0]);
			linepts_[idx++] = (pts_[3 * it->second + 1]);
			linepts_[idx++] = (pts_[3 * it->second + 2]);
		}
		nEgs = egs.size();
	}

	PTriMesh::PTriMesh(const std::vector<int>& tris, const std::vector<Point3f>& pts, const std::vector<Point2f>& uvs, const std::vector<Normal3f>& normals)
		:tris_(tris), uvs(uvs), normals(normals)
	{
		setColor(Vector3f{ 0.6,0.6,0.6 });
		for (const auto& pt : pts) {
			pts_.push_back(pt.x());
			pts_.push_back(pt.y());
			pts_.push_back(pt.z());
		}

		std::set<std::pair<int, int>> egs;
		for (int i = 0; i < tris_.size(); i += 3) {
			int eg[3][2] = { {0,1} ,{0,2}, {1,2} };
			for (int j = 0; j < 3; j++) {
				int p1 = tris_[i + eg[j][0]];
				int p2 = tris_[i + eg[j][1]];
				if (p1 < p2)
					std::swap(p1, p2);
				egs.insert(std::make_pair(p1, p2));
			}
		}
		linepts_.reset(new Float[egs.size() * 6]);
		int idx = 0;
		for (auto it = egs.begin(); it != egs.end(); it++) {
			linepts_[idx++] = (pts_[3 * it->first + 0]);
			linepts_[idx++] = (pts_[3 * it->first + 1]);
			linepts_[idx++] = (pts_[3 * it->first + 2]);
			linepts_[idx++] = (pts_[3 * it->second + 0]);
			linepts_[idx++] = (pts_[3 * it->second + 1]);
			linepts_[idx++] = (pts_[3 * it->second + 2]);
		}
		nEgs = egs.size();
	}

	void PTriMesh::initializeGL()
	{
		GLFUNC->initializeOpenGLFunctions();

		vao = std::make_shared<QOpenGLVertexArrayObject>();
		vbo = std::make_shared<QOpenGLBuffer>();

		vao->create();
		vao->bind();

		vbo->create();
		vbo->bind();
		
		rawPts.reset(new Float[tris_.size() * 3]);
		for (int i = 0; i < tris_.size(); i++) {
			rawPts[3 * i] = (pts_[3 * tris_[i] + 0]);
			rawPts[3 * i + 1] = (pts_[3 * tris_[i] + 1]);
			rawPts[3 * i + 2] = (pts_[3 * tris_[i] + 2]);
		}

		rawUvs.reset(new Float[uvs.size() * 2]);
		for (int i = 0; i < uvs.size(); i++) {
			rawUvs[2 * i + 0] = (uvs[i][0]);
			rawUvs[2 * i + 1] = (uvs[i][1]);
		}

		rawNormals.reset(new Float[normals.size() * 3]);
		for (int i = 0; i < normals.size(); i++) {
			rawNormals[3 * i] = normals[i][0];
			rawNormals[3 * i + 1] = normals[i][1];
			rawNormals[3 * i + 2] = normals[i][2];
		}

		//导入面片顶点数据
		auto totsize = (tris_.size() * 3 + uvs.size() * 2 + normals.size() * 3) * sizeof(Float);
		GLFUNC->glBufferData(GL_ARRAY_BUFFER, tris_.size() * 3 * sizeof(Float), rawPts.get(), GL_STATIC_DRAW);
		GLFUNC->glBufferData(GL_ARRAY_BUFFER, totsize,nullptr , GL_STATIC_DRAW);
		GLintptr offset1 = 0;
		GLFUNC->glBufferSubData(GL_ARRAY_BUFFER, offset1, tris_.size() * 3 * sizeof(Float), &rawPts[0]);
		GLintptr offset2 = tris_.size() * 3 * sizeof(Float);
		GLFUNC->glBufferSubData(GL_ARRAY_BUFFER, offset2, normals.size() * 3 * sizeof(Float), &rawNormals[0]);
		GLintptr offset3 = offset2 + normals.size() * 3 * sizeof(Float);
		GLFUNC->glBufferSubData(GL_ARRAY_BUFFER, offset3, uvs.size() * 2 * sizeof(Float), &rawUvs[0]);

		GLFUNC->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(Float), 0);
		GLFUNC->glEnableVertexAttribArray(0);
		GLFUNC->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(Float), (void*)(sizeof(Float) * tris_.size() * 3));
		GLFUNC->glEnableVertexAttribArray(1);
		GLFUNC->glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(Float), (void*)(sizeof(Float) * (tris_.size() + normals.size()) * 3));
		GLFUNC->glEnableVertexAttribArray(2);

		//导入边顶点数据
		linevao = std::make_shared<QOpenGLVertexArrayObject>();
		linevao->create();
		linevao->bind();
		linevbo = std::make_shared<QOpenGLBuffer>();
		linevbo->create();
		linevbo->bind();
		GLFUNC->glBufferData(GL_ARRAY_BUFFER, nEgs * 6 * sizeof(Float), &linepts_[0], GL_STATIC_DRAW);
		GLFUNC->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(Float), 0);
		GLFUNC->glEnableVertexAttribArray(0);
	}

	void PTriMesh::initialize(PaintVisitor* visitor)
	{
		visitor->initTris(this);
	}

	std::shared_ptr<QOpenGLVertexArrayObject> PTriMesh::getVAO()
	{
		return vao;
	}

	std::shared_ptr<QOpenGLVertexArrayObject> PTriMesh::getLineVAO()
	{
		return linevao;
	}

	void PTriMesh::paint(PaintInfomation* info, PaintVisitor* visitor)
	{
		visitor->paintTris(info, this);
	}

	std::unique_ptr<mcl::Primitive> PTriMesh::clone()
	{
		std::vector<Point3f> pts;
		pts.reserve(pts_.size() / 3);
		for (int i = 0; i < pts_.size()/3; i++) {
			pts.emplace_back(pts_[i * 3], pts_[i * 3 + 1], pts_[i * 3 + 2]);
		}
		return std::make_unique<PTriMesh>(tris_, pts, uvs, normals);
	}

	std::shared_ptr<mcl::Geometry> PTriMesh::createGeometry()
	{
		auto pts = Point3f::fromFloatVec(pts_);
		if(uvs.empty() || normals.empty())
			return std::make_shared<GTriangleMesh>(localTransform(), tris_, pts);
		return std::make_shared<GTriangleMesh>(localTransform(), tris_, pts, uvs, normals);
	}

	mcl::Point3f PTriMesh::getCentroid()
	{
		Point3f ret;
		for (int i = 0; i < pts_.size(); i += 3) {
			ret.x() += pts_[i];
			ret.y() += pts_[i + 1];
			ret.z() += pts_[i + 2];
		}
		return ret / pts_.size() * 3;
	}

	mcl::Bound3f PTriMesh::getBound()
	{
		Bound3f ret;
		for (int i = 0; i < pts_.size(); i += 3) {
			ret.unionPt(Point3f(pts_[i], pts_[i + 1], pts_[i + 2]));
		}
		return this->totTransform()(ret);
	}

	void PTriMesh::initAll()
	{
		if (this->normals.empty())
			generateNormal();
		GeometryPrimitive::initAll();
	}

	void PTriMesh::generateNormal()
	{
		//#TODO0
	}

	int PTriMesh::getTriNumer()
	{
		return getIndices().size() / 3;
	}

	int PTriMesh::getEdgeNumber()
	{
		return nEgs;
	}

	std::unique_ptr<PTriMesh> PTriMesh::createBillBoard()
	{
		std::vector<int> tris{0,1,2};
		std::vector<Point3f> pts{ Point3f{-1,-1,0}, Point3f{3,-1,0}, Point3f{-1,3,0} };
		std::vector<Point2f> uvs{ Point2f{0,0}, Point2f{2,0}, Point2f{0,2} };
		return std::make_unique<PTriMesh>(tris, pts, uvs, std::vector<Normal3f>{Normal3f(), Normal3f(), Normal3f()});
	}

}