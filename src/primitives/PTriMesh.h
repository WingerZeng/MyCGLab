#pragma once
#include "mcl.h"
#include "types.h"
#include "Primitive.h"
#include <QtGui/QOpenGLVertexArrayObject>
#include <QtGui/QOpenGLBuffer>
#include <QOpenGLShaderProgram>
namespace mcl {
	class PTriMesh:public GeometryPrimitive
	{
		RTTI_CLASS(PTriMesh)
	public:
		PTriMesh(const std::vector<int>& tris, const std::vector<Point3f>& pts);
		PTriMesh(const std::vector<int>& tris, const std::vector<Point3f>& pts, const std::vector<Point2f>& uvs, const std::vector<Normal3f>& normals);

		virtual void initializeGL() override;
		void initialize(QOpenGLShaderProgram* shader);
		void initializeLine(QOpenGLShaderProgram* shader);

		const std::vector<int>& getIndices() const { return tris_; }
		const std::vector<Float>& getPts() const { return pts_; }

		std::unique_ptr<Primitive> clone() override;

		virtual std::shared_ptr<Geometry> createGeometry() override;

		Point3f getCentroid();
		virtual Bound3f getBound() override;

		virtual void initAll() override;

		void generateNormal();

		virtual void paint(PaintInfomation* info, PaintVisitor* visitor) override;
		
		std::shared_ptr<QOpenGLVertexArrayObject> getVAO();
		std::shared_ptr<QOpenGLVertexArrayObject> getLineVAO();

		int getTriNumer();

		int getEdgeNumber();

		bool hasNormal() { return !normals.empty(); }

		static std::unique_ptr<PTriMesh> createBillBoard();
	private:
		
		std::vector<int> tris_;
		std::vector<Float> pts_;

		std::vector<Point2f> uvs;
		std::vector<Normal3f> normals;

		std::unique_ptr<Float[]>  rawPts;//将pts和indices合并
		std::unique_ptr<Float[]>  rawUvs;
		std::unique_ptr<Float[]>  rawNormals;
		std::unique_ptr<Float[]>  linepts_;
		int nEgs;

		std::shared_ptr<QOpenGLBuffer> vbo;
		std::shared_ptr<QOpenGLBuffer> linevbo;
		std::shared_ptr<QOpenGLVertexArrayObject> vao;
		std::shared_ptr<QOpenGLVertexArrayObject> linevao;
	};
}