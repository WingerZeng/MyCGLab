#include "axis.h"
#include "GLFunctions.h"
const char *vertexShaderSourceForAxis =
        "#version 430 core\n"
        "layout(location = 0) in vec3 aPos;\n"
        "uniform mat4 modelMat;\n"
        "uniform mat4 viewMat;\n"
        "uniform mat4 projMat;\n"
        "void main(){\n"
        "  gl_Position = projMat * viewMat * modelMat * vec4(aPos.x, aPos.y, aPos.z, 1.0)+vec4(-1.5,-1.3,0.0,1.0);\n"
        "}\n\0";

const char *fragmentShaderSourceForAxis =
        "#version 430 core\n"
        "uniform vec4 axisColor;\n"
        "out vec4 FragColor;\n"
        "void main(){\n"
        "   FragColor = axisColor;\n"
        "}\n\0";

const char *vertexShaderSourceForText =
        "#version 430 core\n"
        "layout(location = 1) in vec3 aPos;\n"
        "uniform mat4 modelMat;\n"
        "uniform mat4 viewMat;\n"
        "uniform mat4 projMat;\n"
        "void main(){\n"
        "  gl_Position = projMat * viewMat * modelMat * vec4(0.0, 0.57, 0.0, 1.0)+vec4(-1.5,-1.3,0.0,0.0)+ projMat * vec4(aPos.x*1.5, aPos.y, aPos.z, 1.0);\n"
        "}\n\0";

const char *fragmentShaderSourceForText =
        "#version 430 core\n"
        "uniform vec4 textColor;\n"
        "out vec4 FragColor;\n"
        "void main(){\n"
        "   FragColor = textColor;\n"
        "}\n\0";


Axis::Axis(){}

Axis::~Axis()
{
    axisVBO.destroy();
    axisVAO.destroy();
    textVBO.destroy();
    textVAO.destroy();
}

void Axis::initialize()
{
    modelMatX.rotate(90.0f,QVector3D(0,0,-1));
    modelMatZ.rotate(90.0f,QVector3D(1,0,0));

    //Shader
    bool success = axisShaderProgram.addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSourceForAxis);
    if (!success) {
        qDebug() << "Axis: shaderProgram addShaderFromSourceFile failed!" << axisShaderProgram.log();
        return;
    }

    success = axisShaderProgram.addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSourceForAxis);
    if (!success) {
        qDebug() << "Axis: shaderProgram addShaderFromSourceFile failed!" << axisShaderProgram.log();
        return;
    }

    success = axisShaderProgram.link();
    if(!success) {
        qDebug() << "Axis: shaderProgram link failed!" << textShaderProgram.log();
    }


   success = textShaderProgram.addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSourceForText);
   if (!success) {
       qDebug() << "Text: shaderProgram addShaderFromSourceFile failed!" << textShaderProgram.log();
       return;
   }

   success = textShaderProgram.addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSourceForText);
   if (!success) {
       qDebug() << "Text: shaderProgram addShaderFromSourceFile failed!" << textShaderProgram.log();
       return;
   }

   success = textShaderProgram.link();
   if(!success) {
       qDebug() << "Text: shaderProgram link failed!" << textShaderProgram.log();
   }

    //箭头顶点数据
    GLfloat vertices[] = {
         0.0f,   0.5f,  0.0f,
        -0.04f,  0.4f,  0.0f,
        -0.02f,  0.4f,  0.0346f,

         0.0f,   0.5f,  0.0f,
        -0.02f,  0.4f,  0.0346f,
         0.02f,  0.4f,  0.0346f,

         0.0f,   0.5f,  0.0f,
         0.02f,  0.4f,  0.0346f,
         0.04f,  0.4f,  0.0f,

         0.0f,   0.5f,  0.0f,
         0.04f,  0.4f,  0.0f,
         0.02f,  0.4f, -0.0346f,

         0.0f,   0.5f,  0.0f,
         0.02f,  0.4f, -0.0346f,
        -0.02f,  0.4f, -0.0346f,

         0.0f,   0.5f,  0.0f,
        -0.02f,  0.4f, -0.0346f,
        -0.04f,  0.4f,  0.0f,

        -0.02f,  0.4f, -0.0346f,
        -0.04f,  0.4f,  0.0f,
        -0.02f,  0.4f,  0.0346f,

        -0.02f,  0.4f, -0.0346f,
         0.02f,  0.4f, -0.0346f,
        -0.02f,  0.4f,  0.0346f,

         0.02f,  0.4f, -0.0346f,
        -0.02f,  0.4f,  0.0346f,
         0.02f,  0.4f,  0.0346f,

         0.02f,  0.4f, -0.0346f,
         0.02f,  0.4f,  0.0346f,
         0.04f,  0.4f,  0.0f,

        -0.005f, 0.4f,  0.0087f,
        -0.01f,  0.4f,  0.0f,
        -0.01f,  0.0f,  0.0f,

        -0.01f,  0.0f,  0.0f,
        -0.005f, 0.0f,  0.0087f,
        -0.005f, 0.4f,  0.0087f,

         0.005f, 0.4f,  0.0087f,
        -0.005f, 0.4f,  0.0087f,
        -0.005f, 0.0f,  0.0087f,

        -0.005f, 0.0f,  0.0087f,
         0.005f, 0.0f,  0.0087f,
         0.005f, 0.4f,  0.0087f,

         0.01f,  0.4f,  0.0f,
         0.005f, 0.4f,  0.0087,
         0.005f, 0.0f,  0.0087f,

         0.005f, 0.0f,  0.0087f,
         0.01f,  0.0f,  0.0f,
         0.01f,  0.4f,  0.0f,

         0.005f, 0.4f, -0.0087f,
         0.01f,  0.4f,  0.0f,
         0.01f,  0.0f,  0.0f,

         0.01f,  0.0f,  0.0f,
         0.005f, 0.0f, -0.0087f,
         0.005f, 0.4f, -0.0087f,

        -0.005f, 0.4f, -0.0087f,
         0.005f, 0.4f, -0.0087f,
         0.005f, 0.0f, -0.0087f,

         0.005f, 0.0f, -0.0087f,
        -0.005f, 0.0f, -0.0087f,
        -0.005f, 0.4f, -0.0087f,

        -0.01f,  0.4f,  0.0f,
        -0.005f, 0.4f, -0.0087f,
        -0.005f, 0.0f, -0.0087f,

        -0.005f, 0.0f, -0.0087f,
        -0.01f,  0.0f,  0.0f,
        -0.01f,  0.4f,  0.0f

    };

    //X、Y、Z字母顶点数据
    GLfloat text[] ={
        //X
         0.02f,  0.05f,  0.0f,
         0.01f,  0.05f,  0.0f,
        -0.02f, -0.05f,  0.0f,

        -0.02f, -0.05f,  0.0f,
        -0.01f, -0.05f,  0.0f,
         0.02f,  0.05f,  0.0f,

        -0.01f,  0.05f,  0.0f,
        -0.02f,  0.05f,  0.0f,
         0.01f, -0.05f,  0.0f,

         0.01f, -0.05f,  0.0f,
         0.02f, -0.05f,  0.0f,
        -0.02f,  0.05f,  0.0f,
        //Y
        -0.01f,  0.05f,  0.0f,
        -0.02f,  0.05f,  0.0f,
         0.0f,   0.015f, 0.0f,

        -0.02f,  0.05f,  0.0f,
        -0.005f, 0.0025f,0.0f,
         0.0f,   0.015f, 0.0f,

         0.0f,   0.015f, 0.0f,
        -0.005f, 0.0025f,0.0f,
        -0.005f,-0.05f,  0.0f,

         0.0f,   0.015f, 0.0f,
        -0.005f,-0.05f,  0.0f,
         0.005f,-0.05f,  0.0f,

         0.0f,   0.015f, 0.0f,
         0.005f,-0.05f,  0.0f,
         0.005f, 0.0025f,0.0f,

         0.02f,  0.05f,  0.0f,
         0.0f,   0.015f, 0.0f,
         0.005f, 0.0025f,0.0f,

         0.02f,  0.05f,  0.0f,
         0.01f,  0.05f,  0.0f,
         0.0f,   0.015f, 0.0f,
         //Z
         0.01f,  0.035f, 0.0f,
        -0.02f,  0.045f, 0.0f,
        -0.02f,  0.035f, 0.0f,

         0.01f,  0.045f, 0.0f,
        -0.02f,  0.045f, 0.0f,
         0.01f,  0.035f, 0.0f,

         0.02f,  0.045f, 0.0f,
         0.01f,  0.045f, 0.0f,
        -0.02f, -0.045f, 0.0f,

        -0.02f, -0.045f, 0.0f,
        -0.01f, -0.045f, 0.0f,
         0.02f,  0.045f, 0.0f,

         0.02f, -0.035f, 0.0f,
        -0.01f, -0.035f, 0.0f,
        -0.01f, -0.045f, 0.0f,

         0.02f, -0.035f, 0.0f,
        -0.01f, -0.045f, 0.0f,
         0.02f, -0.045f, 0.0f
    };

     QOpenGLVertexArrayObject::Binder axisVAOBind(&axisVAO);

     //箭头初始化
     axisVBO.create();
     axisVBO.bind();
     axisVBO.allocate(vertices, sizeof(vertices));

     // position attribute
     int attr = -1;
     attr = axisShaderProgram.attributeLocation("aPos");
     axisShaderProgram.setAttributeBuffer(attr, GL_FLOAT, 0, 3, sizeof(GLfloat) * 3);
     axisShaderProgram.enableAttributeArray(attr);
     axisShaderProgram.bind();

     axisVBO.release();

     //字母初始化
     textVBO.create();
     textVBO.bind();
     textVBO.allocate(text, sizeof(text));

     attr = -1;
     attr = textShaderProgram.attributeLocation("aPos");
     textShaderProgram.setAttributeBuffer(attr, GL_FLOAT, 0, 3, sizeof(GLfloat) * 3);
     textShaderProgram.enableAttributeArray(attr);
     textShaderProgram.bind();

     textVBO.release();
}

void Axis::paint()
{
	mcl::GLFunctions::getSingletonPtr()->glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	GLFUNC->glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);

    //绘制箭头
    axisShaderProgram.bind();
    QOpenGLVertexArrayObject::Binder axisVAOBind(&axisVAO);

    axisShaderProgram.setUniformValue("viewMat",viewMat);
    axisShaderProgram.setUniformValue("projMat",projMat);

    axisShaderProgram.setUniformValue("axisColor",0.9f,0.0f,0.0f,1.0f);
    axisShaderProgram.setUniformValue("modelMat",modelMatX);
	GLFUNC->glDrawArrays(GL_TRIANGLES, 0, 66);

    axisShaderProgram.setUniformValue("axisColor",0.0f,0.8f,0.0f,1.0f);
    axisShaderProgram.setUniformValue("modelMat",modelMatY);
	GLFUNC->glDrawArrays(GL_TRIANGLES, 0, 66);

    axisShaderProgram.setUniformValue("axisColor",0.0f,0.0f,1.0f,1.0f);
    axisShaderProgram.setUniformValue("modelMat",modelMatZ);
	GLFUNC->glDrawArrays(GL_TRIANGLES, 0, 66);

    axisShaderProgram.release();

    //绘制字母
    textShaderProgram.bind();

    textShaderProgram.setUniformValue("viewMat",viewMat);
    textShaderProgram.setUniformValue("projMat",projMat);

//    textShaderProgram.setUniformValue("textColor",0.9f,0.0f,0.0f,1.0f);
    textShaderProgram.setUniformValue("modelMat",modelMatX);
	GLFUNC->glDrawArrays(GL_TRIANGLES, 0, 12);

//    textShaderProgram.setUniformValue("textColor",0.0f,0.8f,0.0f,1.0f);
    textShaderProgram.setUniformValue("modelMat",modelMatY);
	GLFUNC->glDrawArrays(GL_TRIANGLES, 12, 21);

//    textShaderProgram.setUniformValue("textColor",0.0f,0.0f,1.0f,1.0f);
    textShaderProgram.setUniformValue("modelMat",modelMatZ);
    GLFUNC->glDrawArrays(GL_TRIANGLES, 33, 18);

    textShaderProgram.release();
}

void Axis::updateViewMat(QMatrix4x4 mat){
    viewMat = mat;
}

void Axis::updateProjMat(QMatrix4x4 mat){
    projMat = mat;
}
