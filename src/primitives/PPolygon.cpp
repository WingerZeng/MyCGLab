#include "PPolygon.h"
#include <gl/GLU.h>
#include "PaintInformation.h"
#include "utilities.h"
#include "Light.h"
#include "CadCore.h"
#include "mcl.h"
#include "shaders.h"
#include <thread>
#include <mutex>
namespace mcl {			

	PPolygon::~PPolygon()
	{
	}

	void PPolygon::initialize()
	{
		this->initializeOpenGLFunctions();

		if (!checkNormal(this->normal_)) {
			return;
		}

		if((tessPolygon(lps_, &tessPts_, &drawInfo_))) return;
		if (tessPts_.empty()) return;
	
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

		attr = LightShader::ptr()->attributeLocation("aPos");
		LightShader::ptr()->setAttributeBuffer(attr, GL_FLOAT, 0, 3, 0);
		LightShader::ptr()->enableAttributeArray(attr);
		
		for (int i = 0; i < lps_.size(); i++) {
			boundDrawInfo_.push_back(DrawSingleObjInfo(GL_LINE_LOOP,boundPts_.size()/3,lps_[i].size()));
			for (int j = 0; j < lps_[i].size(); j++) {
				boundPts_.push_back(lps_[i][j].x());
				boundPts_.push_back(lps_[i][j].y());
				boundPts_.push_back(lps_[i][j].z());
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

	void PPolygon::paint(PaintInfomation* info)
	{
		doBeforePaint(info);
		if (!readyToDraw) return;
		if (info->fillmode == FILL || info->fillmode == FILL_WIREFRAME) {
			QOpenGLShaderProgram* shader;
			auto viewNormal = QVector3D((info->viewMat).inverted().transposed()*QVector4D(QVector3D(normal_), 0));
			//让正面靠前
			if (viewNormal.z() < 0) {
				glEnable(GL_POLYGON_OFFSET_FILL);
				glPolygonOffset(4, 4);
			}
			if (info->lights.size() && !selected())
			{
				shader = LightShader::ptr();
				shader->bind();
				shader->setUniformValue("modelMat", QMatrix4x4());
				shader->setUniformValue("viewMat", info->viewMat);
				shader->setUniformValue("projMat", info->projMat);
				shader->setUniformValue("ourColor", color().x(), color().y(), color().z(), 1.0f);
				shader->setUniformValue("lightCount", GLint(info->lights.size()));
				//auto viewNormal = QVector3D((info->viewMat).inverted().transposed()*QVector4D(QVector3D(normal_), 0));
				//if (viewNormal.z() < 0) viewNormal = -viewNormal;
				shader->setUniformValue("normal", viewNormal);
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

			for (int i = 0; i < drawInfo_.size(); i++) {
				glDrawArrays(drawInfo_[i].type, drawInfo_[i].offset, drawInfo_[i].size);
			}

			glDisable(GL_POLYGON_OFFSET_FILL);
			glPolygonOffset(0, 0);

			shader->release();
		}
		if (info->fillmode == WIREFRAME || info->fillmode == FILL_WIREFRAME || !selected()) {  //选中时隐藏
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

			for (int i = 0; i < boundDrawInfo_.size(); i++) {
				glDrawArrays(boundDrawInfo_[i].type, boundDrawInfo_[i].offset, boundDrawInfo_[i].size);
			}

			glDisable(GL_POLYGON_OFFSET_FILL);
			glPolygonOffset(0, 0);
			LineShader::ptr()->release();
		}
		doAfterPaint(info);
	}

	bool PPolygon::checkNormal(Vec3f& normal)
	{
		if (lps_.empty()) return false;
		const auto&  pts = lps_[0];
		if (pts.size() <= 2) return false;
		
		//判断多边形是否在yz平面上
		int n = pts.size();
		double result = 0;
		for (int i = 1; i < pts.size(); i++) {
			int aaa = (i - 1) % n;
			result += abs(Normalize(pts[i%n] - pts[(i - 1 + n) % n]).dot(Vec3f(1, 0, 0)));
		}
		int axis;
		if (result > 0.5) axis = 0; //不在yz平面上，选择x轴
		else axis = 1; //否则选择y轴

		double maxVal = -Infinity;
		int maxPt = -1;
		for (int i = 0; i < pts.size();i++) {
			if (pts[i][axis] > maxVal) {
				maxVal = pts[i][axis];
				maxPt = i;
			}
		}

		Vec3f trueNormal = Normalize((pts[maxPt] - pts[(maxPt - 1 + n) % n]).cross(pts[(maxPt + 1) % n] - pts[maxPt]));

		bool first = 1;
		result = 0;
		int cnt = 0;
		for (const auto& lp : lps_) {
			for (int i = 1; i < pts.size() - 1; i++) {
				Vec3f tempNormal = (pts[i] - pts[(i - 1 + n) % n]).cross(pts[(i + 1) % n] - pts[i]); //
				if (tempNormal.length() < 1e-7) continue;
				tempNormal.normalize();
				result += abs(tempNormal.dot(trueNormal));
				cnt++;
				if (first) {
					first = false;
				}
			}
		}

		if (first) return false;
		else if (abs(cnt - result)/cnt > 1e-6) {
			return false;
		}
		normal = trueNormal;

		return true;
	}

	//vrt::Bounds3f PPolygon::getBound()
	//{
	//	Bounds3f bd;
	//	for (const auto& lp : lps_)
	//	{
	//		for (const auto& pt : lp)
	//		{
	//			bd = Union(bd, Point3f(pt));
	//		}
	//	}
	//	return bd;
	//}

	std::mutex tessMtx;
	//由于glu库原因，此处只能用全局变量，来给回调函数提供储存顶点坐标信息的位置
	std::vector<Float>* glbTessPts;
	std::vector<DrawSingleObjInfo>* glbDrawInfo;
	bool glbError;

	void CALLBACK vertexCallback(GLvoid* vertex)
	{
		double* cd = (double*)vertex;
		glbTessPts->push_back(cd[0]);
		glbTessPts->push_back(cd[1]);
		glbTessPts->push_back(cd[2]);
	}

	void CALLBACK beginCallback(GLenum type)
	{
		if (!glbDrawInfo->empty()) glbDrawInfo->back().size = (glbTessPts->size() - glbDrawInfo->back().offset * 3)/3;
		glbDrawInfo->emplace_back();
		glbDrawInfo->back().type = type;
		glbDrawInfo->back().offset = glbTessPts->size()/3;
	}

	void CALLBACK endCallback()
	{

	}

	void CALLBACK errorCallback(GLenum errorCode)
	{
		glbError = true;
		qDebug() << "error:" << errorCode;
	}

	mcl::Normal3f calPolygonNormal(const std::vector<int>& lp, const std::vector<Point3f>& pts)
	{
		std::unique_ptr<Normal3f[]> normals(new Normal3f[lp.size()]);
		std::unique_ptr<bool[]> vflag(new bool[lp.size()]);
		int size = lp.size();
		//首先计算各节点处的法向
		for (int i = 0; i < lp.size(); i++) {
			Vec3f e1 = pts[lp[(i + 1) % size]] - pts[lp[(i) % size]];
			Vec3f e2 = pts[lp[(i + 2) % size]] - pts[lp[(i + 1) % size]];
			normals[i] = (Normal3f)(e1.cross(e2));
			normals[i].normalize();
		}
		int nNeg = 0, nPos = 0;
		Normal3f totalnormal(0);
		//计算法向之和
		for (int i = 0; i < size; i++) {
			totalnormal += normals[i];
		}
		//以总法向作为标准，筛选哪些法向是凸点处的法向
		for (int i = 0; i < size; i++) {
			if (normals[i].dot(totalnormal) > 0) {
				vflag[i] = true;
				nPos++;
			}
			else {
				vflag[i] = false;
				nNeg++;
			}
		}
		bool flag = nPos >= nNeg;
		Normal3f result;
		//凸点处法向取正，凹点处取负
		for (int i = 0; i < size; i++) {
			if (flag == vflag[i]) {
				result += normals[i];
			}
			else {
				result -= normals[i];
			}
		}
		result.normalize();
		return result;
	}

	int tessPolygon(const std::vector<std::vector<PType3f>>& lps, std::vector<Float>* tessPts, std::vector<DrawSingleObjInfo>* drawInfo)
	{
		std::vector<std::vector<std::vector<PType3f>>> plgs{lps};
		return tessPolygons(plgs, tessPts, drawInfo);
	}

	int tessPolygons(const std::vector<std::vector<std::vector<PType3f>>>& plgs, std::vector<Float>* tessPts, std::vector<DrawSingleObjInfo>* drawInfo)
	{
		GLUtesselator * tessobj;
		tessobj = gluNewTess();
		tessPts->clear();
		drawInfo->clear();
		//注册回调函数  
		gluTessCallback(tessobj, GLU_TESS_VERTEX, (void (CALLBACK *)())vertexCallback);
		gluTessCallback(tessobj, GLU_TESS_BEGIN, (void (CALLBACK *)())beginCallback);
		gluTessCallback(tessobj, GLU_TESS_END, (void (CALLBACK *)())endCallback);
		gluTessCallback(tessobj, GLU_TESS_ERROR, (void (CALLBACK *)())errorCallback);

		gluTessProperty(tessobj, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_POSITIVE);
		//gluTessNormal(tessobj, normal_.x(), normal_.y(), normal_.z());

		tessMtx.lock();
		glbTessPts = tessPts;
		glbDrawInfo = drawInfo;
		glbError = false;

		int count = 0;
		for (const auto& lps : plgs) {
			for (const auto& lp : lps)
			{
				for (const PType3f& pt : lp)
				{
					++count; //统计所有顶点数
				}
			}
		}

		//这里要用一个局部保留的空间来存储顶点值，不能用临时数组，也不能用一个空vector不断push_back来存，因为pushback过程中地址会变
		std::vector<GLdouble> tempCd(count * 3);

		count = 0;
		for (const auto& plg : plgs) {
			gluTessBeginPolygon(tessobj, NULL);
			for (const auto& lp : plg)
			{
				gluTessBeginContour(tessobj);//设置多边形的边线 	
				for (const PType3f& pt : lp)
				{
					tempCd[count * 3 + 0] = pt.x();
					tempCd[count * 3 + 1] = pt.y();
					tempCd[count * 3 + 2] = pt.z();
					gluTessVertex(tessobj, &tempCd[count * 3], &tempCd[count * 3]);
					++count;
				}
				gluTessEndContour(tessobj);
			}
			gluTessEndPolygon(tessobj);
		}

		gluDeleteTess(tessobj); 
		glbDrawInfo->back().size = (glbTessPts->size() - glbDrawInfo->back().offset * 3)/3;
		tessMtx.unlock();
		if (glbError) 
			return -1;
		return 0;
	}

	std::vector<std::shared_ptr<mcl::PPolygon>> solidToPolygons(bpSolid* solid)
	{
		bpFace* fc = solid->getFace();
		std::vector<std::shared_ptr<mcl::PPolygon>> plgs;
		for (auto it = fc->begin(); it != fc->end(); it++) {
			std::vector<std::vector<Point3f>> lps;
			for (auto lpit = (*it)->Floops()->begin(); lpit != (*it)->Floops()->end(); lpit++) {
				lps.push_back(std::vector<Point3f>());
				bpHalfEdge* he = (*lpit)->getFirstHalfEdge();
				bpHalfEdge* firstHe = he;
				do
				{
					lps.back().push_back(he->getBeginVtx()->getCoord());
					he = he->Nxt();
				} while (he != firstHe);
			}
			plgs.emplace_back(new mcl::PPolygon(lps));
		}
		return plgs;
	}
}
