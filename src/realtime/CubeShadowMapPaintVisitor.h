#pragma once
#include "mcl.h"
#include "types.h"
#include "PaintVisitor.h"
namespace mcl {
	class CubeShadowMapPaintVisitor: public PaintVisitor
	{
	public:
		CubeShadowMapPaintVisitor(std::shared_ptr<PointLight> light);

		virtual int paintTris(PaintInfomation* info, PTriMesh* tri) override;

	private:
		std::shared_ptr<PointLight> light;
	};
}

