#include "PTriMesh.h"
#include "shaders.h"
#include "PaintInformation.h"
#include "geometries/GTriangleMesh.h"
#include "Light.h"
#include "PaintVisitor.h"
#include "Material.h"
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
		for (auto it = egs.begin(); it != egs.end(); it++) {
			linepts_.push_back(pts_[3 * it->first + 0]);
			linepts_.push_back(pts_[3 * it->first + 1]);
			linepts_.push_back(pts_[3 * it->first + 2]);
			linepts_.push_back(pts_[3 * it->second + 0]);
			linepts_.push_back(pts_[3 * it->second + 1]);
			linepts_.push_back(pts_[3 * it->second + 2]);
		}
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
		for (auto it = egs.begin(); it != egs.end(); it++) {
			linepts_.push_back(pts_[3 * it->first + 0]);
			linepts_.push_back(pts_[3 * it->first + 1]);
			linepts_.push_back(pts_[3 * it->first + 2]);
			linepts_.push_back(pts_[3 * it->second + 0]);
			linepts_.push_back(pts_[3 * it->second + 1]);
			linepts_.push_back(pts_[3 * it->second + 2]);
		}
	}

	void PTriMesh::initializeGL()
	{
		this->initializeOpenGLFunctions();

		vao = std::make_shared<QOpenGLVertexArrayObject>();
		vbo = std::make_shared<QOpenGLBuffer>();

		vao->create();
		vao->bind();

		vbo->create();
		vbo->bind();

		std::vector<Float> allpts; //将pts和indices合并
		for (const auto idx : tris_) {
			allpts.push_back(pts_[3 * idx + 0]);
			allpts.push_back(pts_[3 * idx + 1]);
			allpts.push_back(pts_[3 * idx + 2]);
		}

		std::vector<Float> tempuvs;
		for (const auto uv : uvs) {
			tempuvs.push_back(uv[0]);
			tempuvs.push_back(uv[1]);
		}
		
		std::vector<Float> tempnormals;
		for (const auto normal : normals) {
			tempnormals.push_back(normal[0]);
			tempnormals.push_back(normal[1]);
			tempnormals.push_back(normal[2]);
		}

		//导入面片顶点数据
		auto totsize = (allpts.size() + tempuvs.size() + tempnormals.size()) * sizeof(Float);
		glBufferData(GL_ARRAY_BUFFER, totsize, nullptr, GL_STATIC_DRAW);
		GLintptr offset1 = 0;
		glBufferSubData(GL_ARRAY_BUFFER, offset1, allpts.size() * sizeof(Float), &allpts[0]);
		GLintptr offset2 = allpts.size() * sizeof(Float);
		glBufferSubData(GL_ARRAY_BUFFER, offset2, tempnormals.size() * sizeof(Float), &tempnormals[0]);
		GLintptr offset3 = offset2 + tempnormals.size() * sizeof(Float);
		glBufferSubData(GL_ARRAY_BUFFER, offset3, tempuvs.size() * sizeof(Float), &tempuvs[0]);

		//导入边顶点数据
		linevbo = std::make_shared<QOpenGLBuffer>();
		linevbo->create();
		linevbo->bind();
		glBufferData(GL_ARRAY_BUFFER, linepts_.size() * sizeof(Float), &linepts_[0], GL_STATIC_DRAW);

	}

	void PTriMesh::initialize(QOpenGLShaderProgram* shader)
	{
		vao->bind();
		vbo->bind();
		shader->bind();
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(Float), 0);
		shader->enableAttributeArray(0);
		
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(Float), (void*)(sizeof(Float) * tris_.size() * 3));
		shader->enableAttributeArray(1);

		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(Float), (void*)(sizeof(Float) * (tris_.size() + normals.size()) * 3));
		shader->enableAttributeArray(2);
	}

	void PTriMesh::initialize(PaintVisitor* visitor)
	{
		visitor->initTris(this);
	}

	std::shared_ptr<QOpenGLVertexArrayObject> PTriMesh::getVAO()
	{
		return vao;
	}

	void PTriMesh::initializeLine(QOpenGLShaderProgram* shader)
	{
		vao->bind();
		linevbo->bind();
		shader->bind();
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(Float), 0);
		shader->enableAttributeArray(0);
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

	std::shared_ptr<QOpenGLBuffer> PTriMesh::getVBO()
	{
		return vbo;
	}

	std::shared_ptr<QOpenGLBuffer> PTriMesh::getLineVBO()
	{
		return linevbo;
	}

	int PTriMesh::getTriNumer()
	{
		return getIndices().size() / 3;
	}

	int PTriMesh::getEdgeNumber()
	{
		return linepts_.size() / 6;
	}

}