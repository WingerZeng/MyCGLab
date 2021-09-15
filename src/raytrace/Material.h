#pragma once
#include "mcl.h"
#include "types.h"
#include "Sampler.h"
#include "database.h"
#include "Texture.h"
#include <QString>
#include <vector>
#include <utility>
#include <array>
#include <set>
#include <QOpenGLShaderProgram>
namespace mcl {
	class  LambertainMaterial;
	class  BlinnPhongMaterial;
	class  MirrorMaterial;
	class  DielectricMaterial;
	class  DisneyMaterial;
	class Material
	{
	public:
		virtual std::unique_ptr<BsdfGroup> getBsdfs(HitRecord* rec, Sampler& sampler) const = 0;
		virtual SamplerRequestInfo getSamplerRequest() const { return SamplerRequestInfo(); }
		virtual ~Material() {};

		void setBumpMap(std::shared_ptr<Texture<Color3f>> abumpmap) { bumpmap = abumpmap; }
		void setEmission(Color3f e) { emiss = e; }
		Color3f getEmission() { return emiss; }
		bool hasEmission() const ;
		Color3f emission() const;
		void bumpNormal(HitRecord* rec) const;

		virtual void initGL(); //��GL��ʼ���׶ν���Ԥ����
		virtual void prepareGL(QOpenGLShaderProgram* shader); //�ڻ���ǰ׼��OpenGL����

		enum ParameterType
		{
			P_NULL		= 0,
			P_Float		= 1,
			P_Color		= 2,
			P_Map		= 4,
			P_FloatTex	= 5,
			P_ColorTex	= 6,
		};

		// ����²��ʺ���Ҫ�������²���

		enum MeterialType
		{
			Lambertain = 0,
			BlinnPhong,
			Mirror,
			Dielectric,
			Disney
		};

		static const int MeterialTypeBegin = 0;
		static const int MeterialTypeSize = 5;

		static const std::array<QString, MeterialTypeSize> matTypeToName;


		static const std::map<QString, MeterialType> matNameToType;


		typedef std::vector<std::pair<QString, ParameterType>> parvec_t;
		typedef std::pair<QString, ParameterType> parpair_t;

		static const std::array<std::vector<std::pair<QString, ParameterType>>, MeterialTypeSize> matparvec;

		static std::shared_ptr<Material> createMaterial(DataNode* node);

		// ����²��ʺ���Ҫ�������ϲ���

		static MeterialType getTypeByParameters(std::set<QString> parnames);

		static std::shared_ptr<Texture<Color3f>> getColorTexture(QString name, DataNode* node);
		static ParameterType getColorTexture(QString name, DataNode* node, QVector4D& color, QString& texture);
		static std::shared_ptr<Texture<Float>> getFloatTexture(QString name, DataNode* node);
		static ParameterType getFloatTexture(QString name, DataNode* node, Float& value, QString& texture);

		static int inputToNode(ReadRemainString input, DataNode* node);

	protected:
		MeterialType _type;
	private:
		std::shared_ptr<Texture<Color3f>> bumpmap;
		Color3f emiss; //����ʵʱ��Ⱦ
	};
}

