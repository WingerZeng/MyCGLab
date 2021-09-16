#ifndef AXIS_H
#define AXIS_H

#include "mcl.h"
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions_4_3_Core>
#include <QDebug>
#include <QMatrix4x4>
#include <math.h>

class Axis: public OPENGLCLASS
{
public:
    Axis();
    ~Axis();

    void initialize();
    void paint();
    void updateViewMat(QMatrix4x4 mat);
    void updateProjMat(QMatrix4x4 mat);

private:
    QOpenGLShaderProgram axisShaderProgram;
    QOpenGLShaderProgram textShaderProgram;
    QOpenGLBuffer axisVBO;
    QOpenGLVertexArrayObject axisVAO;
    QOpenGLBuffer textVBO;
    QOpenGLVertexArrayObject textVAO;

    QMatrix4x4 modelMatX;
    QMatrix4x4 modelMatY;
    QMatrix4x4 modelMatZ;
    QMatrix4x4 viewMat;
    QMatrix4x4 projMat;
};

#endif // CUBE_H

