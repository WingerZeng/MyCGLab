#include "PaintInformation.h"
#include <QOpenGLShaderProgram>
#include "light.h"
void mcl::PaintInfomation::setUniformValue(QOpenGLShaderProgram* shader)
{
	shader->setUniformValue("modelMatInv", modelMat.inverted());
	shader->setUniformValue("modelMat", modelMat);
	shader->setUniformValue("modelViewMatInv", (viewMat * modelMat).inverted());
	shader->setUniformValue("viewMat", viewMat);
	shader->setUniformValue("projMat", projMat);
	shader->setUniformValue("lightCount", GLint(lights.size()));
	shader->setUniformValue("u_viewportSize", width, height);
	shader->setUniformValue("u_thickness", GLfloat(lineWidth));
	shader->setUniformValue("pointSize", GLfloat(pointSize));
	shader->setUniformValue("selected", GLint(selected));
	shader->setUniformValue("selectedColor", QVector3D(selectedColor));

	for (int j = 0; j < lights.size(); j++) {
		lights[j]->setUniformValue(shader, j);
	}
}
