#include "FileIO.h"
#include <QString>
#include <fstream>
#include <set>
#include "Primitive.h"
#include "primitives/PTriMesh.h"
#include "Light.h"
#include "utilities.h"
#include "Material.h"
namespace mcl {

	static const std::set<std::string> ignoreMtlKey = {
		"Ni","d","Ka","Tr","Ts","Tf"
	};

	std::map<std::string, std::shared_ptr<DataNode>> readMtl(QString path)
	{
		std::ifstream fin(path.toStdString());
		if (!fin) {
			LOG(FATAL) << "Error file path!";
		}

		std::map<std::string, std::shared_ptr<DataNode>> ret;
		std::string curKey;

		QFileInfo info(path);
		QDir dir = info.absoluteDir();

		struct MatStatus
		{
			QString getMatType() {
				return Material::matTypeToName[Material::getTypeByParameters(hasKey)];
			}

			std::set<QString> hasKey;
		}status;

		while (1)
		{
			std::string title;
			std::string line;

			if (!getline(fin, line)) {
				break;
			}

			if (line == "") {
				continue;
			}
			std::stringstream sin(line);
			sin >> title;
			if (title == "#") continue;
			if (ignoreMtlKey.find(title)!=ignoreMtlKey.end()) continue;
			if (title == "newmtl") {
				if (curKey != "") {
					ret[curKey]->fd("Mat","")->fd("type", status.getMatType());
				}
				sin >> curKey;
				status = MatStatus();
				ret[curKey] = std::make_shared<DataNode>();
				ret[curKey]->setK(QString::fromStdString(curKey));
			}
			else if (title == "map_Bump") {
				std::string path;
				sin >> path;
				ret[curKey]->fd("Mat", "")->fd("map_Bump", dir.absoluteFilePath(QString::fromStdString(path)));
			}
			else if (title == "illum") {
				int type_;
				sin >> type_; //目前默认illum=2处理
			}
			else if (title == "Le" || title == "Ke") {
				ret[curKey]->fd("Mat", "")->fd("Le", QString::fromStdString(readRemainAll(sin)));
			}
			else { //common material parameters
				if (title.substr(0, 3) == "map")
				{
					std::string path;
					sin >> path;
					ret[curKey]->fd("Mat", "")->fd("Pars", "")->fd(QString::fromStdString(title), dir.absoluteFilePath(QString::fromStdString(path)));
					status.hasKey.insert(QString::fromStdString(title.substr(4)));
				}
				else {
					ret[curKey]->fd("Mat", "")->fd("Pars", "")->fd(QString::fromStdString(title), QString::fromStdString(readRemainAll(sin)));
					status.hasKey.insert(QString::fromStdString(title));
				}
			}
		}

		if (curKey != "") {
			ret[curKey]->fd("Mat", "")->fd("type", status.getMatType());
		}

		return ret;
	}

	void readObj(QString path, std::vector < std::shared_ptr<Primitive>>& primitives) {
		std::ifstream fin(path.toStdString());
		if (!fin) {
			LOG(FATAL) << "Error file path!";
		}

		QFileInfo info(path);
		QDir dir = info.absoluteDir();
		std::map<std::string, std::shared_ptr<DataNode>> mtls;
		std::vector<Normal3f> normals;
		std::vector<Point3f> pts;
		std::vector<Normal3f> fcNormals;
		std::vector<Point2f> uvs;
		std::vector<Point2f> fcUvs;
		std::vector<int> indices;

		int vnoffset = 0;
		int voffset = 0;
		int vtoffset = 0;

		std::shared_ptr<DataNode> rtdata;
		Color3f color;
		bool hasMat = false;
		bool hasFace = false;

		auto setupMesh = [&]() {
			// 根据信息构造三角网格
			auto tri = std::make_shared<PTriMesh>(indices, pts, fcUvs, fcNormals);
			tri->setMaterialData(rtdata);
			primitives.emplace_back(tri);

			// 初始化
			voffset += pts.size();
			vnoffset += normals.size();
			vtoffset += uvs.size();
			normals.clear();
			pts.clear();
			fcNormals.clear();
			uvs.clear();
			fcUvs.clear();
			indices.clear();
			color = Color3f(0);
			rtdata->reset();
			hasMat = false;
			hasFace = false;
		};

		while (1)
		{
			std::string title;
			std::string line;

			if (!getline(fin, line)) {
				if (hasMat)
					setupMesh();
				break;
			}

			std::stringstream sin(line);
			sin >> title;

			if (((line == "" || title == "o") && hasMat) || (title == "g" && hasFace)){
				setupMesh();
			}

			if (title == "#") continue;
			else if (title == "mtllib") {
				std::string mtlname;
				sin >> mtlname;
				QString mtlpath = dir.absoluteFilePath(QString::fromStdString(mtlname));
				mtls = readMtl(mtlpath);
			}
			else if (title == "v") {
				pts.emplace_back(sin);
			}
			else if (title == "vn") {
				normals.emplace_back(sin);
			}
			else if (title == "vt") {
				uvs.emplace_back(sin);
			}
			else if (title == "g") {
				continue;
			}
			else if (title == "usemtl") {
				std::string mtlname;
				sin >> mtlname;
				rtdata = mtls[mtlname];
				hasMat = true;
			}
			else if (title == "f") {
				// 目前仅限于Triangles
				char c;
				int v[4], vt[4], vn[4];
				int i= 0;
				for (i; i < 4; i++) {
					sin >> v[i];
					sin >> c;
					sin >> vt[i];
					sin >> c;
					sin >> vn[i];
					if (!sin) {
						break;
					}
				}
				if (i == 3) {
					//三角形网格
					for (int i = 0; i < 3; i++) {
						indices.emplace_back(v[i] - 1 - voffset);
						fcUvs.emplace_back(uvs[vt[i] - 1 - vtoffset]);
						fcNormals.emplace_back(normals[vn[i] - 1 - vnoffset]);
					}
				}
				else if (i == 4) {
					//四边形网格
					const int iv[2][3] = { {0,1,3},{1,2,3} };
					for (int i = 0; i < 2; i++) {
						for (int j = 0; j < 3; j++) {
							indices.emplace_back(v[iv[i][j]] - 1 - voffset);
							fcUvs.emplace_back(uvs[vt[iv[i][j]] - 1 - vtoffset]);
							fcNormals.emplace_back(normals[vn[iv[i][j]] - 1 - vnoffset]);
						}
					}
				}
				else {
					LOG(FATAL) << "Error polygon type!";
				}
				hasFace = true;
			}
		}

		CHECK(pts.empty());

		return;
	}
}