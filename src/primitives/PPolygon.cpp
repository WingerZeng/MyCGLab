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
#include "GLFunctions.h"
namespace mcl {			
	std::mutex tessMtx;
	//����glu��ԭ�򣬴˴�ֻ����ȫ�ֱ����������ص������ṩ���涥��������Ϣ��λ��
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
		//���ȼ�����ڵ㴦�ķ���
		for (int i = 0; i < lp.size(); i++) {
			Vec3f e1 = pts[lp[(i + 1) % size]] - pts[lp[(i) % size]];
			Vec3f e2 = pts[lp[(i + 2) % size]] - pts[lp[(i + 1) % size]];
			normals[i] = (Normal3f)(e1.cross(e2));
			normals[i].normalize();
		}
		int nNeg = 0, nPos = 0;
		Normal3f totalnormal(0);
		//���㷨��֮��
		for (int i = 0; i < size; i++) {
			totalnormal += normals[i];
		}
		//���ܷ�����Ϊ��׼��ɸѡ��Щ������͹�㴦�ķ���
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
		//͹�㴦����ȡ�������㴦ȡ��
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
		//ע��ص�����  
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
					++count; //ͳ�����ж�����
				}
			}
		}

		//����Ҫ��һ���ֲ������Ŀռ����洢����ֵ����������ʱ���飬Ҳ������һ����vector����push_back���棬��Ϊpushback�����е�ַ���
		std::vector<GLdouble> tempCd(count * 3);

		count = 0;
		for (const auto& plg : plgs) {
			gluTessBeginPolygon(tessobj, NULL);
			for (const auto& lp : plg)
			{
				gluTessBeginContour(tessobj);//���ö���εı��� 	
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

	//std::vector<std::shared_ptr<mcl::PPolygon>> solidToPolygons(bpSolid* solid)
	//{
	//	bpFace* fc = solid->getFace();
	//	std::vector<std::shared_ptr<mcl::PPolygon>> plgs;
	//	for (auto it = fc->begin(); it != fc->end(); it++) {
	//		std::vector<std::vector<Point3f>> lps;
	//		for (auto lpit = (*it)->Floops()->begin(); lpit != (*it)->Floops()->end(); lpit++) {
	//			lps.push_back(std::vector<Point3f>());
	//			bpHalfEdge* he = (*lpit)->getFirstHalfEdge();
	//			bpHalfEdge* firstHe = he;
	//			do
	//			{
	//				lps.back().push_back(he->getBeginVtx()->getCoord());
	//				he = he->Nxt();
	//			} while (he != firstHe);
	//		}
	//		plgs.emplace_back(new mcl::PPolygon(lps));
	//	}
	//	return plgs;
	//}
}
