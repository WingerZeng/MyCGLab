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
			RasPolygon data;	//��դ���ö��������
			Bound3f sbound;  //��Ļ����ϵ�µİ�Χ��
			Bound3f wbound;  //��������ϵ�µİ�Χ��
			bool isDrawn;	 //�Ƿ��Ѿ����ƹ�
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

