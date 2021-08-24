#pragma once
#include "mcl.h"
#include "types.h"
#include "database.h"
namespace mcl {
class PaintInfomation;

#define RTTI_CLASS(T)\
	public:\
		typedef char* TypeTag;\
		static TypeTag tag(){ \
			static TypeTag _tag = new char;\
			return _tag;\
		}\
		virtual TypeTag dynamic_tag(){\
			return T::tag();\
		}

class Primitive : public OPENGLCLASS
{
	RTTI_CLASS(Primitive)
public:
	inline Primitive();
	inline Primitive(const Transform& localTransform);
	virtual ~Primitive();

	/* 属性接口 */
	unsigned int id() const { return id_; }
	bool operator<(const Primitive& rhs) {
		return this->id_ < rhs.id_;
	}
	mcl::Bound3f getBound() { return mcl::Bound3f(); } //#TODO1
	
	virtual void copyAttribute(std::shared_ptr<Primitive> other);

	mcl::Transform localTransform() { return localTrans_; }
	void setLocalTransform(mcl::Transform localtrans) { localTrans_ = localtrans; }
	void appendLocalTransform(mcl::Transform trans) { localTrans_ = localTrans_ * trans; }

	virtual Color3f color() = 0;

	/* OpenGL绘制接口 */
	virtual void initialize() = 0;
	virtual void paint(PaintInfomation* info) = 0;

	/* 界面信息接口 */
	virtual QString name() { return ""; }; //#TODO Primitive名称功能
	bool selected() { return selected_; }
	virtual void setSelected(bool selected) { selected_ = selected; }

	/* 深复制接口 */
	virtual std::unique_ptr<Primitive> clone();

	/* 光线追踪接口 */
	virtual std::shared_ptr<RTPrimitive> createRTPrimitive() = 0;
	virtual std::shared_ptr<DataNode> getMaterialNode() {
		return nullptr;
	}

private:
	unsigned int id_;
	static unsigned int globalId;
	bool selected_ = false;

	Transform localTrans_;
};

inline Primitive::Primitive()
	:id_(globalId++)
{
}	

inline Primitive::Primitive(const Transform& localTransform)
	: id_(globalId++), localTrans_(localTransform)
{
}

inline Primitive::~Primitive()
{
}

class GeometryPrimitive : public Primitive
{
	RTTI_CLASS(GeometryPrimitive)
public:
	void copyAttribute(std::shared_ptr<Primitive> other) override;

	void setColor(Color3f color) { color_ = color; }
	Color3f color() override { return color_; }
	void setSelected(bool selected) override;
	void setRayTraceData(const std::shared_ptr<DataNode> data) { rtdata = data; }

	virtual void doBeforePaint(PaintInfomation* info);
	virtual void doAfterPaint(PaintInfomation* info);

	virtual std::shared_ptr<RTPrimitive> createRTPrimitive() override;
	virtual std::shared_ptr<DataNode> getMaterialNode() override;
	virtual std::shared_ptr<Geometry> createGeometry();
	virtual void createMaterialAndLight(std::shared_ptr<Material>& mat, std::shared_ptr<RTSurfaceLight>& light);

protected:
	QMatrix4x4 tempTrans_;

private:
	Color3f color_;
	Color3f tempColor_;

	//#TODO2 克隆时需要克隆该信息
	std::shared_ptr<DataNode> rtdata; //生成光追元素所需信息
};
}
