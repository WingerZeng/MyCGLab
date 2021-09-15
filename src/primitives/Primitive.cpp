#include "Primitive.h"
#include "PaintInformation.h"
#include "RTLight.h"
#include "RTPrimitive.h"
#include "Material.h"
#include "materials/Lambertain.h"
namespace mcl {

mcl::Bound3f Primitive::getBound()
{
	LOG(FATAL) << "Unimplemented method!";
}

void Primitive::copyAttribute(std::shared_ptr<Primitive> other)
{
	setLocalTransform(other->localTransform());
}

void Primitive::initMaterial()
{
	LOG(FATAL) << "unimplemented method!";
}

mcl::Transform Primitive::totTransform() const
{
	if (!father)
		return localTransform();
	else
		return father->localTransform() * localTransform();
}

void Primitive::initAll()
{
	initializeGL();
}

void Primitive::paint(PaintInfomation* info, PaintVisitor* visitor)
{
	LOG(FATAL) << "unimplemented method!";
}

void Primitive::initialize(PaintVisitor* visitor)
{
	LOG(FATAL) << "unimplemented method!";
}

std::unique_ptr<mcl::Primitive> Primitive::clone()
{
	LOG(FATAL) << "unimplemented method!";
	return nullptr;
}

unsigned int Primitive::globalId = 0;

void GeometryPrimitive::copyAttribute(std::shared_ptr<Primitive> other)
{
	Primitive::copyAttribute(other);
	auto geo = std::dynamic_pointer_cast<GeometryPrimitive>(other);
	if(geo)
		setColor(geo->color());
	geo->setMaterialData(this->mtdata->clone());
}

void GeometryPrimitive::setSelected(bool selected)
{
	if (selected == this->selected()) return;
	if (selected) {
		tempColor_ = color();
		setColor(Vector3f{ 1,0,0 }); //Ñ¡ÖÐÉ«
	}
	else {
		setColor(tempColor_);
	}
	Primitive::setSelected(selected);
}

void GeometryPrimitive::setMaterialData(const std::shared_ptr<DataNode> data)
{
	mtdata = data;
	initMaterial();
}

std::shared_ptr<mcl::Material> GeometryPrimitive::getMaterial()
{
	if (!mat) {
		mat = getDefaultMaterial();
	}
	return mat;
}

std::shared_ptr<mcl::Material> GeometryPrimitive::getDefaultMaterial()
{
	auto matnode = getMaterialNode();
	matnode->clear();
	Material::inputToNode("Lambertain 0.7,0.7,0.7 \"\"", matnode.get());
	return Material::createMaterial(matnode.get());
}

std::shared_ptr<mcl::RTPrimitive> GeometryPrimitive::createRTPrimitive()
{
	std::shared_ptr<Material> mat;
	std::shared_ptr<RTSurfaceLight> light;
	createMaterialAndLight(mat, light);
	std::shared_ptr<Geometry> geo;
	if (light) {
		geo = light->getSharedGeometry();
	}
	else {
		geo = createGeometry();
	}
	if (!geo || !mat) {
		LOG(FATAL) << "Create ray tracing information failed!";
		return nullptr;
	}
	std::shared_ptr<RTPrimitive> ret = std::make_shared<RTGeometryPrimitive>(geo, mat, light);
	return ret;
}

std::shared_ptr<DataNode> GeometryPrimitive::getMaterialNode()
{
	if (!mtdata)
		mtdata = std::make_shared<DataNode>();
	return mtdata->fd("Mat", "");
}

std::shared_ptr<mcl::Geometry> GeometryPrimitive::createGeometry()
{
	return nullptr;
}

void GeometryPrimitive::createMaterialAndLight(std::shared_ptr<Material>& mat, std::shared_ptr<RTSurfaceLight>& light)
{
	if (!mtdata && !this->mat) {
		mat = std::make_shared<LambertainMaterial>(color());
		light = nullptr;
		return;
	}

	auto le = mtdata->fd("Mat", "")->fd("Le");
	if (le && le->toVector3f().length() > FloatZero) {
		light = std::make_shared<RTSurfaceLight>(le->toVector3f(), createGeometry(),false);
		light->setBackground(Color3f(0, 0, 0));
	}
	else
	{
		light = nullptr;
	}
	mat = this->mat;
}

void GeometryPrimitive::initMaterial()
{
	mat = Material::createMaterial(mtdata->fd("Mat").get());
}

void GeometryPrimitive::initAll()
{
	initializeGL();
	mat->initGL();
}
}
