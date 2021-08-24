#pragma once
#include "mcl.h"
#include "types.h"
namespace mcl {
	class PaintInfomation;

	class Light
	{
	public:
		enum PositionType
		{
			L_ABSOLUTE = 0,
			L_RELATIVE,
		};
		Light(PositionType posType = L_RELATIVE)
			:posType_(posType) {}

		virtual ~Light() {};
		virtual Color3f getAmbient() = 0;
		virtual Color3f getDiffuse() = 0;
		virtual Color3f getSpecular() = 0;
		/**
		 * @brief ���ع�Դ����������ϵ�µ�����
		 */
		virtual Point3f getPosition(PaintInfomation* info=nullptr) = 0;  
		virtual Point3f getPosition(Transform invViewMatrix) = 0;

		virtual PositionType getPositionType() { return posType_; }
	protected:
		PositionType posType_;
	};

	class ConstantLight:public Light
	{
	public:
		ConstantLight(Color3f Ambient, Color3f Diffuse, Color3f Specular, Point3f pos, PositionType posType = L_RELATIVE);

		virtual Color3f getAmbient() 	{ return ambient_;};
		virtual Color3f getDiffuse()	{ return diffuse_;}	;
		virtual Color3f getSpecular() { return specular_; };
		virtual Point3f getPosition(PaintInfomation* info = nullptr) { return pos_; } //#TODO1 ���ݱ任����͹�Դ�������������ƹ�Դλ��, ���Ҹ�����ɫ��Ϊ����������ϵ�¼������
		virtual Point3f getPosition(Transform invViewMatrix);

	private:
		Color3f ambient_;
		Color3f diffuse_;
		Color3f specular_;
		Point3f pos_;
	};
}
