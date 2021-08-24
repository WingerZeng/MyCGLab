#pragma once
#include "mcl.h"
#include "types.h"
#include "Primitive.h"
namespace mcl {
	/* 命令行接口，将细分函数映射到命令行 */
	int dooSabinSubdivPolygonMesh(int id, int nlevels);

	int catmullClarkSubdivPolygonMesh(int id, int nlevels);

	int dooSabinSubdivTriangles(int id, int nlevels);

	int catmullClarkSubdivTriangles(int id, int nlevels);

	int loopSubdivideTriangles(int id, int nlevels, bool toLimit);
	/**
	 * @brief 将多边形网格进行DooSabin细分
	 */
	std::vector<std::shared_ptr<Primitive>> dooSabinSubdivide(int nLevels, std::shared_ptr<PPolygonMesh> plgMesh);

	/**
	 * @brief 将多边形网格进行CatmullClarl细分
	 */
	std::vector<std::shared_ptr<Primitive>> catmullClarkSubdivide(int nLevels, std::shared_ptr<PPolygonMesh> plgMesh);

	/**
	 * @brief 将三角网格进行DooSabin细分
	 */
	std::vector<std::shared_ptr<Primitive>> dooSabinSubdivideTri(int nLevels, std::shared_ptr<PTriMesh> triMesh);

	/**
	 * @brief 将三角网格进行CatmullClark细分
	 */
	std::vector<std::shared_ptr<Primitive>> catmullClarkSubdivideTri(int nLevels, std::shared_ptr<PTriMesh> triMesh);

	/**
	 * @brief 将三角网格进行Loop细分
	 */
	std::vector<std::shared_ptr<Primitive>> loopSubdivideTri(int nLevels, std::shared_ptr<PTriMesh> triMesh, bool toLimit);
}