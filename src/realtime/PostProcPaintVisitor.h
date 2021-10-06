#pragma once
#include "mcl.h"
#include "types.h"
#include "PaintVisitor.h"
namespace mcl {
	class PostProcPaintVisitor: public PaintVisitor
	{
	};

	class GammaPaintVisitor : public PostProcPaintVisitor
	{
	public:
		GammaPaintVisitor(GLfloat gamma = 2.2);

		virtual int paintTris(PaintInfomation* info, PTriMesh* tri) override;

		virtual int initTris(PTriMesh* tri) override;

	private:
		GLfloat gamma;
	};

	class DeferedPaintVisitor : public PostProcPaintVisitor
	{
	public:
		virtual int paintTris(PaintInfomation* info, PTriMesh* tri) override;

		virtual int initTris(PTriMesh* tri) override;
	};
}

