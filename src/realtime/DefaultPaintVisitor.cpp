#include "DefaultPaintVisitor.h"
#include "PTriMesh.h"
#include "PPolygonMesh.h"
#include "PPoint.h"
#include "PPolygon.h"
#include "PLines.h"
#include "PaintInformation.h"
#include "shaders.h"
#include "Material.h"
namespace mcl{
	
	int DefaultPaintVisitor::paintTris(PaintInfomation* info, PTriMesh* tri)
	{
		//do before paint
		if (tri->selected()) {
			glEnable(GL_POLYGON_OFFSET_FILL);
			glPolygonOffset(-1, -1);
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
			info->setUniformValue(shader);
			tri->getMaterial()->prepareGL(shader);

			tri->getVAO()->bind();
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glDrawArrays(GL_TRIANGLES, 0, tri->getTriNumer() * 3);

			glDisable(GL_POLYGON_OFFSET_FILL);
			glPolygonOffset(0, 0);

			shader->release();
		}
		if (info->fillmode == WIREFRAME || info->fillmode == FILL_WIREFRAME) {
			LineShader::ptr()->bind();
			info->setUniformValue(LineShader::ptr());
			LineShader::ptr()->setUniformValue("ourColor", .0, .0, .0, 1.0f);
			glEnable(GL_POLYGON_OFFSET_FILL);
			glPolygonOffset(-1, -1);

			tri->getVAO()->bind();

			glDrawArrays(GL_LINES, 0, tri->getEdgeNumber() * 2);

			glDisable(GL_POLYGON_OFFSET_FILL);
			glPolygonOffset(0, 0);
			LineShader::ptr()->release();
		}

		//do after paint
		glDisable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(0, 0);
		info->modelMat = tempTrans_;
		info->selected = tempSelected_;

		return 0;
	}

	int DefaultPaintVisitor::initTris(PTriMesh* tri)
	{
		tri->initialize(CommonShader::ptr());
		tri->initialize(LightPerFragShader::ptr());
		tri->initializeLine(LineShader::ptr());
		return 0;
	}

	int DefaultPaintVisitor::paintPoint(PaintInfomation* info, PPoint* point)
	{
		//do before paint
		QMatrix4x4 tempTrans_ = info->modelMat;
		bool tempSelected_ = info->selected;
		info->modelMat = info->modelMat * point->localTransform().toQMatrix();
		info->selected = point->selected() || info->selected;

		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(-1, -1);

		point->getVAO().bind();
		PointShader::ptr()->bind();
		info->setUniformValue(PointShader::ptr());
		PointShader::ptr()->setUniformValue("ourColor", QVector4D(point->color().x(), point->color().y(), point->color().z(), 1.0f));

		glDrawArrays(GL_POINTS, 0, 1);

		glDisable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(0, 0);

		PointShader::ptr()->release();

		//do after paint
		glDisable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(0, 0);
		info->modelMat = tempTrans_;
		info->selected = tempSelected_;
		return 0;
	}

	int DefaultPaintVisitor::initPoint(PPoint* point)
	{
		PointShader::ptr()->setAttributeBuffer(0, GL_FLOAT, 0, 3, 0);
		PointShader::ptr()->enableAttributeArray(0);
		return 0;
	}

	int DefaultPaintVisitor::paintPolygons(PaintInfomation* info, PPolygonMesh* polygon)
	{
		//do before paint
		if (polygon->selected()) {
			glEnable(GL_POLYGON_OFFSET_FILL);
			glPolygonOffset(-1, -1);
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
				info->setUniformValue(shader);
				shader->setUniformValue("ourColor", polygon->color().x(), polygon->color().y(), polygon->color().z(), 1.0f);
			}
			else {
				shader = CommonShader::ptr();
				shader->bind();
				info->setUniformValue(shader);
				shader->setUniformValue("ourColor", polygon->color().x(), polygon->color().y(), polygon->color().z(), 1.0f);
			}

			polygon->getVAO()->bind();
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

			for (int i = 0; i < polygon->getDrawInfo().size(); i++)
				glDrawArrays(polygon->getDrawInfo()[i].type, polygon->getDrawInfo()[i].offset, polygon->getDrawInfo()[i].size);

			glDisable(GL_POLYGON_OFFSET_FILL);
			glPolygonOffset(0, 0);

			shader->release();
		}
		if (info->fillmode == WIREFRAME || info->fillmode == FILL_WIREFRAME) {  //Ñ¡ÖÐÊ±Òþ²Ø
			LineShader::ptr()->bind();
			info->setUniformValue(LineShader::ptr());
			LineShader::ptr()->setUniformValue("ourColor", .0, .0, .0, 1.0f);
			glEnable(GL_POLYGON_OFFSET_FILL);
			glPolygonOffset(-1, -1);

			polygon->getVAO()->bind();

			for (int i = 0; i < polygon->getBoundDrawInfo().size(); i++)
				glDrawArrays(polygon->getBoundDrawInfo()[i].type, polygon->getBoundDrawInfo()[i].offset, polygon->getBoundDrawInfo()[i].size);

			glDisable(GL_POLYGON_OFFSET_FILL);
			glPolygonOffset(0, 0);
			LineShader::ptr()->release();
		}

		//do after paint
		glDisable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(0, 0);
		info->modelMat = tempTrans_;
		info->selected = tempSelected_;
		return 0;
	}

	int DefaultPaintVisitor::initPolygons(PPolygonMesh* polygon)
	{
		polygon->getVAO()->bind();
		polygon->getVBO()->bind();
		CommonShader::ptr()->setAttributeBuffer(0, GL_FLOAT, 0, 3, 0);
		CommonShader::ptr()->enableAttributeArray(0);
		LightPerFragShader::ptr()->setAttributeBuffer(0, GL_FLOAT, 0, 3, 0);
		LightPerFragShader::ptr()->enableAttributeArray(0);

		LineShader::ptr()->setAttributeBuffer(0, GL_FLOAT, 0, 3, 0);
		LineShader::ptr()->enableAttributeArray(0);
		return 0;
	}

	int DefaultPaintVisitor::paintLines(PaintInfomation* info, PLines* lines)
	{
		//do before paint
		glPolygonOffset(-2, -2);
		glEnable(GL_POLYGON_OFFSET_FILL);

		QMatrix4x4 tempTrans_ = info->modelMat;
		bool tempSelected_ = info->selected;
		info->modelMat = info->modelMat * lines->localTransform().toQMatrix();
		info->selected = info->selected || lines->selected();

		//do paint
		lines->getVAO().bind();
		LineShader::ptr()->bind();
		info->setUniformValue(LineShader::ptr());
		LineShader::ptr()->setUniformValue("ourColor", QVector4D(lines->color().x(), lines->color().y(), lines->color().z(), 1.0f));
		if (lines->isLoop())
			glDrawArrays(GL_LINE_LOOP, 0, lines->getPtNum());
		else
			glDrawArrays(GL_LINE_STRIP, 0, lines->getPtNum());

		LineShader::ptr()->release();

		//do after paint
		glDisable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(0, 0);
		info->modelMat = tempTrans_;
		info->selected = tempSelected_;
		return 0;
	}

	int DefaultPaintVisitor::initLines(PLines* lines)
	{
		lines->getVAO().bind();
		lines->getVBO().bind();
		LineShader::ptr()->setAttributeBuffer(0, GL_FLOAT, 0, 3, 0);
		LineShader::ptr()->enableAttributeArray(0);
		return 0;
	}

}