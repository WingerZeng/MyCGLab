#pragma once
#include "mcl.h"
#include "types.h"
#include <QtGui/QOpenGLVertexArrayObject>
#include <QtGui/QOpenGLBuffer>
#include "Primitive.h"
#include "PPolygon.h"
namespace mcl {
	class PTriMesh;

	class PPolygonMesh : public GeometryPrimitive
	{
		RTTI_CLASS(PPolygonMesh)
	public:
		struct Polygon {
			std::vector<std::vector<int>> lps_;
		};
	public:
		PPolygonMesh(const std::vector<Polygon>& plgs,const std::vector<Point3f>& pts) //multi loop
			:plgs_(plgs),pts_(pts){
			setColor(Vector3f{ 0.6,0.6,0.6 });
			buildTessPts();
		};
		PPolygonMesh(std::shared_ptr<PTriMesh> trimesh);
		~PPolygonMesh();

		int toTriangleMesh(std::vector<int>& tris, std::vector<Point3f>& pts);

		virtual void initializeGL() override;

		const std::vector<Polygon>& getPlgs() { return plgs_; }
		const std::vector<Point3f>& getPts() { return pts_; }

		virtual std::unique_ptr<Primitive> clone();

		std::shared_ptr<QOpenGLVertexArrayObject> getVAO() { return vao; };
		std::shared_ptr<QOpenGLBuffer> getVBO() { return vbo; };
		std::shared_ptr<QOpenGLBuffer> getLineVBO() { return linevbo; };

		const std::vector<DrawSingleObjInfo>& getDrawInfo() { return drawInfo_; }
		const std::vector<DrawSingleObjInfo>& getBoundDrawInfo() { return boundDrawInfo_; }

	private:
		int buildTessPts();

		std::vector<Polygon> plgs_;
		std::vector<Point3f> pts_;

		std::vector<Float> boundPts_;
		std::vector<DrawSingleObjInfo> boundDrawInfo_; //记录每条边线在boundPts中的offset
		std::vector<Float> tessPts_;
		std::vector<DrawSingleObjInfo> drawInfo_;

		std::shared_ptr<QOpenGLBuffer> vbo;
		std::shared_ptr<QOpenGLBuffer> linevbo;
		std::shared_ptr<QOpenGLVertexArrayObject> vao;
	};

	//#PERF2 将命令接口整合到一个文件中，并清理文件中的包含Scene等头文件
	int triangulatePolygonMesh(int id);
}