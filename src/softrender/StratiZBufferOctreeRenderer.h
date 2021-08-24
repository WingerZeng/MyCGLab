#pragma once
#include "mcl.h"
#include "types.h"
#include "StratiZBufferRenderer.h"
#include "OcTree.h"

namespace mcl {
	class StratiZBufferOctreeRenderer:public StratiZBufferRenderer
	{
	public:
		StratiZBufferOctreeRenderer(Scene* scene, int px, int py);
		using StratiZBufferRenderer::StratiZBufferRenderer;

		virtual int paint(PaintInfomation* info = nullptr);

	private:
		struct MyPolygon
		{
			RasPolygon data;	//光栅化用多边形数据
			Bound3f sbound;  //屏幕坐标系下的包围盒
			Bound3f wbound;  //世界坐标系下的包围盒
			bool isDrawn;	 //是否已经绘制过
		};
		struct PlgBoundFunc {
			Bound3f operator()(MyPolygon* rhs) {
				return rhs->wbound;
			}
		};

		typedef Octree<MyPolygon*, PlgBoundFunc> octree_t;
		typedef typename Octree<MyPolygon*, PlgBoundFunc>::node_t ocnode_t;

		octree_t tree;
	};
}

