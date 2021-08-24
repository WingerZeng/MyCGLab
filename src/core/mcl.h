#pragma once

// Global Include Files
#include <type_traits>
#include <algorithm>
#include <cinttypes>
#include <cmath>
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <vector>
#include <assert.h>
#include <string.h>
#include <QOpenGLFunctions_3_3_Core>

#ifndef BUILD_STATIC
# if defined(GLWIDGET_LIB)
#  define GLWIDGET_EXPORT Q_DECL_EXPORT
# else
#  define GLWIDGET_EXPORT Q_DECL_IMPORT
# endif
#else
# define GLWIDGET_EXPORT
#endif

//Global Definition
#define OPENGLCLASS QOpenGLFunctions_3_3_Core

//#define FLOAT_AS_DOUBLE
#pragma warning(disable:4267)
#pragma warning(disable:4100)
#pragma warning(disable:4305)
#pragma warning(disable:4819)

namespace mcl {
	// Global Forward Declarations
	template <typename T>
	class Vector2;
	template <typename T>
	class Vector3;
	template <typename T>
	class Point3;
	template <typename T>
	class Point2;
	template <typename T>
	class Normal3;
	template <typename T>
	class Bound2;
	template <typename T>
	class Bound3;
	class Transform;
	class PixelData;

	class Scene;
	class bpSolid;
	class bpFace;
	class bpLoop;
	class bpHalfEdge;
	class bpVertex;
	class bpEdge;
	class Primitive;
	class PPolygonMesh;
	class PTriMesh;
	class Light;
	class Camera;

	class Sampler;
	class RTCamera;
	class RTPrimitive;
	class RTGeometryPrimitive;
	class RTLight;
	class RTSurfaceLight;
	class Ray;
	class RTScene;
	class Film;
	struct HitRecord;
	struct SurfaceRecord;
	class Material;
	class Bsdf;
	class BsdfGroup;
	class Geometry;
	class TimeController;

#ifdef FLOAT_AS_DOUBLE
	typedef double Float;
#else
	typedef float Float;
#endif

	// Global Constants
	inline const Float MaxFloat = std::numeric_limits<Float>::max();
	inline const Float MinFloat = std::numeric_limits<Float>::min();
	inline const Float Infinity = std::numeric_limits<Float>::infinity();
	inline const int MaxInt = std::numeric_limits<int>::max();
	inline const Float MachineEpsilon = (std::numeric_limits<Float>::epsilon() * 0.5);
	inline const Float InvEpsilon = 1 / MachineEpsilon;
	inline const Float FloatZero = 1e2*MachineEpsilon;
	inline const Float RayTraceZero = 5e3*MachineEpsilon;
	inline const Float One_Epsilon = 1 - MachineEpsilon;
	inline const int MAX_LIGHT_COUNT = 10;
	inline const Float PI = 3.14159265358979323846f;
	inline const Float PI_2 = 3.14159265358979323846f * 2;
	inline const Float INVPI = 1 / PI;
	inline const Float SQRT2 = 1.4142135623731f;

	inline bool FloatEq(const Float& lhs, const Float& rhs) {
		return std::abs(lhs - rhs) <= FloatZero;
	}
}