#include "PPolygonMesh.h"
#include "PPolygon.h"
#include "shaders.h"
#include "PaintInformation.h"
#include "Light.h"

#include "PTriMesh.h"
#include "realtime/Scene.h"
#include "ui/MainWindow.h"
#include "ui/ItemManager.h"
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
		if(linevao)linevao->destroy();
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

	void PPolygonMesh::initialize()
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

		int attr = -1;
		attr = CommonShader::ptr()->attributeLocation("aPos");
		CommonShader::ptr()->setAttributeBuffer(attr, GL_FLOAT, 0, 3, 0);
		CommonShader::ptr()->enableAttributeArray(attr);

		attr = LightPerFragShader::ptr()->attributeLocation("aPos");
		LightPerFragShader::ptr()->setAttributeBuffer(attr, GL_FLOAT, 0, 3, 0);
		LightPerFragShader::ptr()->enableAttributeArray(attr);

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

		linevao = std::make_shared<QOpenGLVertexArrayObject>();
		linevbo = std::make_shared<QOpenGLBuffer>();

		linevao->create();
		linevao->bind();
		linevbo->create();
		linevbo->bind();
		linevbo->allocate(&boundPts_[0], boundPts_.size() * sizeof(Float));

		attr = -1;
		attr = LineShader::ptr()->attributeLocation("aPos");
		LineShader::ptr()->setAttributeBuffer(attr, GL_FLOAT, 0, 3, 0);
		LineShader::ptr()->enableAttributeArray(attr);

		readyToDraw = true;
	}

	void PPolygonMesh::paint(PaintInfomation* info)
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

			for(int i =0;i<drawInfo_.size();i++)
				glDrawArrays(drawInfo_[i].type, drawInfo_[i].offset, drawInfo_[i].size);

			glDisable(GL_POLYGON_OFFSET_FILL);
			glPolygonOffset(0, 0);

			shader->release();
		}
		if (info->fillmode == WIREFRAME || info->fillmode == FILL_WIREFRAME) {  //选中时隐藏
			LineShader::ptr()->bind();
			LineShader::ptr()->setUniformValue("modelMat", QMatrix4x4());
			LineShader::ptr()->setUniformValue("viewMat", info->viewMat);
			LineShader::ptr()->setUniformValue("projMat", info->projMat);
			LineShader::ptr()->setUniformValue("ourColor", .0, .0, .0, 1.0f);
			LineShader::ptr()->setUniformValue("u_viewportSize", info->width, info->height);
			LineShader::ptr()->setUniformValue("u_thickness", GLfloat(info->lineWidth));
			glEnable(GL_POLYGON_OFFSET_FILL);
			glPolygonOffset(-1, -1);

			linevao->bind();

			for (int i = 0; i < boundDrawInfo_.size(); i++)
				glDrawArrays(boundDrawInfo_[i].type, boundDrawInfo_[i].offset, boundDrawInfo_[i].size);
			
			glDisable(GL_POLYGON_OFFSET_FILL);
			glPolygonOffset(0, 0);
			LineShader::ptr()->release();
		}
		doAfterPaint(info);
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