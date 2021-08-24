#include "Primitive.h"
#include "PaintInformation.h"
#include "RTLight.h"
#include "RTPrimitive.h"
#include "Material.h"
#include "materials/Lambertain.h"
namespace mcl {

void Primitive::copyAttribute(std::shared_ptr<Primitive> other)
{
	setLocalTransform(other->localTransform());
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

void GeometryPrimitive::doBeforePaint(PaintInfomation* info)
{
	if (selected()) {
		this->glEnable(GL_POLYGON_OFFSET_FILL);
		this->glPolygonOffset(-1, -1);
	}
	tempTrans_ = info->viewMat;
	info->viewMat = info->viewMat * localTransform().toQMatrix();
}

void GeometryPrimitive::doAfterPaint(PaintInfomation* info)
{
	if (selected()) {
		this->glDisable(GL_POLYGON_OFFSET_FILL);
		this->glPolygonOffset(0, 0);
	}
	info->viewMat = tempTrans_;
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
	if (!rtdata)
		rtdata = std::make_shared<DataNode>();
	return rtdata->fd("Mat", "");
}

std::shared_ptr<mcl::Geometry> GeometryPrimitive::createGeometry()
{
	return nullptr;
}

void GeometryPrimitive::createMaterialAndLight(std::shared_ptr<Material>& mat, std::shared_ptr<RTSurfaceLight>& light)
{
	if (!rtdata) {
		mat = std::make_shared<LambertainMaterial>(color());
		light = nullptr;
		return;
	}

	auto le = rtdata->fd("Le");
	if (le && le->toVector3f().length() > FloatZero) {
		light = std::make_shared<RTSurfaceLight>(le->toVector3f(), createGeometry(),false);
		light->setBackground(Color3f(0, 0, 0));
	}
	else
	{
		light = nullptr;
	}
	mat = Material::createMaterial(rtdata->fd("Mat").get());
}

}
