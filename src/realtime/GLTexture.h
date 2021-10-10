#pragma once
#include "mcl.h"
#include "types.h"

class QOpenGLShaderProgram;

namespace mcl {
	class GLAbstractTexture
	{
	public:
		GLAbstractTexture();
		virtual ~GLAbstractTexture();

		GLuint id() { return tex; }

		virtual void bindToUniform(std::string name, QOpenGLShaderProgram* shader) = 0;

		virtual void bind() = 0;

		virtual void release() = 0;

		virtual void resize(int w, int h);

		virtual void bindToFbo(GLuint fbo, GLuint attachment) = 0;

		Vector2f size() { return Vector2f(w, h); }
	protected:
		GLuint tex;
		GLuint activedTexId;
	private:
		static GLuint ActivedTextureCnt;
		static std::vector<GLuint> DestroyedActiveId;
		int w, h;
	};

	class GLTexture2D: public GLAbstractTexture 
	{
	public:
		GLTexture2D(GLuint internalType, GLuint baseType, GLuint unitType);

		virtual void bindToUniform(std::string name, QOpenGLShaderProgram* shader) override;

		virtual void bind() override;

		virtual void release() override;

		virtual void resize(int w, int h) override;

		virtual void bindToFbo(GLuint fbo, GLuint attachment) override;

		void setFilter(GLuint minFilter, GLuint maxFilter);

		void setWrap(GLuint s, GLuint t);
	private:
		GLuint internalType;
		GLuint baseType;
		GLuint unitType;
	};

	class GLTextureMultiSample: public GLAbstractTexture
	{
	public:
		GLTextureMultiSample(GLuint internalType, int nsample);

		virtual void bindToUniform(std::string name, QOpenGLShaderProgram* shader) override;

		
		virtual void bind() override;


		virtual void release() override;


		virtual void resize(int w, int h) override;


		virtual void bindToFbo(GLuint fbo, GLuint attachment) override;

	private:
		GLuint internalType;
		int nsample;
	};

	class GLTextureCubeMap : public GLAbstractTexture
	{
	public:
		GLTextureCubeMap(GLuint internalType, GLuint baseType, GLuint unitType);

		virtual void bindToUniform(std::string name, QOpenGLShaderProgram* shader) override;


		virtual void bind() override;


		virtual void release() override;


		virtual void resize(int w, int h) override;


		virtual void bindToFbo(GLuint fbo, GLuint attachment) override;


		void setFilter(GLuint minFilter, GLuint maxFilter);

		void setWrap(GLuint s, GLuint t, GLuint r);
	private:
		GLuint internalType;
		GLuint baseType;
		GLuint unitType;
	};
}

