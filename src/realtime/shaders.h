#pragma once
#include "mcl.h"
#include "types.h"
#include <fstream>
#include <sstream>
#include <QFile>
#include "QOpenGLShaderProgram"
#include "ui/ProjectMagager.h"
namespace mcl {
	inline bool compileVrtShader(QOpenGLShaderProgram& shaderProgram, const char* vs, const char* fs) {
		bool success = shaderProgram.addShaderFromSourceCode(QOpenGLShader::Vertex, vs);
		if (!success) {
			qDebug() << "Mesh: shaderProgram addShaderFromSourceFile failed!" << shaderProgram.log();
			return false;
		}
		success = shaderProgram.addShaderFromSourceCode(QOpenGLShader::Fragment, fs);
		if (!success) {
			qDebug() << "Mesh: shaderProgram addShaderFromSourceFile failed!" << shaderProgram.log();
			return false;
		}
		success = shaderProgram.link();
		if (!success) {
			qDebug() << "Mesh: shaderProgram link failed!" << shaderProgram.log();
			return false;
		}
		return true;
	}

	inline bool compileVrtShader(QOpenGLShaderProgram& shaderProgram, const char* vs, const char* gs, const char* fs) {
		bool success = shaderProgram.addShaderFromSourceCode(QOpenGLShader::Vertex, vs);
		if (!success) {
			qDebug() << "Mesh: shaderProgram addShaderFromSourceFile failed!" << shaderProgram.log();
			return false;
		}
		success = shaderProgram.addShaderFromSourceCode(QOpenGLShader::Geometry, gs);
		if (!success) {
			qDebug() << "Mesh: shaderProgram addShaderFromSourceFile failed!" << shaderProgram.log();
			return false;
		}
		success = shaderProgram.addShaderFromSourceCode(QOpenGLShader::Fragment, fs);
		if (!success) {
			qDebug() << "Mesh: shaderProgram addShaderFromSourceFile failed!" << shaderProgram.log();
			return false;
		}
		success = shaderProgram.link();
		if (!success) {
			qDebug() << "Mesh: shaderProgram link failed!" << shaderProgram.log();
			return false;
		}
		return true;
	}

//---------------------------------
//
//单体着色器模板与宏定义
//
//---------------------------------
#define DEF_SHADER_SOURCE(Name)\
	class Name {\
	public:\
		static std::string text(){\
			QFile file(PROPTR->swPath()+"/shaders/"+#Name);\
			file.open(QIODevice::ReadOnly | QIODevice::Text);\
			auto bt = QString(file.readAll());\
			return bt.toStdString();\
		}\
	};

	//单体着色器模板
	template <typename ...Shaders>
	class Shader
	{
	public:
		static QOpenGLShaderProgram* ptr();
		static void clear();
	private:
		static QOpenGLShaderProgram* shader;
	};

	template <typename ...Shaders>
	QOpenGLShaderProgram* mcl::Shader<Shaders...>::shader = nullptr;

	template <typename ...Shaders>
	QOpenGLShaderProgram* mcl::Shader<Shaders...>::ptr()
	{
		if (!shader) {
			shader = new QOpenGLShaderProgram;
			if (!compileVrtShader(*shader, (Shaders::text().c_str())...)) {
				shader = nullptr;
				delete shader;
			}
		}
		return shader;
	}

	template <typename ...Shaders>
	void mcl::Shader<Shaders...>::clear()
	{
		if (shader) {
			delete shader;
		}
		shader = nullptr;
	}
//---------------------------------
//
//定义着色器代码
//
//---------------------------------
	DEF_SHADER_SOURCE(VertexShader_Mesh);

	DEF_SHADER_SOURCE(FragShader_Mesh);

	DEF_SHADER_SOURCE(VertexShader_LightedMesh_SingleNormal);

	DEF_SHADER_SOURCE(FragShader_LightedMesh);

	DEF_SHADER_SOURCE(GeoShader_LightedMesh_CalNormal);

	DEF_SHADER_SOURCE(GeoShader_Line);

	DEF_SHADER_SOURCE(GeoShader_Point);

	DEF_SHADER_SOURCE(VertexShader_OutputPos);

	DEF_SHADER_SOURCE(FragShader_LightPerFrag);

	DEF_SHADER_SOURCE(VertexShader_PostProc);

	DEF_SHADER_SOURCE(FragShader_ToneMap);

	DEF_SHADER_SOURCE(FragShader_DeferredDirectLight);

	DEF_SHADER_SOURCE(FragShader_DeferredSSDO);

	DEF_SHADER_SOURCE(VertexShader_CubeShadowMap);

	DEF_SHADER_SOURCE(GeoShader_CubeShadowMap);

	DEF_SHADER_SOURCE(FragShader_ShadowMap);
#undef DEF_SHADER_SOURCE
//---------------------------------
//
//定义着色器程序
//
//---------------------------------

	using CommonShader = Shader<VertexShader_Mesh, FragShader_Mesh>;
	using LightShader = Shader<VertexShader_LightedMesh_SingleNormal, FragShader_LightedMesh>;
	using LineShader = Shader<VertexShader_Mesh, GeoShader_Line, FragShader_Mesh>;
	using PointShader = Shader<VertexShader_Mesh, GeoShader_Point, FragShader_Mesh>;
	using LightPerFragShader = Shader<VertexShader_OutputPos, GeoShader_LightedMesh_CalNormal, FragShader_LightPerFrag>;
	using ToneMapShader = Shader<VertexShader_PostProc, FragShader_ToneMap>;
	using CubeShaodwMapShader = Shader<VertexShader_CubeShadowMap, GeoShader_CubeShadowMap, FragShader_ShadowMap>;
	using DeferredDirectLightShader = Shader<VertexShader_PostProc, FragShader_DeferredDirectLight>;
	using DeferredSsdoShader = Shader<VertexShader_PostProc, FragShader_DeferredSSDO>;
	int initAllShaders();
}

