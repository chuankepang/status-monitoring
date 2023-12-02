#ifndef MY_OPENGL_CAMERA_H
#define MY_OPENGL_CAMERA_H

#include <QtGui/QVector3D>
#include <QtGui/QMatrix4x4>

#include <cmath>

#include <iostream>

using std::cout;
using std::endl;

const float kYaw = -90.f;
const float kPitch = 0.f;
const float kSpeed = 2.5f;
const float kSensitivity = 0.1f;
const float kZoom = 45.f;

// 模拟 opengl 相机 类
class MyOpenGLCamera {

public:
    enum CameraMovement {
        kForward,
        kBackward,
        kLeft,
        kRight
    };

public:
    // 构造
    MyOpenGLCamera(QVector3D initPos   = QVector3D{0.f, 0.f, 0.f}
                             , QVector3D initUp = QVector3D{0.f, 1.f, 1.f}
                             , QVector3D front = QVector3D{0.0f, 0.0f, -1.0f}
                             , float initYaw   = kYaw
                             , float initPitch = kPitch);
    // 析构
    ~MyOpenGLCamera();
    // 获取 当前 相机 位置
    QVector3D getPos();
    // 获取 view 矩阵
    QMatrix4x4 getViewMatrix();
    // 获取 透视 矩阵
    QMatrix4x4 getPerspectiveMatrix();
    // 处理 键盘 输入
    void processKeyboard(CameraMovement direction, float deltaTime = 0.005f);
    // 处理 鼠标移动
    void processMouseMovement(float xOffset, float yOffset, bool isCheck = true);
    // 处理 鼠标 滚轮
    void processMouseScroll(float yOffset);
    // 设置 fov
    void setFov(float val) { fov = val; }
    // 设置 视角长宽
    void setViewPortSize(int setWidth, int setHeight) { viewPortWidth = setWidth; viewPortHeight = setHeight; }
private:
    // 更新 矩阵
    void updateCameraVectors();
    // 转换
    static float toRadians(const float degree);

private:
    QVector3D position, front, up, right, worldUp;  // 相机参数向量
    float yaw, pitch;   // 角度描述
    float movementSpeed, mouseSensitivity, zoom;    // 控制 参数
    float fov;  // 视角
    int viewPortWidth, viewPortHeight;  // 长宽

};

// 简单 相机
class SimpleCamera {
public:
    // 构造函数
    SimpleCamera(QVector3D initPos = QVector3D{0.f, 0.f, 0.f},
                 QVector3D initTargetPos = QVector3D{0.f, 0.f, 1.f},
                 QVector3D initUp = QVector3D{0.f, 1.f, 0.f},
                 float initFov = 45,
                 float initAspectRatio = 1.0) : pos(initPos), targetPos(initTargetPos), up(initUp), fov(initFov), aspectRatio(initAspectRatio) {
        update();
        }
    // 获取 位置
    QVector3D getPos() {
        return pos;
    }
    QVector3D getTargetPos() {
        return targetPos;
    }
    // 获取 view  矩阵
    QMatrix4x4 getViewMatrix() {
        return viewMatrix;
    }
    // 获取 perspect 矩阵
    QMatrix4x4 getPerspectiveMatrix() {
        return prespectiveMatrix;
    }
    // 重置相机
    void resetCamera(QVector3D initPos = QVector3D{0.f, 0.f, 0.f},
                     QVector3D initTargetPos = QVector3D{0.f, 0.f, 1.f},
                     float initFov = 45,
                     float initAspectRatio = 1.0) {
        pos = initPos;
        targetPos = initTargetPos;
        fov = initFov;
        aspectRatio = initAspectRatio;
        update();
    }
    // 设置 位置
    void setPos(QVector3D newPos) {
        pos = newPos;
        update();
    }
    // 设置 目标 位置
    void setTargetPos(QVector3D newTargetPos) {
        targetPos = newTargetPos;
        update();
    }
    // 设置 上 向量
    void setUp(QVector3D newUp) {
        up = newUp;
        update();
    }
    // 设置 fov
    void setFov(float newFov) {
        fov = newFov;
        update();
    }
    // 设置 长宽比
    void setAspectRatio(float newAspectRatio) {
        aspectRatio = newAspectRatio;
        update();
    }
private:
    // 根据参数跟新矩阵
    void update() {
        QVector3D forward = targetPos - pos;
        QVector3D right;
        if (forward.y() != 0 || forward.x() != 0) {
            float len = sqrt(pow(forward.x(), 2) + pow(forward.y(), 2));
            right = QVector3D{forward.y(), -forward.x(), -len};
            right.setZ(right.z()*0.1);
            right = right.normalized();
        } else {
            right = QVector3D{0, 1, 0};
        }

        up = QVector3D::crossProduct(right, forward).normalized();
        // cout << "pos: " << pos.x() << ", " << pos.y() << ", " << pos.z() << endl
        //      << "target pos: " << targetPos.x() << ", " << targetPos.y() << ", " << targetPos.z() << endl
        //      << "forward: " << forward.x() << ", " << forward.y() << ", " << forward.z() << endl
        //      << "right: " << right.x() << ", " << right.y() << ", " << right.z() << endl
        //      << "up: " << up.x() << ", " << up.y() << ", " << up.z() << endl;

        viewMatrix.setToIdentity();
        viewMatrix.lookAt(pos, targetPos, up);

        prespectiveMatrix.setToIdentity();
        prespectiveMatrix.perspective(fov, aspectRatio, 0.1f, 1000.f);
    }
private:
    QVector3D pos;  // 位置
    QVector3D targetPos;    // 目标位置
    QVector3D up;   // 上向量
    float fov;  // 视角
    float aspectRatio;  // 比例
    QMatrix4x4 viewMatrix;  // view 矩阵
    QMatrix4x4 prespectiveMatrix;   // perspective 矩阵
};

#endif

