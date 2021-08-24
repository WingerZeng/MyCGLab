#pragma once
#include "mcl.h"
#include "types.h"
#include "database.h"
namespace mcl {
	std::map<std::string, std::shared_ptr<DataNode>> readMtl(QString path);
	void readObj(QString path, std::vector < std::shared_ptr<Primitive>>& prims, std::vector<std::shared_ptr<Light>>& lights);
}

