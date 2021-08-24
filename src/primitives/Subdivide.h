#pragma once
#include "mcl.h"
#include "types.h"
#include "Primitive.h"
namespace mcl {
	/* �����нӿڣ���ϸ�ֺ���ӳ�䵽������ */
	int dooSabinSubdivPolygonMesh(int id, int nlevels);

	int catmullClarkSubdivPolygonMesh(int id, int nlevels);

	int dooSabinSubdivTriangles(int id, int nlevels);

	int catmullClarkSubdivTriangles(int id, int nlevels);

	int loopSubdivideTriangles(int id, int nlevels, bool toLimit);
	/**
	 * @brief ��������������DooSabinϸ��
	 */
	std::vector<std::shared_ptr<Primitive>> dooSabinSubdivide(int nLevels, std::shared_ptr<PPolygonMesh> plgMesh);

	/**
	 * @brief ��������������CatmullClarlϸ��
	 */
	std::vector<std::shared_ptr<Primitive>> catmullClarkSubdivide(int nLevels, std::shared_ptr<PPolygonMesh> plgMesh);

	/**
	 * @brief �������������DooSabinϸ��
	 */
	std::vector<std::shared_ptr<Primitive>> dooSabinSubdivideTri(int nLevels, std::shared_ptr<PTriMesh> triMesh);

	/**
	 * @brief �������������CatmullClarkϸ��
	 */
	std::vector<std::shared_ptr<Primitive>> catmullClarkSubdivideTri(int nLevels, std::shared_ptr<PTriMesh> triMesh);

	/**
	 * @brief �������������Loopϸ��
	 */
	std::vector<std::shared_ptr<Primitive>> loopSubdivideTri(int nLevels, std::shared_ptr<PTriMesh> triMesh, bool toLimit);
}