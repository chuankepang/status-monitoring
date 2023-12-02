
#include <cmath>
#include <iostream>

#include "GL_include/my_opengl_camera.h"
#define M_PI       3.14159265358979323846   // pi
using std::cout;
using std::endl;

MyOpenGLCamera::MyOpenGLCamera(QVector3D initPos
                             , QVector3D initUp
                             , QVector3D front
                             , float initYaw
                             , float initPitch)
    : position(initPos)
    , worldUp(initUp.normalized())
    , front(front.normalized())
    , yaw(initYaw)
    , pitch(initPitch)
    , movementSpeed(kSpeed)
    , mouseSensitivity(kSensitivity)
    , zoom(kZoom)
    , fov(45)
    , viewPortWidth(1)
    , viewPortHeight(1) {
    updateCameraVectors();
}
MyOpenGLCamera::~MyOpenGLCamera() {
    
}

QVector3D MyOpenGLCamera::getPos() {
    return position;
}
// inline
QMatrix4x4 MyOpenGLCamera::getViewMatrix() {
    return QMatrix4x4 {
        right.x(), up.x(), -front.x(), QVector3D::dotProduct(right, position),
        right.y(), up.y(), -front.y(), QVector3D::dotProduct(   up, position),
        right.z(), up.z(), -front.z(), QVector3D::dotProduct(front, position),
                0,      0,          0,                                      1,
    };
    // QMatrix4x4 m;
    // m.lookAt(position, position + front, up);
    // return m;
}
// inline
QMatrix4x4 MyOpenGLCamera::getPerspectiveMatrix() {
    QMatrix4x4 m;
    // m.perspective(fov, float(viewPortHeight) / (float)(viewPortWidth), 0.1f, 100.f);
    m.perspective(fov, 1.f, 0.1f, 1000.f);
    return m;
}
void MyOpenGLCamera::processKeyboard(CameraMovement direction, float deltaTime) {
    float velocity = movementSpeed * deltaTime;
    if (direction == CameraMovement::kForward) {
        position += front * velocity;
    } else if (direction == CameraMovement::kBackward) {
        position -= front * velocity;
    } else if (direction == CameraMovement::kLeft) {
        position -= right * velocity;
    } else if (direction == CameraMovement::kRight) {
        position += right * velocity;
    }
}
void MyOpenGLCamera::processMouseMovement(float xOffset, float yOffset, bool isCheck) {

    xOffset *= mouseSensitivity;
    yOffset *= mouseSensitivity;

    yaw   += xOffset;
    pitch -= yOffset;

    if (isCheck) {
        if (pitch > 89.0f) {
            pitch = 89.0f;
        } else if (pitch < -89.0f) {
            pitch = -89.0f;
        }
    }

    updateCameraVectors();
}
void MyOpenGLCamera::processMouseScroll(float yOffset) {
    fov -= yOffset;
    if (fov < 1.0f) {
        fov = 1.0f;
    } else if (fov > 89.0f) {
        fov = 89.0f;
    }
}
void MyOpenGLCamera::updateCameraVectors() {
    
    QVector3D tmpFront;
    tmpFront.setX(cos(toRadians(yaw)) * cos(toRadians(pitch)));
    tmpFront.setY(sin(toRadians(pitch)));
    tmpFront.setZ(sin(toRadians(yaw)) * cos(toRadians(pitch)));

    front = tmpFront.normalized();
    right = QVector3D::crossProduct(front, worldUp).normalized();
    up = QVector3D::crossProduct(right, front).normalized();

    cout << front[0] << ", " << front[1] << ", " << front[2] << endl 
         << right[0] << ", " << right[1] << ", " << right[2] << endl 
         <<    up[0] << ", " <<    up[1] << ", " <<    up[2] << endl;
}

inline
float MyOpenGLCamera::toRadians(const float degree) {
    return degree * M_PI / 180.0;
}
