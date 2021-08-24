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
//������ɫ��ģ����궨��
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

	//������ɫ��ģ��
	template <typename ...Shaders>
	class Shader
	{
	public:
		static QOpenGLShaderProgram* ptr();
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

//---------------------------------
//
//������ɫ������
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
#undef DEF_SHADER_SOURCE
//---------------------------------
//
//������ɫ������
//
//---------------------------------

	using CommonShader = Shader<VertexShader_Mesh, FragShader_Mesh>;
	using LightShader = Shader<VertexShader_LightedMesh_SingleNormal, FragShader_LightedMesh>;
	using LineShader = Shader<VertexShader_Mesh, GeoShader_Line, FragShader_Mesh>;
	using PointShader = Shader<VertexShader_Mesh, GeoShader_Point, FragShader_Mesh>;
	using LightPerFragShader = Shader<VertexShader_OutputPos, GeoShader_LightedMesh_CalNormal, FragShader_LightPerFrag>;
}
