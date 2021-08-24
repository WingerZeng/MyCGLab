#include "PTriMesh.h"
#include "shaders.h"
#include "PaintInformation.h"
#include "geometries/GTriangleMesh.h"
#include "Light.h"
namespace mcl{
	
	PTriMesh::PTriMesh(const std::vector<int>& tris, const std::vector<Point3f>& pts) :tris_(tris)
	{
		setColor(Vector3f{ 0.6,0.6,0.6 });
		for (const auto& pt : pts) {
			pts_.push_back(pt.x());
			pts_.push_back(pt.y());
			pts_.push_back(pt.z());
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
	}

	void PTriMesh::initialize()
	{
		this->initializeOpenGLFunctions();

		if (getIndices().size() < 3 || pts_.size() <= 9) return;

		ebo = std::make_shared<QOpenGLBuffer>(QOpenGLBuffer::IndexBuffer);
		vao = std::make_shared<QOpenGLVertexArrayObject>();
		vbo = std::make_shared<QOpenGLBuffer>();

		vao->create();
		vao->bind();
		vbo->create();
		vbo->bind();
		ebo->create();
		ebo->bind();
		vbo->allocate(&pts_[0], pts_.size() * sizeof(Float));
		ebo->allocate(&getIndices()[0], getIndices().size() * sizeof(int));

		int attr = -1;
		attr = CommonShader::ptr()->attributeLocation("aPos");
		CommonShader::ptr()->setAttributeBuffer(attr, GL_FLOAT, 0, 3, 0);
		CommonShader::ptr()->enableAttributeArray(attr);

		attr = LightPerFragShader::ptr()->attributeLocation("aPos");
		LightPerFragShader::ptr()->setAttributeBuffer(attr, GL_FLOAT, 0, 3, 0);
		LightPerFragShader::ptr()->enableAttributeArray(attr);


		attr = LineShader::ptr()->attributeLocation("aPos");
		LineShader::ptr()->setAttributeBuffer(attr, GL_FLOAT, 0, 3, 0);
		LineShader::ptr()->enableAttributeArray(attr);

		readyToDraw = true;
	}

	void PTriMesh::paint(PaintInfomation* info)
	{
		doBeforePaint(info);
		if (!readyToDraw) return;

		if (info->fillmode == FILL || info->fillmode == FILL_WIREFRAME) {
			QOpenGLShaderProgram* shader;
			if (info->lights.size() && !selected())
			{
				shader = LightPerFragShader::ptr();
				shader->bind();
				shader->setUniformValue("modelMat", QMatrix4x4());
				shader->setUniformValue("viewMat", info->viewMat);
				shader->setUniformValue("projMat", info->projMat);
				shader->setUniformValue("ourColor", color().x(), color().y(), color().z(), 1.0f);
				shader->setUniformValue("lightCount", GLint(info->lights.size()));
				//auto viewNormal = QVector3D((info->viewMat).inverted().transposed()*QVector4D(QVector3D(normal_), 0));
				//if (viewNormal.z() < 0) viewNormal = -viewNormal;
				for (int j = 0; j < info->lights.size(); j++) {
					std::string lightname = ("lights[" + std::to_string(j) + "]").c_str();
					shader->setUniformValue((lightname + ".ambient").c_str(), QVector3D(info->lights[j]->getAmbient()));
					shader->setUniformValue((lightname + ".pos").c_str(), QVector3D(info->lights[j]->getPosition()));
					shader->setUniformValue((lightname + ".diffuse").c_str(), QVector3D(info->lights[j]->getDiffuse()));
				}
			}
			else {
				shader = CommonShader::ptr();
				shader->bind();
				shader->setUniformValue("modelMat", QMatrix4x4());
				shader->setUniformValue("viewMat", info->viewMat);
				shader->setUniformValue("projMat", info->projMat);

				shader->setUniformValue("ourColor", color().x(), color().y(), color().z(), 1.0f);
			}

			vao->bind();
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glDrawElements(GL_TRIANGLES, getIndices().size(), GL_UNSIGNED_INT, 0);

			glDisable(GL_POLYGON_OFFSET_FILL);
			glPolygonOffset(0, 0);

			shader->release();

		}
		if (info->fillmode == WIREFRAME || info->fillmode == FILL_WIREFRAME) {
			LineShader::ptr()->bind();
			LineShader::ptr()->setUniformValue("modelMat", QMatrix4x4());
			LineShader::ptr()->setUniformValue("viewMat", info->viewMat);
			LineShader::ptr()->setUniformValue("projMat", info->projMat);
			LineShader::ptr()->setUniformValue("ourColor", .0, .0, .0, 1.0f);
			LineShader::ptr()->setUniformValue("u_viewportSize", info->width, info->height);
			LineShader::ptr()->setUniformValue("u_thickness", GLfloat(info->lineWidth));
			glEnable(GL_POLYGON_OFFSET_FILL);
			glPolygonOffset(-1, -1);

			vao->bind();

			// #PERF3 使用一个大的数组来保存所有边界线 1.去掉循环 2.去掉函数调用中指定索引
			int nTri = getIndices().size() / 3;
			for(int i=0;i< nTri;i++)
				glDrawElements(GL_LINE_LOOP, 3, GL_UNSIGNED_INT, (GLvoid*)((i*3)*sizeof(unsigned int)));

			glDisable(GL_POLYGON_OFFSET_FILL);
			glPolygonOffset(0, 0);
			LineShader::ptr()->release();
		}

		doAfterPaint(info);
	}

	std::unique_ptr<mcl::Primitive> PTriMesh::clone()
	{
		std::vector<Point3f> pts;
		pts.reserve(pts_.size() / 3);
		for (int i = 0; i < pts_.size()/3; i++) {
			pts.emplace_back(pts_[i * 3], pts_[i * 3 + 1], pts_[i * 3 + 2]);
		}
		return std::make_unique<PTriMesh>(tris_, pts);
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

}