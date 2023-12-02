#ifndef MYOPENGL_H
#define MYOPENGL_H


#include <QtWidgets/QOpenGLWidget>
#include <QtGui/QKeyEvent>
#include <QtGui/QOpenGLFunctions>
#include <QtGui/QOpenGLExtraFunctions>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QOpenGLShader>
#include <QtGui/QOpenGLVertexArrayObject>
#include <QtGui/QOpenGLBuffer>
#include <QtGui/QOpenGLTexture>
#include <QWheelEvent>
#include <QOpenGLFunctions>

#include "opengl_base_data.h"
#include "my_opengl_camera.h"
#include "stl_helper.h"
using Float4x4 = float[4][4];

// 模型信息 描述
struct ModelInfo {
    std::string modelPath;
    float rpy[3];
    float t[3];
    int parentIndex;
    float linkRpy[3];
    float linkT[3];
    float axis[3];
    float color[4];
    int dir;
    float angleBaseOffset;
};
using ModelInfos = std::vector<ModelInfo>;

// 模型 绘制 信息 类型
struct ModelData : OpenGLBaseData{
    // 位置 属性 颜色 等
    float theta;
    QMatrix4x4 baseOffsetMatrix;
    QMatrix4x4 coorOffset;
    QMatrix4x4 thetaMatrix;
    QMatrix4x4 stlToPosMatrix;
    QVector3D axis;
    QMatrix4x4 *pParent, *pChild;
    QMatrix4x4 posMatrix;
    Light light;
    Material matrial;
    QVector4D color;
    int dir;
    float angleBaseOffset;
    int parentIndex;
    // opengl 相关
    QOpenGLShaderProgram *pShaderProgram;
    QOpenGLVertexArrayObject *pVao;
    QOpenGLBuffer *pVbo;
    int pointNum;
};
using ModelDatas = std::vector<ModelData>;

// 最近点 绘制
struct NearstPointPaintData {
    // bool isPaint = false;
    QOpenGLShaderProgram *pShader;
    QOpenGLVertexArrayObject *pVao;
    QOpenGLBuffer *pVbo;
};
// 路径 绘制
struct RoutePaintData {
    QOpenGLShaderProgram *pShader;
    QOpenGLVertexArrayObject *pVao;
    QOpenGLBuffer *pVbo;
};

// opengl 类
class MyOpenGLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
    // 单位 矩阵
    static const QMatrix4x4 eye4x4;
signals:


public:
    MyOpenGLWidget(QWidget *parent = nullptr);
    ~MyOpenGLWidget();

    // 设置 背景
    void setBackground(const QImage &img);

    // 主手 模型 接口
    int getMasterModelNum();
    // 1 起始
    int setMasterJointAngle(int index, float angle);
    // 0 起始
    void setMasterJointAngles(std::vector<float> jointAngles);
    // 从手 模型 接口
    int getSlaveModelNum();
    // 1 起始
    int setSlaveJointAngle(int index, float angle);
    // 0 起始
    void setSlaveJointAngles(std::vector<float> jointAngles);

    // 绘制 控制 标志位 设置，
    void setIsPaintMaster(bool flag) { isPaintMaster = flag; }              // 绘制 主手
    void setIsPaintSlave(bool flag) { isPaintSlave = flag; }                // 绘制 从手
    void setIsPaintObs(bool flag) { isPaintObs = flag; }                    // 绘制 障碍物
    void setIsPaintTarget(bool flag) { isPaintTarget = flag; }              // 绘制 目标物
    void setIsEnableFcl(bool flag) { isEnableFcl = flag; }                  // 使能 fcl 碰撞检测
    void setIsPaintNearstPoint(bool flag) { isPaintNearstPoint = flag; }    // 绘制 最近点
    void setIsPaintRoute(bool flag) { isPaintRoute = flag; }                // 绘制 末端 路径
    void setIsRecord(bool flag) { isRecord = flag; }                        // 是否 记录

    // 黑色 背景
    void setBackGroundBlack(bool flag) {
        isBackgroundBlack = flag;
        if (isBackgroundBlack) {
            isUpdateNewBackground = true;
            newBackground = blackImg;
        }
    }
    // 抓取 目标物体
    void graspTarget(bool flag) {
        isGrasp = flag;
        if (isSlave && isTar) {
            graspPosMatrix = slaveModelDatas[slaveModelDatas.size() - 1].posMatrix.inverted() * tarModelDatas[0].posMatrix;
        }
    }
    // 按键 响应
    void doKeyPressEvent(QKeyEvent *event);

    SimpleCamera &getCamera() { return camera; }    // 获取 相机 引用， 用于 设置 相机

protected:
    // opengl 相关
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    // 鼠标， 按键 响应函数
    void keyPressEvent(QKeyEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    // 模型 相关
    int getModelNum(const ModelDatas &modelDatas);  // 获取 个数
    int setModelJointAngle(ModelDatas &modelDatas, int index, float angle); // 设置 关节角 index 方式
    void setModelJointAngles(ModelDatas &modelDatas, std::vector<float> jointAngles);   // 设置 关节角 vector 方式
    int setModelPosMatrix(ModelDatas &modelDatas, int index, const QMatrix4x4 &posMatrix);  // 矩阵方式 设定
    void setModelPosMatrixs(ModelDatas &modelDatas, const std::vector<QMatrix4x4> &posMatrixs); // 矩阵方式 设定
    // 加载 模型
    void loadAllModel(const std::string &loadDesPath = ""); // 读取 模型描述文件 描述
    static void readModelDescription(const std::string &descriptionPath, ModelInfos &modelInfos);   // 读取 描述
    void addModelsToModelDatasFromModelInfos(const ModelInfos &modelInfos, ModelDatas &modelDatas);   // 添加模型 到 fcl
    void initStlModelData(ModelData &modelData, const ModelInfo &modelInfo);  // 初始化 stl 模型
    // 根据 theta 更新 模型 位姿
    void updateModelPos(ModelDatas &modelDatas);
    // 获取 末端 工具坐标系 位置, 不会进行 modeldata计算，直接使用其中 pos 计算
    void getToolPos(const ModelDatas &modelDatas, const QVector3D &toolOffset, QVector3D &toolPos);

    // fcl 碰撞检测
    static void qMatrixToFloat4x4(const QMatrix4x4 &qMatrix, Float4x4 &floatMatrix);
    // 绘制 最近点
    void initPointPaint();
    void destroy(NearstPointPaintData &nearstPointsPaintData);
    // 绘制 路径 相关
    void initRoutPaint();
    void paintRoute(const Points &points, int startPointIndex = 0, int endPointNotPaintNum = 0);
    void paintRoute(const std::string &filePath, int startPointIndex = 0, int endPointNotPaintNum = 0);
    void routeAddPoint(const Point &point);
    void destroy(RoutePaintData &routePaintData);

    // 打印 qmatrix4x4
    static void printMatrix(const QMatrix4x4 matrix);

    // 着色器 初始化
    void initShaderProgramByCode(const     QString &vs        , const     QString &fs        , QOpenGLShaderProgram &shaderPargram);
    void initShaderProgramByCode(const std::string &vs        , const std::string &fs        , QOpenGLShaderProgram &shaderPargram);
    void initShaderProgramByFile(const     QString &vsFilePath, const     QString &fsFilePath, QOpenGLShaderProgram &shaderPargram);
    void initShaderProgramByFile(const std::string &vsFilePath, const std::string &fsFilePath, QOpenGLShaderProgram &shaderPargram);

    // 背景 初始化， 更新， 绘制
    void initBackgroundData();
    void updateBackground();
    void paintBackground();

    // 模型 绘制
    void paintModel(ModelData &modelData);
    void setModelPos(ModelData &modelData, const QMatrix4x4 &posMatrix) { modelData.posMatrix = posMatrix; }

    // destroy 相关 资源
    void destroy(OpenGLDataWithIndexTexture &data);
    void destroy(ModelData &data);
private:
    // 相机

    SimpleCamera camera;

    // 控制 标志位
    bool isPaintMaster = false, isPaintSlave = false, isPaintObs = false, isPaintTarget = false;    // 绘制 主手， 从手， 障碍物， 目标物
    bool isEnableFcl = false, isPaintNearstPoint = false, isPaintRoute = false; // 开启 fcl 碰撞检测， 绘制 最近点， 末端路径
    bool isBackgroundBlack = false; // 背景 黑色

    // 模型 信息
    bool isMaster = false, isSlave = false, isObs = false, isTar = false;   // 是否 有 主手、 从手、 障碍物、 目标物 模型
    ModelDatas masterModelDatas, slaveModelDatas, obsModelDatas, tarModelDatas; // 主手、 从手、 障碍物、 目标物 模型信息

    // 背景  绘制 相关
    OpenGLDataWithIndexTexture backgroundData;  // 绘制 opengl 结构体
    QImage newBackground;   // 新 背景 图片
    QImage blackImg;
    bool isUpdateNewBackground = false; // 是否 更新背景， 相当于 数据锁

    // 最近点 绘制 opengl 模块
    NearstPointPaintData nearstPointsPaintData;

    // 路径 绘制 opengl 模块
    RoutePaintData routePaintData;
    bool isRecord;
    std::vector<Point> routePoints;

    // 抓取 标志位
    bool isGrasp = false;
    QMatrix4x4 graspPosMatrix;

    //
    QPoint last_mouse_position;
};

#endif // MYOPENGL_H
