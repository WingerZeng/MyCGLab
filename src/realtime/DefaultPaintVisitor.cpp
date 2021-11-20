#include "DefaultPaintVisitor.h"
#include "PTriMesh.h"
#include "PPolygonMesh.h"
#include "PPoint.h"
#include "PPolygon.h"
#include "PLines.h"
#include "PaintInformation.h"
#include "shaders.h"
#include "Material.h"
#include "GLFunctions.h"
#include "utilities.h"
namespace mcl {
	void setDefferPrepareUniform(PaintInfomation* info, QOpenGLShaderProgram* shader)
	{
		shader->setUniformValue("modelMatInv", info->modelMat.inverted());
		shader->setUniformValue("modelMat", info->modelMat);
		shader->setUniformValue("modelViewMatInv", (info->viewMat * info->modelMat).inverted());
		shader->setUniformValue("viewMat", info->viewMat);
		shader->setUniformValue("projMat", info->projMat);
		shader->setUniformValue("selected", GLint(info->selected));
		shader->setUniformValue("selectedColor", QVector3D(info->selectedColor));
		shader->setUniformValue("hasNormal", info->hasNormal);
	}

	//#TODO1 info可以优化为栈模式
	int DefaultPaintVisitor::paintTris(PaintInfomation* info, PTriMesh* tri)
	{
		//#TODO0 线框的渲染可以放在延迟渲染之后的正向渲染中
		//do before paint
		if (tri->selected()) {
			GLFUNC->glEnable(GL_POLYGON_OFFSET_FILL);
			GLFUNC->glPolygonOffset(-1, -1);
		}
		QMatrix4x4 tempTrans_ = info->modelMat;
		bool tempSelected_ = info->selected;
		info->modelMat = info->modelMat * tri->localTransform().toQMatrix();
		info->selected = tri->selected() || info->selected;

		//paint
		if (info->fillmode == FILL || info->fillmode == FILL_WIREFRAME) {
			QOpenGLShaderProgram* shader;
			shader = LightPerFragShader::ptr();
			shader->bind();

			info->hasNormal = tri->hasNormal();

			setDefferPrepareUniform(info, shader);
			tri->getMaterial()->prepareGL(shader);

			tri->getVAO()->bind();
			GLFUNC->glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			GLFUNC->glDrawArrays(GL_TRIANGLES, 0, tri->getTriNumer() * 3);

			GLFUNC->glDisable(GL_POLYGON_OFFSET_FILL);
			GLFUNC->glPolygonOffset(0, 0);

			info->hasNormal = false;
			shader->release();
		}
		if (info->fillmode == WIREFRAME || info->fillmode == FILL_WIREFRAME) {
			LineShader::ptr()->bind();
			setLineUniform(info);
			LineShader::ptr()->setUniformValue("ourColor", .0, .0, .0, 1.0f);
			GLFUNC->glEnable(GL_POLYGON_OFFSET_FILL);
			GLFUNC->glPolygonOffset(-1, -1);

			tri->getLineVAO()->bind();

			GLFUNC->glDrawArrays(GL_LINES, 0, tri->getEdgeNumber() * 2);

			GLFUNC->glDisable(GL_POLYGON_OFFSET_FILL);
			GLFUNC->glPolygonOffset(0, 0);
			LineShader::ptr()->release();
		}

		//do after paint
		GLFUNC->glDisable(GL_POLYGON_OFFSET_FILL);
		GLFUNC->glPolygonOffset(0, 0);
		info->modelMat = tempTrans_;
		info->selected = tempSelected_;

		return 0;
	}

	int DefaultPaintVisitor::paintPoint(PaintInfomation* info, PPoint* point)
	{
		//do before paint
		QMatrix4x4 tempTrans_ = info->modelMat;
		bool tempSelected_ = info->selected;
		info->modelMat = info->modelMat * point->localTransform().toQMatrix();
		info->selected = point->selected() || info->selected;

		GLFUNC->glEnable(GL_POLYGON_OFFSET_FILL);
		GLFUNC->glPolygonOffset(-1, -1);

		point->getVAO().bind();
		PointShader::ptr()->bind();
		setPointUniform(info);
		PointShader::ptr()->setUniformValue("ourColor", QVector4D(point->color().x(), point->color().y(), point->color().z(), 1.0f));

		GLFUNC->glDrawArrays(GL_POINTS, 0, 1);

		GLFUNC->glDisable(GL_POLYGON_OFFSET_FILL);
		GLFUNC->glPolygonOffset(0, 0);

		PointShader::ptr()->release();

		//do after paint
		GLFUNC->glDisable(GL_POLYGON_OFFSET_FILL);
		GLFUNC->glPolygonOffset(0, 0);
		info->modelMat = tempTrans_;
		info->selected = tempSelected_;
		return 0;
	}

	int DefaultPaintVisitor::paintPolygons(PaintInfomation* info, PPolygonMesh* polygon)
	{
		//do before paint
		if (polygon->selected()) {
			GLFUNC->glEnable(GL_POLYGON_OFFSET_FILL);
			GLFUNC->glPolygonOffset(-1, -1);
		}
		QMatrix4x4 tempTrans_ = info->modelMat;
		bool tempSelected_ = info->selected;
		info->modelMat = info->modelMat * polygon->localTransform().toQMatrix();
		info->selected = polygon->selected() || info->selected;

		//paint
		if (info->fillmode == FILL || info->fillmode == FILL_WIREFRAME) {
			QOpenGLShaderProgram* shader;
			if (info->lights.size() && !polygon->selected())
			{
				shader = LightPerFragShader::ptr();
				shader->bind();
				setDefferPrepareUniform(info, shader);
				shader->setUniformValue("material.mtype", -1);
				shader->setUniformValue("material.ourColor", polygon->color().x(), polygon->color().y(), polygon->color().z(), 1.0f);
			}
			else {
				shader = CommonShader::ptr();
				shader->bind();
				setDefferPrepareUniform(info, shader);
				shader->setUniformValue("ourColor", polygon->color().x(), polygon->color().y(), polygon->color().z(), 1.0f);
			}

			polygon->getVAO()->bind();
			GLFUNC->glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

			for (int i = 0; i < polygon->getDrawInfo().size(); i++)
				GLFUNC->glDrawArrays(polygon->getDrawInfo()[i].type, polygon->getDrawInfo()[i].offset, polygon->getDrawInfo()[i].size);

			GLFUNC->glDisable(GL_POLYGON_OFFSET_FILL);
			GLFUNC->glPolygonOffset(0, 0);

			shader->release();
		}
		if (info->fillmode == WIREFRAME || info->fillmode == FILL_WIREFRAME) {  //选中时隐藏
			LineShader::ptr()->bind();
			setLineUniform(info);
			LineShader::ptr()->setUniformValue("material.ourColor", .0, .0, .0, 1.0f);
			GLFUNC->glEnable(GL_POLYGON_OFFSET_FILL);
			GLFUNC->glPolygonOffset(-1, -1);

			polygon->getVAO()->bind();

			for (int i = 0; i < polygon->getBoundDrawInfo().size(); i++)
				GLFUNC->glDrawArrays(polygon->getBoundDrawInfo()[i].type, polygon->getBoundDrawInfo()[i].offset, polygon->getBoundDrawInfo()[i].size);

			GLFUNC->glDisable(GL_POLYGON_OFFSET_FILL);
			GLFUNC->glPolygonOffset(0, 0);
			LineShader::ptr()->release();
		}

		//do after paint
		GLFUNC->glDisable(GL_POLYGON_OFFSET_FILL);
		GLFUNC->glPolygonOffset(0, 0);
		info->modelMat = tempTrans_;
		info->selected = tempSelected_;
		return 0;
	}

	int DefaultPaintVisitor::paintLines(PaintInfomation* info, PLines* lines)
	{
		//do before paint
		GLFUNC->glPolygonOffset(-2, -2);
		GLFUNC->glEnable(GL_POLYGON_OFFSET_FILL);

		QMatrix4x4 tempTrans_ = info->modelMat;
		bool tempSelected_ = info->selected;
		info->modelMat = info->modelMat * lines->localTransform().toQMatrix();
		info->selected = info->selected || lines->selected();

		//do paint
		lines->getVAO().bind();
		LineShader::ptr()->bind();
		setLineUniform(info);
		LineShader::ptr()->setUniformValue("ourColor", QVector4D(lines->color().x(), lines->color().y(), lines->color().z(), 1.0f));
		if (lines->isLoop())
			GLFUNC->glDrawArrays(GL_LINE_LOOP, 0, lines->getPtNum());
		else
			GLFUNC->glDrawArrays(GL_LINE_STRIP, 0, lines->getPtNum());

		LineShader::ptr()->release();

		//do after paint
		GLFUNC->glDisable(GL_POLYGON_OFFSET_FILL);
		GLFUNC->glPolygonOffset(0, 0);
		info->modelMat = tempTrans_;
		info->selected = tempSelected_;
		return 0;
	}

	void DefaultPaintVisitor::setPointUniform(PaintInfomation* info)
	{
		PointShader::ptr()->setUniformValue("u_viewportSize", info->width, info->height);
		PointShader::ptr()->setUniformValue("pointSize", GLfloat(info->pointSize));
	}

	void DefaultPaintVisitor::setLineUniform(PaintInfomation* info)
	{
		LineShader::ptr()->setUniformValue("u_viewportSize", info->width, info->height);
		LineShader::ptr()->setUniformValue("u_thickness", GLfloat(info->lineWidth));
	}

	int DeferredMtrPaintVisitor::paintTris(PaintInfomation* info, PTriMesh* tri)
	{
		//#TODO0 线框的渲染可以放在延迟渲染之后的正向渲染中
		//do before paint
		if (tri->selected()) {
			GLFUNC->glEnable(GL_POLYGON_OFFSET_FILL);
			GLFUNC->glPolygonOffset(-1, -1);
		}
		QMatrix4x4 tempTrans_ = info->modelMat;
		bool tempSelected_ = info->selected;
		info->modelMat = info->modelMat * tri->localTransform().toQMatrix();
		info->selected = tri->selected() || info->selected;

		//paint
		if (info->fillmode == FILL || info->fillmode == FILL_WIREFRAME) {
			QOpenGLShaderProgram* shader;
			shader = LightPerFragShader::ptr();
			shader->bind();

			info->hasNormal = tri->hasNormal();

			setDefferPrepareUniform(info, shader);
			shader->setUniformValue("primid", QVector3D(intToRgb8(tri->id())));
			tri->getMaterial()->prepareGL(shader);

			tri->getVAO()->bind();
			GLFUNC->glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			GLFUNC->glDrawArrays(GL_TRIANGLES, 0, tri->getTriNumer() * 3);

			GLFUNC->glDisable(GL_POLYGON_OFFSET_FILL);
			GLFUNC->glPolygonOffset(0, 0);

			info->hasNormal = false;
			shader->release();
		}

		//do after paint
		GLFUNC->glDisable(GL_POLYGON_OFFSET_FILL);
		GLFUNC->glPolygonOffset(0, 0);
		info->modelMat = tempTrans_;
		info->selected = tempSelected_;

		return 0;
	}

}