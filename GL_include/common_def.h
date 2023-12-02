
#ifndef COMMON_DEF_HPP
#define COMMON_DEF_HPP

#include <iostream>
const int kJointNum = 6;

// 数据类型 定义
using JointAngle = float[kJointNum];    // 关节角
using JointTorque = float[kJointNum];   // 关节力矩
using MatrixR = float[3][3];            // R矩阵
using MatrixRt = float[4][4];           // Rt矩阵
using Rpyt = float[kJointNum];          // rpyt
using MasterLinkLen = float[kJointNum]; // 关节长度
// 力 和 力矩
struct ForceAndTorque {
    float force[3];
    float torque[3];
};
// 力
struct Force {
    float x, y, z;
    friend std::ostream &operator<<(std::ostream &os, const Force &f) {
        // os << "(" << f.x << ", " << f.y << ", " << f.z << ")";
        return os;
    }

    float &operator [] (const int &i) {
        if (i == 0) {
            return this->x;
        } else if (i == 1) {
            return this->y;
        } else if (i == 2) {
            return this->z;
        }
    }
};
// 力矩
struct Torque {
    float x, y, z;
};


#endif
