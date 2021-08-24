#pragma once
#include "mcl.h"
#include "types.h"
namespace mcl {

	struct SurfaceRecord
	{
		SurfaceRecord(){
		}

		virtual ~SurfaceRecord() {};

		Point3f pos;
		Normal3f n;
		Normal3f bumped_n;
		Point2f uv;
		Vector3f uvec; //u����
		Vector3f vvec; //v����

		void transform(Transform tran) {
			pos = tran(pos);
			n = Normalize(tran(n));
			uvec = tran(uvec);
			vvec = tran(vvec);
		}
	};

	struct HitRecord :public SurfaceRecord
	{
		double t;
		RTPrimitive* prim;
	};

}

