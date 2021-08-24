#pragma once
#include "mcl.h"

namespace mcl {

	/***************
	* 数学函数
	**************/
	template<typename T>
	inline T sigmoid(T x) {
		return 1.0 / (1 + exp(-(2 * x - 1)))*2.164 - 0.582;
	}

	inline Float degreeToRad(const Float& deg) {
		return deg / 180 * PI;
	}

	template<typename T>
	inline T interpolate(const T& val1, const T& val2, const Float& ratio) {
		return T((val2 - val1) * ratio + val1);
	}

	template<typename T>
	inline T pow5(const T& val) {
		return val * val*val*val*val;
	};

	template<typename T>
	inline T pow2(const T& val) {
		return val * val;
	};
	/***************
	* 几何函数
	**************/
	inline Vector3f calHalfVec(const Vector3f& vec1, const Vector3f& vec2) {
		DCHECK(vec1.normalized() && vec2.normalized());
		return Normalize(vec1 + vec2);
	}

	inline Float calAbsCosTheta(const Vector3f& vec) {
		return std::abs(vec.z());
	}

	inline Float calAbsTanTheta(const Vector3f& vec) {
		return std::abs(sqrt(vec.x()*vec.x() + vec.y()*vec.y()) / vec.z());
	}

	inline Float calCosPhi(const Vector3f& vec) {
		CHECK(!(vec.z()==1));
		return vec.x() / std::sqrt(vec.x()*vec.x() + vec.y() * vec.y());
	}

	inline Float calPhi(const Vector3f& vec) {
		Float phi0_pi = std::acos(vec.x() / std::sqrt(vec.x()*vec.x() + vec.y() * vec.y()));
		return vec.y() >= 0 ? phi0_pi : PI_2 - phi0_pi;
	}

	inline Float calTheta(const Vector3f& vec) {
		return std::asin(vec.z() / vec.length());
	}

	inline Point2f calUvInSphere(const Vector3f vec) {
		return Point2f(calPhi(vec) * INVPI * 0.5, (calTheta(vec) + PI * 0.5) * INVPI);
	}

	inline bool isInversePhi(const Vector3f& vec1, const Vector3f& vec2) {
		return FloatEq(vec1.x() * vec2.y(), vec1.y() *vec2.x());
	}

	inline bool isMirror(const Vector3f& out, const Vector3f& in) {
		return in.x() == -out.x() && in.y() == -out.y() && in.z() == out.z();
	}

	inline bool isReflect(const Vector3f& out, const Vector3f& in) {
		return out.z() * in.z() > 0;
	}

	inline bool isRefract(const Vector3f& out, const Vector3f& in) {
		return out.z() * in.z() < 0; //暂时不支持临界反射
	}

	inline bool isRefract(const Vector3f& out, const Vector3f& in, Float eta1, Float eta2)
	{
		if (!isRefract(out, in)) return false;
		if (out.z() < 0) //如果考虑到边界情况，则要考虑z==0的情况
		{
			std::swap(eta1, eta2);
		}
		return (isInversePhi(out, in)) && (FloatEq((1 - out.z()*out.z())*eta1*eta1, (1 - in.z()*in.z())*eta2*eta2));
	}

	inline Vector3f calReflect(const Vector3f& out) {
		return Vector3f(-out.x(), -out.y(), out.z());
	}

	// 调用前保证normal为单位向量
	inline Vector3f reflectByNormal(const Vector3f& out, const Vector3f& normal) {
		return -out + normal * normal.dot(out) * 2;
	}

	inline Vector3f SphereToXYZ(const Float& phi, const Float& theta) {
		using namespace std;
		return Vector3f(cos(phi) * sin(theta), sin(phi) * sin(theta), cos(theta));
	}
	/***************
	* 采样工具函数
	**************/

	//返回单位球上采样点
	Point3f uniformSampleSphere(const Point2f& sample);
}