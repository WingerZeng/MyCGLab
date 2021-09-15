#include "Material.h"
#include "materials/Lambertain.h"
#include "materials/BlinnPhongMaterial.h"
#include "materials/Disney.h"
#include "materials/Mirror.h"
#include "materials/Dielectric.h"
#include "Texture.h"
#include "algorithms.h"
namespace mcl{

	bool Material::hasEmission() const
	{
		return emiss.length() > FloatZero;
	}

	mcl::Color3f Material::emission() const
	{
		return emiss;
	}

	void Material::bumpNormal(HitRecord* rec) const
	{
		if (bumpmap) {
			Color3f bump = bumpmap->value(rec->uv);
			bump = bump * 2 - 1;
			bump.normalize();
			rec->bumped_n = Normal3f(bump.x() * Normalize(rec->uvec) + bump.y() * Normalize(rec->vvec)+ bump.z() * Vector3f(rec->n));
		}
		else {
			rec->bumped_n = rec->n;
		}
	}

	void Material::initGL()
	{
		LOG(FATAL) << "Unimplemented method!";
	}

	void Material::prepareGL(QOpenGLShaderProgram* shader)
{
		LOG(FATAL) << "Unimplemented method!";
	}

	const std::array<QString, mcl::Material::MeterialTypeSize> Material::matTypeToName = {
			"Lambertain",
			"BlinnPhong",
			"Mirror",
			"Dielectric",
			"Disney"
	};

	const std::map<QString, mcl::Material::MeterialType> Material::matNameToType = {
			std::pair<QString, MeterialType>("Lambertain", Lambertain),
			std::pair<QString, MeterialType>("BlinnPhong", BlinnPhong),
			std::pair<QString, MeterialType>("Mirror", Mirror),
			std::pair<QString, MeterialType>("Dielectric", Dielectric),
			std::pair<QString, MeterialType>("Disney", Disney)
	};

	const std::array<std::vector<std::pair<QString, Material::ParameterType>>, Material::MeterialTypeSize> Material::matparvec = {
		//Lambertain
		parvec_t{parpair_t("Kd",P_ColorTex)},
		//BlinnPhong				
		parvec_t{parpair_t("Kd",P_ColorTex),parpair_t("Ks",P_ColorTex),parpair_t("Ns",P_FloatTex)},
		//Mirror				
		parvec_t{parpair_t("Kr",P_ColorTex)},
		//Dielectric		
		parvec_t{parpair_t("Kr",P_ColorTex),parpair_t("Kt",P_ColorTex),parpair_t("Eta1",P_Float),parpair_t("Eta2",P_Float)},
		//Disney		
		parvec_t{parpair_t("BaseColor",P_ColorTex),parpair_t("metallic",P_FloatTex),parpair_t("subsurface",P_FloatTex),
			parpair_t("specular",P_FloatTex),parpair_t("specularTint",P_FloatTex),parpair_t("roughness",P_FloatTex),
			parpair_t("anisotropic",P_FloatTex),parpair_t("sheen",P_FloatTex),parpair_t("sheenTint",P_FloatTex),
			parpair_t("clearcoat",P_FloatTex),parpair_t("clearcoatGloss",P_FloatTex)}
	};

//#define LOADTEXTURE(name)\
//	auto name = data->fd(#name);\
//	std::shared_ptr<Texture<Color3f>> tex_##name;\
//	if (name) {\
//		Color3f c = name->toVector3f();\
//		auto map##name = data->fd("map_"#name);\
//		if(map##name){\
//			/* 同时有贴图纹理和常数纹理时 */ \
//			QString mappath = map##name->getV();\
//			tex_##name = std::make_shared<ProductTexture<Color3f>>(\
//							std::make_shared<ConstantTexture<Color3f>>(c),\
//							std::make_shared<PixelMapTexture>(mappath));\
//		}\
//		else{\
//			/* 只有常数纹理时 */ \
//			tex_##name = std::make_shared<ConstantTexture<Color3f>>(c);\
//		}\
//	}\
//	else {\
//		/* 只有贴图纹理时 */ \
//		QString mappath = data->fd("map_"#name)->getV();\
//		tex_##name = std::make_shared<PixelMapTexture>(mappath);\
//	}
//
//
//	std::shared_ptr<vrt::Material> createMaterialByData(std::shared_ptr<DataNode> data)
//	{
//		QString type = data->fd("type")->getV();
//		if (type == "Lambertain") {
//			LOADTEXTURE(Kd);
//			DisneyMaterialInfo info;
//			info.baseColor = tex_Kd;
//			return std::make_shared<DisneyMaterial>(info);
//		}
//		else if (type == "BlinnPhong") {
//			LOADTEXTURE(Kd);
//			LOADTEXTURE(Ks);
//			//std::shared_ptr<Texture<Float>> tex_Ns = std::make_shared<ConstantTexture<Float>>(data->fd("Ns")->toDouble());
//			//if(tex_Ks->value(Point2f(0.5,0.5)).length() == 0)
//			//	return std::make_shared<LambertainMaterial>(tex_Kd);
//			//return std::make_shared<BlinnPhongMaterial>(tex_Kd, tex_Ks, tex_Ns);
//			DisneyMaterialInfo info;
//			if (tex_Ks->value(Point2f(0.5, 0.5)).length() == 0) {
//				info.baseColor = tex_Kd;
//			}
//			else {
//				info.baseColor = std::make_shared<AddTexture<Color3f>>(tex_Ks, tex_Kd);
//				Float specular = std::clamp(data->fd("Ns")->toDouble() / 60, 0.0, 1.0);
//				Float metalic = std::clamp(data->fd("Ns")->toDouble() / 100, 0.0, 0.8);
//				Float clearcoat = std::clamp(data->fd("Ns")->toDouble() / 30, 0.0, 1.0);
//// 				Float specular = std::clamp(data->fd("Ns")->toDouble() / 128, 0.0, 1.0);
//// 				Float metalic = std::clamp(data->fd("Ns")->toDouble() / 256, 0.0, 0.8);
//// 				Float clearcoat = std::clamp(data->fd("Ns")->toDouble() / 64, 0.0, 1.0);
//				Float roughness = interpolate(0.8f, 0.05f ,specular);
//				info.specular = genTexture(specular);
//				info.metallic = genTexture(metalic);
//				info.clearcoat = genTexture(clearcoat);
//				info.roughness = genTexture(roughness);
//				info.subsurface = genTexture(0.8);
//			}
//			return std::make_shared<DisneyMaterial>(info);
//		}
//		else {
//			LOG(FATAL) << "Invalid material type!";
//		}
//	}
//
//
//#undef LOADTEXTURE


	std::shared_ptr<mcl::Material> Material::createMaterial(DataNode* node)
	{
		MeterialType t = matNameToType.find(node->fd("type")->getV())->second;
		DataNode* parnode = node->fd("Pars").get();
		std::shared_ptr<Material> ret;
		switch (t)
		{
		case mcl::Material::Lambertain:
			ret = std::make_shared <LambertainMaterial>(parnode);
			break;
		case mcl::Material::BlinnPhong:
			ret = std::make_shared <BlinnPhongMaterial>(parnode);
			break;
		case mcl::Material::Mirror:
			ret = std::make_shared <MirrorMaterial>(parnode);
			break;
		case mcl::Material::Dielectric:
			ret = std::make_shared <DielectricMaterial>(parnode);
			break;
		case mcl::Material::Disney:
			ret = std::make_shared <DisneyMaterial>(parnode);
			break;
		default:
			CHECK(0);
			break;
		}
		ret->_type = t;
		//处理bumpmap
		if(node->fd("map_Bump") && QFile(node->fd("map_Bump")->getV()).exists())
			ret->setBumpMap(getColorTexture("Bump", node));

		//处理Le
		if (node->fd("Le"))
			ret->setEmission(node->fd("Le")->toVector3f());
		return ret;
	}

	mcl::Material::MeterialType Material::getTypeByParameters(std::set<QString> parnames)
	{
		static bool first = true;
		static std::array<std::set<QString>, MeterialTypeSize> sets;
		if (first) {
			for (int i = 0; i < MeterialTypeSize; i++) {
				for (const auto& par : matparvec[i]) {
					sets[i].insert(par.first);
				}
			}
			first = false;
		}

		for (int i = 0; i < MeterialTypeSize; i++) {
			if (parnames == sets[i])
				return MeterialType(i + MeterialTypeBegin);
		}

		CHECK(0);
	}

	std::shared_ptr<mcl::Texture<mcl::Color3f>> Material::getColorTexture(QString name, DataNode* node)
	{
		std::shared_ptr<ConstantTexture<Color3f>> ct;
		auto cnode = node->fd(name);
		if (cnode) {
			ct = std::make_shared<ConstantTexture<Color3f>>(cnode->toVector3f());
		}
		QString mapname = "map_" + name;
		auto mapnode = node->fd(mapname);
		std::shared_ptr<PixelMapTexture> mt;
		if (mapnode && QFile(mapnode->getV()).exists()) {
			mt = std::make_shared<PixelMapTexture>(mapnode->getV());
		}

		if (mt && (!ct || cnode->toVector3f().length() == 0)) {
			return mt;
		}
		if (!mt && ct) {
			return ct;
		}
		if (mt && ct) {
			return std::make_shared<ProductTexture<Color3f>>(mt, ct);
		}
		else {
			CHECK(0);
		}
	}

	mcl::Material::ParameterType Material::getColorTexture(QString name, DataNode* node, QVector4D& color, QString& texture)
	{
		ParameterType ret = P_NULL;
		auto cnode = node->fd(name);
		if (cnode) {
			color = QVector4D(QVector3D(cnode->toVector3f()));
			ret = ParameterType(ret | P_Color);
		}
		QString mapname = "map_" + name;
		auto mapnode = node->fd(mapname);
		if (mapnode && QFile(mapnode->getV()).exists()) {
			texture = mapnode->getV();
			ret = ParameterType(ret | P_Map);
		}

		if ((ret&P_Map) && (!(ret&P_Color) || cnode->toVector3f().length() == 0)) {
			return P_Map;
		}
		return ret;
	}

	std::shared_ptr<mcl::Texture<mcl::Float>> Material::getFloatTexture(QString name, DataNode* node)
	{
		std::shared_ptr<ConstantTexture<Float>> ct;
		auto cnode = node->fd(name);
		if (cnode) {
			ct = std::make_shared<ConstantTexture<Float>>(cnode->toDouble());
		}
		std::shared_ptr<PixelMapTexture> mt;
		//暂时不支持Float纹理的贴图
		//QString mapname = "map_" + name;
		//auto mapnode = node->fd(mapname);
		//if (mapnode && QFile(mapnode->getV()).exists()) {
		//	mt = std::make_shared<PixelMapTexture>(mapnode->getV());
		//}

		if (!mt && ct) {
			return ct;
		}
		//if (mt && !ct) {
		//	return mt;
		//}
		//if (mt && ct) {
		//	return std::make_shared<ProductTexture<Float>>(mt, ct);
		//}
		else {
			CHECK(0);
		}
	}

	mcl::Material::ParameterType Material::getFloatTexture(QString name, DataNode* node, Float& value, QString& texture)
	{
		ParameterType ret = P_NULL;
		auto cnode = node->fd(name);
		if (cnode) {
			ret = ParameterType(ret | P_Color);
			value = cnode->toDouble();
		}
		//暂时不支持Float纹理的贴图
		//QString mapname = "map_" + name;
		//auto mapnode = node->fd(mapname);
		//if (mapnode && QFile(mapnode->getV()).exists()) {
		//	mt = std::make_shared<PixelMapTexture>(mapnode->getV());
		//}

		//if (mt && !ct) {
		//	return mt;
		//}
		//if (mt && ct) {
		//	return std::make_shared<ProductTexture<Float>>(mt, ct);
		//}
		return ret;
	}

	int Material::inputToNode(ReadRemainString input, DataNode* node)
	{
		std::string temp;
		std::stringstream sin(input.str);
		sin >> temp;
		auto it = matNameToType.find(QString::fromStdString(temp));
		if (it == matNameToType.end()) {
			return -2;
		}
		node->setChild("type", QString::fromStdString(temp));
		DataNode* parnode = node->fd("Pars", "").get();
		parnode->clear();
		MeterialType tp = it->second;
		const auto& parvec = matparvec[tp - MeterialTypeBegin];
		for (const auto& par : parvec) {
			if (par.second & P_Color) {
				Color3f val;
				if (!(sin >> val))
					return -1;
				parnode->setChild(par.first, QString::number(val.x()) + " " + QString::number(val.y()) + " " + QString::number(val.z()));
			}
			if (par.second & P_Float) {
				Float val;
				if (!(sin >> val))
					return -1;
				parnode->setChild(par.first, QString::number(val));
			}
			if (par.second & P_Map)
			{
				PathString path;
				if (!(sin >> path))
					return -1;
				if (!path.empty())
					if (path.exist())
						parnode->setChild("map_" + par.first, QString::fromStdString(path.str));
					else
						return -2;
			}
		}
		//处理bumpmap
		PathString path;
		if (sin >> path && path.exist()) {
			node->setChild("map_Bump", QString::fromStdString(path.str));
		}
		Color3f emiss;
		if (sin >> emiss) {
			node->setChild("Le", QString::number(emiss.x()) + " " + QString::number(emiss.y()) + " " + QString::number(emiss.z()));
		}
		return 0;
	}

}