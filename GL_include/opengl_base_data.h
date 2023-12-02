#ifndef OPENGL_BASE_DATA_H
#define OPENGL_BASE_DATA_H

#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QOpenGLVertexArrayObject>
#include <QtGui/QOpenGLBuffer>
#include <QtGui/QOpenGLTexture>

// opengl 绘制 基本数据类型组合

// 着色器 + vao + vbo
struct OpenGLBaseData {
    QOpenGLShaderProgram *pShaderProgram;
    QOpenGLVertexArrayObject *pVao;
    QOpenGLBuffer *pVbo;
};

// 着色器 + vao + vbo + index
struct OpenGLDataWithIndex {
    QOpenGLShaderProgram *pShaderProgram;
    QOpenGLVertexArrayObject *pVao;
    QOpenGLBuffer *pVbo;
    QOpenGLBuffer *pEbo;
};

// 着色器 + vao + vbo + index + texture
struct OpenGLDataWithIndexTexture {
    QOpenGLShaderProgram *pShaderProgram;
    QOpenGLVertexArrayObject *pVao;
    QOpenGLBuffer *pVbo;
    QOpenGLBuffer *pEbo;
    QOpenGLTexture *pTexture1;
};

struct OpenGLDataWithIndexTexture2 {
    QOpenGLShaderProgram *pShaderProgram;
    QOpenGLVertexArrayObject *pVao;
    QOpenGLBuffer *pVbo;
    QOpenGLBuffer *pEbo;
    QOpenGLTexture *pTexture1, *pTexture2;
};
struct OpenGLDataWithTexture {
    QOpenGLShaderProgram *pShaderProgram;
    QOpenGLVertexArrayObject *pVao;
    QOpenGLBuffer *pVbo;
    QOpenGLBuffer *pEbo;
    QOpenGLTexture *pTexture1;
};

struct OpenGLDataWithTexture2 {
    QOpenGLShaderProgram *pShaderProgram;
    QOpenGLVertexArrayObject *pVao;
    QOpenGLBuffer *pVbo;
    QOpenGLTexture *pTexture1, *pTexture2;
};


// 光照 情况 数据类型定义
struct Light {
    QVector3D pos;
    QVector3D ambient;
    QVector3D diffuse;
    QVector3D specular;
};
struct Material {
    QVector3D ambient;
    QVector3D diffuse;
    QVector3D specular;
    float shininess;
};
#endif
