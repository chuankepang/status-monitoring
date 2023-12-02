#include "GL_include/myopenglwidget.h"


#include <QtCore/QTimer>

#include "GL_include/stl_helper.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#define M_PI       3.14159265358979323846   // pi

using std::cout;
using std::endl;

const int kWidgetWidth = 16 * 72;
const int kWidgetHeight = 9 * 72;

const std::string kStlModelVertexShaderFilePath     = "../502/GL/shader_source/model.vs";
const std::string kStlModelFragmentShaderFilePath   = "../502/GL/shader_source/model.fs";
const std::string kPointVertexShaderFilePath     = "../502/GL/shader_source/point.vs";
const std::string kPointFragmentShaderFilePath   = "../502/GL/shader_source/point.fs";

const std::string kSide1ImgPath = "../502/GL/img_source/side1.png";
const std::string kSide2ImgPath = "../502/GL/img_source/side2.png";

const std::string kTexture1ImgPath = "../502/GL/img_source/black.jpeg";
const std::string kTexture2ImgPath = "../502/GL/img_source/texture2.png";

const std::string kLoadDesPath = "../502/GL/stl_model/load_des.txt";
const std::string kMasterModelDescriptionPath = "../502/GL/stl_model/master/model_description.txt";
const std::string kSlaveDescriptionPath = "../502/GL/stl_model/slave/model_description.txt";
const std::string kObsDescriptionPath = "../502/GL/stl_model/obs/model_description.txt";

//const std::string kRoutePath = "/home/houyp/remote/python_plot/out.txt";

const QVector3D toToolPos{0, 0.17f, 0.};

const QMatrix4x4 MyOpenGLWidget::eye4x4 =  {
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1,
};
MyOpenGLWidget::MyOpenGLWidget(QWidget *parent)
    : QOpenGLWidget(parent) ,
    blackImg{QSize(4, 4), QImage::Format_RGB888},

#ifdef SIMPLE_CAMERA
    // camera(QVector3D{1.0f, 1.0f, 1.0f},
    //        QVector3D{0.f, 0.f, 0.f},
    //        QVector3D{-1.f, 1.f, -1.f}
    //        , 45) {
    camera(QVector3D{1.0f, 1.0f, 1.0f},
           QVector3D{0.f, 0.f, 0.2f},
           QVector3D{-1.f, -1.f, 1.f},
           18,
           1.85) {
#else
    //         pos,                                   up,                      front
    camera(QVector3D(0.f, 0.f, 1.f), QVector3D(0.0f, 1.0f, 0.0f), QVector3D(1.0f, 0.0f, .0f)) {
#endif

    // setCursor(Qt::BlankCursor);
    // setMouseTracking(true);
//    setFixedSize(kWidgetWidth , kWidgetHeight);
    resize(kWidgetWidth,kWidgetHeight);
//    setMouseTracking(true);

    blackImg.fill(QColor(Qt::black).rgb());
}
MyOpenGLWidget::~MyOpenGLWidget() {
    makeCurrent();

    destroy(backgroundData);
    for (auto &it : masterModelDatas) {
        destroy(it);
    }
    for (auto &it : slaveModelDatas) {
        destroy(it);
    }
    for (auto &it : obsModelDatas) {
        destroy(it);
    }
    for (auto &it : tarModelDatas) {
        destroy(it);
    }

    destroy(nearstPointsPaintData);
    destroy(routePaintData);

    doneCurrent();
}

#define DESTROY_DATA(pData) if (pData) { \
    pData->destroy(); \
    delete pData; \
}

void MyOpenGLWidget::destroy(OpenGLDataWithIndexTexture &data) {
    if (data.pShaderProgram) {
        delete data.pShaderProgram;
    }
    DESTROY_DATA(data.pVao);
    DESTROY_DATA(data.pVbo);
    DESTROY_DATA(data.pEbo);
    DESTROY_DATA(data.pTexture1);
}
void MyOpenGLWidget::destroy(ModelData &data) {
    if (data.pShaderProgram) {
        delete data.pShaderProgram;
    }
    DESTROY_DATA(data.pVao);
    DESTROY_DATA(data.pVbo);
}

void MyOpenGLWidget::destroy(NearstPointPaintData &nearstPointsPaintData) {
    if (nearstPointsPaintData.pShader) {
        delete nearstPointsPaintData.pShader;
    }
    DESTROY_DATA(nearstPointsPaintData.pVao);
    DESTROY_DATA(nearstPointsPaintData.pVbo);
}

void MyOpenGLWidget::destroy(RoutePaintData &routePaintData) {
    if (routePaintData.pShader) {
        delete routePaintData.pShader;
    }
    DESTROY_DATA(routePaintData.pVao);
    DESTROY_DATA(routePaintData.pVbo);
}

void MyOpenGLWidget::initializeGL() {

    // init function
    initializeOpenGLFunctions();
    // 开启 深度测试 等 opengl 功能
    glEnable(GL_DEPTH_TEST | GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 加载模型， 初始化 绘制器
    initBackgroundData();   // 背景 绘制

    loadAllModel(kLoadDesPath);
    if (isMaster) {
        cout << "master model num: " << getMasterModelNum() << endl;
    }
    if (isSlave) {
        cout << "slave model num: " << getSlaveModelNum() << endl;
    }
    if (isObs) {
        cout << "has obs" << endl;
    }
    if (isTar) {
        cout << "has tar" << endl;
    }
    // 最近点 绘制器
    initPointPaint();
    // 路径 绘制器
    initRoutPaint();

}

void MyOpenGLWidget::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);
    // camera.setViewPortSize(w, h);
}

void MyOpenGLWidget::paintGL() {

    // gl clear
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // paint
    // 背景 更新
    if (isUpdateNewBackground) {
        updateBackground();
        isUpdateNewBackground = false;
    }
    // 绘制 背景
    paintBackground();
    // 模型 位置 更新, 并 绘制
    if (isMaster) {
        updateModelPos(masterModelDatas);
        if (isPaintMaster) {
            for (auto &it : masterModelDatas) {
                paintModel(it);
            }
        }
    }
    if (isSlave) {
        updateModelPos(slaveModelDatas);
        if (isPaintSlave) {
            for (auto &it : slaveModelDatas) {
                paintModel(it);
            }
        }
        QVector3D toolPos{0, 0, 0};
        getToolPos(slaveModelDatas, toToolPos, toolPos);

        Point toolPoint{toolPos[0], toolPos[1], toolPos[2]};

        if (isRecord) {
            routeAddPoint(toolPoint);   // 添加 路径点， 函数内 可进行 点校验， 滤波 等
        }
        // paintModel(slaveModelDatas[0]);
        // paintModel(slaveModelDatas[1]);
    }
    if (isObs) {
        updateModelPos(obsModelDatas);
        if (isPaintObs) {
            for (auto &it : obsModelDatas) {
                paintModel(it);
            }
        }
    }
    if (isTar) {
        if (!isGrasp) {
            updateModelPos(tarModelDatas);
        } else {
            tarModelDatas[0].posMatrix = slaveModelDatas[slaveModelDatas.size() - 1].posMatrix * graspPosMatrix;
            tarModelDatas[0].stlToPosMatrix = tarModelDatas[0].posMatrix * tarModelDatas[0].baseOffsetMatrix;
            tarModelDatas[0].stlToPosMatrix = tarModelDatas[0].posMatrix * tarModelDatas[0].baseOffsetMatrix;
            Float4x4 floatMatrix;
            qMatrixToFloat4x4(tarModelDatas[0].stlToPosMatrix, floatMatrix);
        }
        if (isPaintTarget) {
            for (auto &it : tarModelDatas) {
                paintModel(it);
            }
        }
    }
    // 绘制 末端 路径
    if (isPaintRoute) {
        paintRoute(routePoints);
    }
    paintRoute("../data/pos_and_force_data.txt");
    // 发送 位置 及 最小距离 信号
}
// 设置 背景
void MyOpenGLWidget::setBackground(const QImage &img) {
    if (!isUpdateNewBackground && !isBackgroundBlack) {
        // newBackground = img.scaled(560, 480);
        newBackground = img.scaled(960, 576);
        isUpdateNewBackground = true;
    }
}
// 打印 模型 信息
void printModelInfo(const ModelInfo &modelInfo) {
    cout << "===================== model info =================" << endl
         << modelInfo.modelPath
         << endl
         << modelInfo.rpy[0] << "\t" << modelInfo.rpy[1] << "\t" << modelInfo.rpy[2] << "\t"
         << modelInfo.t[0] << "\t" << modelInfo.t[1] << "\t" << modelInfo.t[2] << "\t"
         << endl
         << modelInfo.linkRpy[0] << "\t" << modelInfo.linkRpy[1] << "\t" << modelInfo.linkRpy[2] << "\t"
         << modelInfo.linkT[0] << "\t" << modelInfo.linkT[1] << "\t" << modelInfo.linkT[2] << "\t"
         << endl
         << modelInfo.axis[0] << "\t" << modelInfo.axis[1] << "\t" << modelInfo.axis[2]
         << endl
         << modelInfo.color[0] << "\t" << modelInfo.color[1] << "\t" << modelInfo.color[2] << "\t" << modelInfo.color[3]
         << endl
         << modelInfo.dir << "\t" << modelInfo.angleBaseOffset
         << endl;
}

void MyOpenGLWidget::qMatrixToFloat4x4(const QMatrix4x4 &qMatrix, Float4x4 &floatMatrix) {
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            floatMatrix[i][j] = qMatrix(i, j);
        }
    }
}
// 模型描述文件路径 读取
void MyOpenGLWidget::loadAllModel(const std::string &loadDesPath) {
    if (loadDesPath == "") {
        return;
    }
    std::fstream f(loadDesPath);
    std::string line;
    if (getline(f, line)) {
        if (line != "") {
            ModelInfos masterModelInfos;
            readModelDescription(line, masterModelInfos);
            addModelsToModelDatasFromModelInfos(masterModelInfos, masterModelDatas);
            isMaster = true;
        }
    }
    if (getline(f, line)) {
        if (line != "") {
            ModelInfos slaveModelInfos;
            readModelDescription(line, slaveModelInfos);
            addModelsToModelDatasFromModelInfos(slaveModelInfos, slaveModelDatas);
            isSlave = true;
        }
    }
    if (getline(f, line)) {
        if (line != "") {
            ModelInfos obsModelInfos;
            readModelDescription(line, obsModelInfos);
            addModelsToModelDatasFromModelInfos(obsModelInfos, obsModelDatas);
            isObs = true;
        }
    }
    if (getline(f, line)) {
        if (line != "") {
            ModelInfos tarModelInfos;
            readModelDescription(line, tarModelInfos);
            addModelsToModelDatasFromModelInfos(tarModelInfos, tarModelDatas);
            isTar = true;
        }
    }
}
// 模型 加载
void MyOpenGLWidget::readModelDescription(const std::string &descriptionPath, std::vector< ModelInfo > &modelInfos) {
    std::ifstream f(descriptionPath);
    std::string line;
    while (getline(f, line)) {
        std::stringstream sf(line);
        ModelInfo modelInfo;

        // 路径
        sf >> modelInfo.modelPath;
        if (modelInfo.modelPath.substr(0, 2) == "//") {
            continue;
        }
        // rpy
        sf >> modelInfo.rpy[0] >> modelInfo.rpy[1] >> modelInfo.rpy[2];
        // t
        sf >> modelInfo.t[0] >> modelInfo.t[1] >> modelInfo.t[2];
        // 关节 父 索引
        sf >> modelInfo.parentIndex;
        // 关节 rpy
        sf >> modelInfo.linkRpy[0] >> modelInfo.linkRpy[1] >> modelInfo.linkRpy[2];
        // 关节 t
        sf >> modelInfo.linkT[0] >> modelInfo.linkT[1] >> modelInfo.linkT[2];
        // 旋转轴
        sf >> modelInfo.axis[0] >> modelInfo.axis[1] >> modelInfo.axis[2];
        // 颜色
        sf >> modelInfo.color[0] >> modelInfo.color[1] >> modelInfo.color[2] >> modelInfo.color[3];
        // 方向
        sf >> modelInfo.dir;
        // 基本角度偏移值
        sf >> modelInfo.angleBaseOffset;
        // 打印 模型 信息
//        printModelInfo(modelInfo);
        // 添加
        modelInfos.push_back(modelInfo);
    }
    f.close();
    cout << "model info num: " << modelInfos.size() << endl;
}
// 添加 单个 stl 模型
void MyOpenGLWidget::addModelsToModelDatasFromModelInfos(const ModelInfos &modelInfos, ModelDatas &modelDatas) {
    for (auto it : modelInfos) {
        ModelData modelData;
        initStlModelData(modelData, it);
        modelDatas.push_back(modelData);
    }
}
// stl 模型 初始化
void MyOpenGLWidget::initStlModelData(ModelData &modelData, const ModelInfo &modelInfo) {
    // init param set
    modelData.light.ambient = QVector3D(0.6f, 0.6f, 0.6f);
    modelData.light.diffuse = QVector3D(0.6f, 0.6f, 0.6f);
    modelData.light.specular = QVector3D(0.2f, 0.2f, 0.2f);
    modelData.light.pos = QVector3D(1.0f, 1.0f, 1.0f);

    modelData.matrial.ambient = QVector3D(1.0f, 0.5f, 0.31f);
    modelData.matrial.diffuse = QVector3D(1.0f, 0.5f, 0.31f);
    modelData.matrial.specular = QVector3D(0.5f, 0.5f, 0.5f);
    modelData.matrial.shininess = 32;

    modelData.posMatrix = eye4x4;
    modelData.baseOffsetMatrix = eye4x4;
    modelData.baseOffsetMatrix.translate(modelInfo.t[0], modelInfo.t[1], modelInfo.t[2]);
    modelData.baseOffsetMatrix.rotate(modelInfo.rpy[2] * 180 / M_PI, 0, 0, 1);
    modelData.baseOffsetMatrix.rotate(modelInfo.rpy[1] * 180 / M_PI, 0, 1, 0);
    modelData.baseOffsetMatrix.rotate(modelInfo.rpy[0] * 180 / M_PI, 1, 0, 0);
    // modelData.baseOffsetMatrix = modelData.baseOffsetMatrix.inverted();
    // printMatrix(modelData.baseOffsetMatrix);
    modelData.coorOffset = eye4x4;
    modelData.coorOffset.translate(modelInfo.linkT[0], modelInfo.linkT[1], modelInfo.linkT[2]);
    modelData.coorOffset.rotate(modelInfo.linkRpy[2] * 180 / M_PI, 0, 0, 1);
    modelData.coorOffset.rotate(modelInfo.linkRpy[1] * 180 / M_PI, 0, 1, 0);
    modelData.coorOffset.rotate(modelInfo.linkRpy[0] * 180 / M_PI, 1, 0, 0);

    // printMatrix(modelData.coorOffset);
    modelData.theta = 0.f;
    modelData.axis[0] = modelInfo.axis[0];
    modelData.axis[1] = modelInfo.axis[1];
    modelData.axis[2] = modelInfo.axis[2];

    modelData.color[0] = modelInfo.color[0];
    modelData.color[1] = modelInfo.color[1];
    modelData.color[2] = modelInfo.color[2];
    modelData.color[3] = modelInfo.color[3];

    // dir and offset
    modelData.dir = modelInfo.dir;
    modelData.angleBaseOffset = modelInfo.angleBaseOffset;
    modelData.parentIndex = modelInfo.parentIndex;

    modelData.pointNum = 0;
    // shader program
    modelData.pShaderProgram = new QOpenGLShaderProgram();
    initShaderProgramByFile(kStlModelVertexShaderFilePath, kStlModelFragmentShaderFilePath, *modelData.pShaderProgram);

    // VAO
    modelData.pVao = new QOpenGLVertexArrayObject();
    modelData.pVao->create();
    modelData.pVao->bind();

    StlData stlData;

    std::vector< float > stlVertices, stlNorVeritices;
    StlHelper::readStl(modelInfo.modelPath, stlData);
    StlHelper::stlDataToFloatVector(stlData, stlVertices);
    float maxVal = 0;
    for (auto it : stlVertices) {
        if (it > maxVal) {
            maxVal = it;
        }
        stlNorVeritices.push_back(it / 1.f);
    }
    // cout << "max val: " << maxVal << endl;
    modelData.pointNum = stlVertices.size() / 6;
    cout << modelInfo.modelPath << " point num: " << modelData.pointNum << endl;

    // VBO
    modelData.pVbo = new QOpenGLBuffer();
    modelData.pVbo->create();
    modelData.pVbo->bind();
    modelData.pVbo->allocate(stlNorVeritices.data(), sizeof(float) * stlNorVeritices.size());

    // buff loc
    modelData.pShaderProgram->setAttributeBuffer("aPos", GL_FLOAT, 0, 3, sizeof(GLfloat) * 6);
    modelData.pShaderProgram->setAttributeBuffer("aNormal", GL_FLOAT, sizeof(GLfloat) * 3, 3, sizeof(GLfloat) * 6);

    // buff enable
    glEnableVertexAttribArray(modelData.pShaderProgram->attributeLocation("aPos"));
    glEnableVertexAttribArray(modelData.pShaderProgram->attributeLocation("aNormal"));

    // 初始 角度
    modelData.theta = modelInfo.angleBaseOffset;
}
// 更新 模型 位置
void MyOpenGLWidget::updateModelPos(ModelDatas &modelDatas) {
    if (modelDatas.size() == 0) {
        return;
    }
    // 基本变换矩阵
    QMatrix4x4 baseTrans = eye4x4;
    baseTrans = eye4x4;
    // 基座 计算
    modelDatas[0].thetaMatrix.setToIdentity();
    modelDatas[0].thetaMatrix.rotate(modelDatas[0].theta, modelDatas[0].axis);

    modelDatas[0].posMatrix = baseTrans * modelDatas[0].coorOffset * modelDatas[0].thetaMatrix;
    for (int i = 1; i < modelDatas.size(); ++i) {
        // cout << i << " " << modelDatas[i].theta << endl;
        modelDatas[i].thetaMatrix.setToIdentity();
        modelDatas[i].thetaMatrix.rotate(modelDatas[i].theta, modelDatas[i].axis);
        // printMatrix(modelDatas[i].thetaMatrix);
        modelDatas[i].posMatrix = modelDatas[modelDatas[i].parentIndex-1].posMatrix * modelDatas[i].coorOffset * modelDatas[i].thetaMatrix;
    }
    for (int i = 0; i < modelDatas.size(); ++i) {
        modelDatas[i].stlToPosMatrix = modelDatas[i].posMatrix * modelDatas[i].baseOffsetMatrix;
        Float4x4 floatMatrix;
        qMatrixToFloat4x4(modelDatas[i].stlToPosMatrix, floatMatrix);
        // printMatrix(modelDatas[i].stlToPosMatrix);
        // cout << modelDatas[i].pose.linear() << endl;
        // cout << modelDatas[i].pose.translation() << endl;
    }
}

// 获取 工具 坐标系 位置
void MyOpenGLWidget::getToolPos(const ModelDatas &modelDatas, const QVector3D &toolOffset, QVector3D &toolPos) {
    QMatrix4x4 toToolMatrix, toolMatrix;
    toToolMatrix.setToIdentity();
    toToolMatrix.translate(toolOffset);
    toolMatrix = modelDatas[modelDatas.size() - 1].posMatrix * toToolMatrix;
    toolPos = QVector3D{toolMatrix(0, 3), toolMatrix(1, 3), toolMatrix(2, 3)};
}

// 绘制 单个 stl  模型
void MyOpenGLWidget::paintModel(ModelData &modelData) {
    modelData.pShaderProgram->bind();
    modelData.pVao->bind();

    modelData.pShaderProgram->setUniformValue("light.position", camera.getPos());
    modelData.pShaderProgram->setUniformValue("viewPos", camera.getPos());

    modelData.pShaderProgram->setUniformValue("light.ambient", modelData.light.ambient);
    modelData.pShaderProgram->setUniformValue("light.diffuse", modelData.light.diffuse);
    modelData.pShaderProgram->setUniformValue("light.specular", modelData.light.specular);
    // material properties
    modelData.pShaderProgram->setUniformValue("material.ambient", modelData.matrial.ambient);
    modelData.pShaderProgram->setUniformValue("material.diffuse", modelData.matrial.diffuse);
    modelData.pShaderProgram->setUniformValue("material.specular", modelData.matrial.specular); // specular lighting doesn't have full effect on this object's material
    modelData.pShaderProgram->setUniformValue("material.shininess", modelData.matrial.shininess);

    modelData.pShaderProgram->setUniformValue("color", modelData.color);

    modelData.pShaderProgram->setUniformValue("projection", camera.getPerspectiveMatrix());
    modelData.pShaderProgram->setUniformValue("view", camera.getViewMatrix());

    modelData.pShaderProgram->setUniformValue("model", modelData.stlToPosMatrix);
    glDrawArrays(GL_TRIANGLES, 0, modelData.pointNum);
}

// 按键 响应
void MyOpenGLWidget::doKeyPressEvent(QKeyEvent *event) {
    keyPressEvent(event);
}
// 按键 响应
void MyOpenGLWidget::keyPressEvent(QKeyEvent *event) {

    if (event->key() == Qt::Key_1) {
        QImage newBackground{QString::fromStdString(kSide1ImgPath)};
        setBackground(newBackground);
        update();
    } else if (event->key() == Qt::Key_2) {
        QImage oldBackground{QString::fromStdString(kTexture1ImgPath)};
        setBackground(oldBackground);
        update();
    } else if (event->key() == Qt::Key_3) {
        QImage oldBackground{QString::fromStdString(kSide2ImgPath)};
        setBackground(oldBackground);
        update();
    } else if (event->key() == Qt::Key_4) {
        update();
    } else if (event->key() == Qt::Key_5) {
        QImage oldBackground{QString::fromStdString(kSide2ImgPath)};
        oldBackground.fill((0, 0, 0));
        setBackground(oldBackground);
        update();
    } else if (event->key() == Qt::Key_6) {
        QImage oldBackground{QString::fromStdString(kSide2ImgPath)};
        oldBackground.fill((0, 2, 5));
        setBackground(oldBackground);
        update();
    }
}
// 打印 QMatrix4x4 矩阵
void MyOpenGLWidget::printMatrix(const QMatrix4x4 matrix) {
    cout << "===========" << endl;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            cout << matrix(i, j) << "\t";
        }
        cout << endl;
    }
}

// 主手 模型 个数
int MyOpenGLWidget::getMasterModelNum() {
    return getModelNum(masterModelDatas);
}
// 设置 主手 单个 关节角
int MyOpenGLWidget::setMasterJointAngle(int index, float angle) {
    if (isMaster) {
        return setModelJointAngle(masterModelDatas, index, angle);
    }
    return -1;
}
// 设置 主手 关节角
void MyOpenGLWidget::setMasterJointAngles(std::vector< float > jointAngles) {
    if (isMaster) {
        setModelJointAngles(masterModelDatas, jointAngles);
    }
}

// 从手 模型 个数
int MyOpenGLWidget::getSlaveModelNum() {
    return getModelNum(slaveModelDatas);
}
// 设置 从手 单个 关节角
int MyOpenGLWidget::setSlaveJointAngle(int index, float angle) {
    if (isSlave) {
        return setModelJointAngle(slaveModelDatas, index, angle);
    }
    return -1;
}
// 设置 从手 关节角
void MyOpenGLWidget::setSlaveJointAngles(std::vector< float > jointAngles) {
    if (isSlave) {
        std::vector<float> newAngles;
        for (const auto &it : jointAngles) {
            newAngles.push_back(it);
        }
        setModelJointAngles(slaveModelDatas, newAngles);
    }
}

// ModelDatas 类 获取模型 个数、 设置 单个 关节角、 设置 关节角 接口
inline
int MyOpenGLWidget::getModelNum(const ModelDatas &modelDatas) {
    return modelDatas.size();
}
inline
int MyOpenGLWidget::setModelJointAngle(ModelDatas &modelDatas, int index, float angle) {
    if (index > modelDatas.size()) {
        return -1;
    }
    modelDatas[index].theta = modelDatas[index].dir * angle  + modelDatas[index].angleBaseOffset;
}
inline
void MyOpenGLWidget::setModelJointAngles(ModelDatas &modelDatas, std::vector<float> jointAngles) {
    for (int i = 0; i < jointAngles.size(), i + 1< modelDatas.size(); ++i) {
        modelDatas[i + 1].theta = modelDatas[i].dir * jointAngles[i] + modelDatas[i + 1].angleBaseOffset;
    }
}
inline
int MyOpenGLWidget::setModelPosMatrix(ModelDatas &modelDatas, int index, const QMatrix4x4 &posMatrix) {
    if (index > modelDatas.size()) {
        return -1;
    }
    modelDatas[index].posMatrix = posMatrix;
}
inline
void MyOpenGLWidget::setModelPosMatrixs(ModelDatas &modelDatas, const std::vector<QMatrix4x4> &posMatrixs) {
    for (int i = 0; i < modelDatas.size(), i < posMatrixs.size(); ++i) {
        modelDatas[i].posMatrix = posMatrixs[i];
    }
}




// 初始化 背景  绘制器
void MyOpenGLWidget::initBackgroundData() {
    // shader program
    const QString backgroundVs = "\n"
                                 "#version 460 core\n"
                                 "\n"
                                 "layout (location = 0) in vec3 aPos;\n"
                                 "layout (location = 1) in vec2 aTexCoord;\n"
                                 "\n"
                                 "out vec2 TexCoord;\n"
                                 "\n"
                                 "void main()\n"
                                 "{\n"
                                 "   gl_Position = vec4(aPos, 1.0);\n"
                                 "   TexCoord = aTexCoord;\n"
                                 "};\n"
                                 ;
    const QString backgroundFs = "\n"
                                 "#version 460 core\n"
                                 "\n"
                                 "out vec4 FragColor;\n"
                                 "\n"
                                 "in vec2 TexCoord;\n"
                                 "\n"
                                 "uniform sampler2D texture1;\n"
                                 "\n"
                                 "void main()\n"
                                 "{\n"
                                 "   FragColor = texture(texture1, TexCoord);\n"
                                 "};\n"
                                 ;
    backgroundData.pShaderProgram = new QOpenGLShaderProgram();
    backgroundData.pShaderProgram->create();
    initShaderProgramByCode(backgroundVs, backgroundFs, *backgroundData.pShaderProgram);

    // VAO
    backgroundData.pVao = new QOpenGLVertexArrayObject();
    backgroundData.pVao->create();
    backgroundData.pVao->bind();

    // vertices
    const GLfloat backgroundVertices[] = {
        // positions        // texture coords
         1.0f,  1.0f, 1.0f, 1.0f, 1.0f, // top right
         1.0f, -1.0f, 1.0f, 1.0f, 0.0f, // bottom right
        -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, // bottom left
        -1.0f,  1.0f, 1.0f, 0.0f, 1.0f  // top left
    };

    // indices
    const unsigned int indices[] = {
        0, 1, 3,  // first Triangle
        1, 2, 3   // second Triangle
    };

    // VBO
    backgroundData.pVbo = new QOpenGLBuffer();
    backgroundData.pVbo->create();
    backgroundData.pVbo->bind();
    backgroundData.pVbo->allocate(backgroundVertices, sizeof(backgroundVertices));

    // EBO
    backgroundData.pEbo = new QOpenGLBuffer {QOpenGLBuffer::IndexBuffer};
    backgroundData.pEbo->create();
    backgroundData.pEbo->bind();
    backgroundData.pEbo->allocate(indices, sizeof(indices));

    // texture
    backgroundData.pTexture1 = new QOpenGLTexture {QOpenGLTexture::Target2D};
    backgroundData.pTexture1->create();
    backgroundData.pTexture1->setData(QImage(QString::fromStdString(kTexture1ImgPath)).mirrored());
    backgroundData.pTexture1->bind(0);

    // buff loc
    backgroundData.pShaderProgram->setAttributeBuffer("aPos", GL_FLOAT, 0, 3, sizeof(GLfloat) * 5);
    backgroundData.pShaderProgram->setAttributeBuffer("aTexCoord", GL_FLOAT, sizeof(GLfloat) * 3, 2, sizeof(GLfloat) * 5);
    backgroundData.pShaderProgram->setUniformValue("texture1", 0);

    // enable buff
    glEnableVertexAttribArray(backgroundData.pShaderProgram->attributeLocation("aPos"));
    glEnableVertexAttribArray(backgroundData.pShaderProgram->attributeLocation("aTexCoord"));
}
// 更新 背景
void MyOpenGLWidget::updateBackground() {
    backgroundData.pTexture1->destroy();
    backgroundData.pTexture1->create();
    backgroundData.pTexture1->setData(newBackground.mirrored());
}
// 绘制 背景
void MyOpenGLWidget::paintBackground() {

    glDisable(GL_DEPTH_TEST);
    backgroundData.pShaderProgram->bind();
    backgroundData.pVao->bind();
    backgroundData.pTexture1->bind(0);
    backgroundData.pShaderProgram->setUniformValue("texture1", 0);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glEnable(GL_DEPTH_TEST);
}





// 初始化 着色器
void MyOpenGLWidget::initShaderProgramByCode(const std::string &vs, const std::string &fs, QOpenGLShaderProgram &shaderPargram) {
    initShaderProgramByCode(QString::fromStdString(vs), QString::fromStdString(fs), shaderPargram);
}

void MyOpenGLWidget::initShaderProgramByFile(const std::string &vsFilePath, const std::string &fsFilePath, QOpenGLShaderProgram &shaderPargram) {
    initShaderProgramByFile(QString::fromStdString(vsFilePath), QString::fromStdString(fsFilePath), shaderPargram);
}

void MyOpenGLWidget::initShaderProgramByCode(const QString &vs, const QString &fs, QOpenGLShaderProgram &shaderPargram) {
    if (!shaderPargram.addShaderFromSourceCode(QOpenGLShader::Vertex, vs)) {
        cout << "vs add failed." << endl;
    }
    if (!shaderPargram.addShaderFromSourceCode(QOpenGLShader::Fragment, fs)) {
        cout << "fs add failed." << endl;
    }
    if (!shaderPargram.link()) {
        cout << "shader link error." << endl;
    }
}

void MyOpenGLWidget::initShaderProgramByFile(const QString &vsFilePath, const QString &fsFilePath, QOpenGLShaderProgram &shaderPargram) {
    if (!shaderPargram.addShaderFromSourceFile(QOpenGLShader::Vertex, vsFilePath)) {
        cout << "vs add failed." << endl;
    }
    if (!shaderPargram.addShaderFromSourceFile(QOpenGLShader::Fragment, fsFilePath)) {
        cout << "fs add failed." << endl;
    }
    if (!shaderPargram.link()) {
        cout << "shader link error." << endl;
    }
}

// 初始化 最近点 绘制器
void MyOpenGLWidget::initPointPaint() {

    nearstPointsPaintData.pShader = new QOpenGLShaderProgram();
    nearstPointsPaintData.pVao = new QOpenGLVertexArrayObject();
    nearstPointsPaintData.pVbo = new QOpenGLBuffer();
    nearstPointsPaintData.pShader->create();
    initShaderProgramByFile(kPointVertexShaderFilePath, kPointFragmentShaderFilePath, *nearstPointsPaintData.pShader);

    nearstPointsPaintData.pVao->create();
    nearstPointsPaintData.pVbo->create();
}


// 路径中 添加 点
void MyOpenGLWidget::routeAddPoint(const Point &point) {
    routePoints.push_back(point);
}
// 初始化 路径 绘制器
void MyOpenGLWidget::initRoutPaint() {
    routePaintData.pShader = new QOpenGLShaderProgram();
    routePaintData.pVao = new QOpenGLVertexArrayObject();
    routePaintData.pVbo = new QOpenGLBuffer();
    routePaintData.pShader->create();
    initShaderProgramByFile(kPointVertexShaderFilePath, kPointFragmentShaderFilePath, *routePaintData.pShader);

    routePaintData.pVao->create();
    routePaintData.pVbo->create();
}
void MyOpenGLWidget::paintRoute(const Points &points, int startPointIndex, int endPointNotPaintNum) {
    glDisable(GL_DEPTH_TEST);
    routePaintData.pShader->bind();
    routePaintData.pVao->bind();
    routePaintData.pVbo->bind();

    routePaintData.pVbo->allocate(points.data(), sizeof(Point) * points.size());
    routePaintData.pShader->setAttributeBuffer("aPos", GL_FLOAT, 0, 3, sizeof(GLfloat) * 3);

    glEnableVertexAttribArray(routePaintData.pShader->attributeLocation("aPos"));

    routePaintData.pShader->setUniformValue("view", camera.getViewMatrix());
    routePaintData.pShader->setUniformValue("projection", camera.getPerspectiveMatrix());

    glDrawArrays(GL_LINE_STRIP, startPointIndex, points.size() - startPointIndex - endPointNotPaintNum);
}
void MyOpenGLWidget::paintRoute(const std::string &filePath, int startPointIndex, int endPointNotPaintNum) {
    std::ifstream f(filePath);
    std::string line;
    std::vector<Point> points;
    int times;
    float x, y, z;
    bool isAttr;
    float fax, fay, faz;
    bool isRepu;
    float frx, fry, frz;
    bool isForce;
    float fx, fy, fz, fvalue;
    getline(f, line);
    while (getline(f, line)) {
        std::stringstream sf(line);
        sf >> times;
        sf >> x >> y >> z;
        sf >> isAttr >> fax >> fay >> faz;
        sf >> isRepu >> frx >> fry >> frz;
        sf >> isForce >> fx >> fy >> fz;
        sf >> fvalue;
        // x *= 1.1;
        points.push_back(Point{x, y, z});
    }
    f.close();
    paintRoute(points, startPointIndex, endPointNotPaintNum);
}

void MyOpenGLWidget::wheelEvent(QWheelEvent *event)
{
    // 获取滚轮滚动的角度
    int delta = event->angleDelta().y();

    // 根据滚动角度计算缩放因子
    float scaleFactor = 1.0f + delta / 1000.0f;
    // 获取当前视口大小
    QSize viewportSize = size();

    // 计算鼠标相对于视口中心的偏移量
    QPoint centerOffset = event->pos() - QPoint(viewportSize.width() / 2, viewportSize.height() / 2);

    // 将偏移量进行归一化，使其范围在[-1, 1]
    //    qreal normalizedX = static_cast<qreal>(centerOffset.x()) / viewportSize.width();
    //    qreal normalizedY = static_cast<qreal>(centerOffset.y()) / viewportSize.height();

    // 进行缩放操作
    QVector3D pos = camera.getPos();
    QVector3D targetPos = camera.getTargetPos();
    QVector3D posVector = pos - targetPos;
    QVector3D deltaPose = posVector * scaleFactor;
    camera.setPos(deltaPose+targetPos);

    // 请求重绘
    update();

    // 调用基类的事件处理函数
    QOpenGLWidget::wheelEvent(event);
}

void MyOpenGLWidget::mousePressEvent(QMouseEvent *event)
{
//    QOpenGLWidget::mouseMoveEvent(event);
    if (event->button() == Qt::MiddleButton || event->button() == Qt::LeftButton) {
       last_mouse_position = event->pos();
    }
}

void MyOpenGLWidget::mouseMoveEvent(QMouseEvent *event)
{
//    QOpenGLWidget::mousePressEvent(event);
    if (event->buttons() & Qt::MiddleButton) {
        QPoint current_mouse_position = event->pos();
        QPoint delta = current_mouse_position - last_mouse_position;

        QVector3D target_position = camera.getTargetPos();
        QVector3D camera_position = camera.getPos();

        QVector3D direction = target_position - camera_position;
        float distance = direction.length(); // 相机到目标的距离

        // 计算缩放因子
        float scale_factor_h = distance / height() *0.3; // 可根据需要调整缩放因子
        float scale_factor_w = distance / width() * 0.6;

        QVector3D right,up;

        if (direction.y() != 0 || direction.x() != 0) {
            float len = sqrt(pow(direction.x(), 2) + pow(direction.y(), 2));
            right = QVector3D{direction.y(), -direction.x(), -len};
            right.setZ(right.z()*0.1);
            right = right.normalized();
        } else {
            right = QVector3D{0, 1, 0};
        }

        up = QVector3D::crossProduct(right, direction).normalized();

        // 计算平移距离
        float distance_x = delta.x() * scale_factor_w;
        float distance_y = delta.y() * scale_factor_h;

        target_position += -distance_x * right + distance_y * up;
        camera_position += -distance_x * right + distance_y * up;

        last_mouse_position = current_mouse_position;

        camera.setPos(camera_position);
        camera.setTargetPos(target_position);

        update();
    }
    else if (event->buttons() & Qt::LeftButton) {
        QPoint current_mouse_position = event->pos();
        QPoint delta = current_mouse_position - last_mouse_position;

        // 计算旋转角度
        float rotation_angle_x = delta.y() * 0.5f; // 垂直方向旋转角度
        float rotation_angle_y = delta.x() * 0.5f; // 水平方向旋转角度


        QVector3D target_position = camera.getTargetPos();
        QVector3D camera_position = camera.getPos();


        QVector3D direction = target_position - camera_position;
//        float distance = direction.length(); // 相机到目标的距离

        // 计算缩放因子
//        float scale_factor_h = distance / height() *0.3; // 可根据需要调整缩放因子
//        float scale_factor_w = distance / width() * 0.6;

        QVector3D right,up;

        if (direction.y() != 0 || direction.x() != 0) {
            float len = sqrt(pow(direction.x(), 2) + pow(direction.y(), 2));
            right = QVector3D{direction.y(), -direction.x(), -len};
            right.setZ(right.z()*0.1);
            right = right.normalized();
        } else {
            right = QVector3D{0, 1, 0};
        }

        up = QVector3D::crossProduct(right, direction).normalized();

        // 绕目标位置进行旋转
        QMatrix4x4 rotation_matrix;
        rotation_matrix.rotate(rotation_angle_x, -right);
        rotation_matrix.rotate(rotation_angle_y, -up);

        camera_position = target_position + rotation_matrix * (camera_position - target_position);

        last_mouse_position = current_mouse_position;

        camera.setPos(camera_position);
        camera.setTargetPos(target_position);

        update();

    }
}

void MyOpenGLWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::MiddleButton || event->button() == Qt::LeftButton) {
        last_mouse_position = QPoint();
    }
}

