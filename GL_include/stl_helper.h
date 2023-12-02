#ifndef STL_HELPER_H
#define STL_HELPER_H
#include <string>
#include <cmath>
#include <vector>
#include <algorithm>
#include <iostream>

const float eps = 1e-6f;

// 点， 包含 加减乘除 等 操作
struct Point {
    float x, y, z;
    void shake() {
        x += (static_cast<float>(rand()) / RAND_MAX - 0.5f) * eps;
        y += (static_cast<float>(rand()) / RAND_MAX - 0.5f) * eps;
        z += (static_cast<float>(rand()) / RAND_MAX - 0.5f) * eps;
    }
    Point getShakePoint() const {
        return Point{x + (static_cast<float>(rand()) / RAND_MAX - 0.5f) * eps, y + (static_cast<float>(rand()) / RAND_MAX - 0.5f) * eps, z + (static_cast<float>(rand()) / RAND_MAX - 0.5f) * eps};
    }
    // void operator = (const Point &p) {
    //     x = p.x;
    //     y = p.y;
    //     z = p.z;
    // }
    friend std::ostream &operator<<(std::ostream &os, const Point &p) {
        os << "(" << p.x << ", " << p.y << ", " << p.z << ")";
        return os;
    }
    Point operator + (const Point &p) const {
        return {x + p.x, y + p.y, z + p.z};
    }
    Point operator - (const Point &p) const {
        return {x - p.x, y - p.y, z - p.z};
    }
    float operator * (const Point &p) const {
        return x * p.x + y * p.y + z * p.z;
    }
    Point operator / (float num) const {
        return {x / num, y / num, z / num};
    }
    Point cross(const Point &p) const {
        return {y * p.z - p.y * z, p.x * z - x * p.z, x * p.y - y * p.x};
    }
    float len() const {
        return sqrt(x * x + y * y + z * z);
    }
    bool operator == (const Point &point) const {
        return this->x == point.x && this->y == point.y && this->z == point.z;
    }
    bool operator > (const Point &point) const {
        if (this-> x > point.x) {
            return true;
        } else if (this->x == point.x) {
            if (this->y > point.y) {
                return true;
            } else if (this->y == point.y) {
                if (this->z > point.z) {
                    return true;
                }
            }
        }
        return false;
    }
    bool operator < (const Point &point) const {
        if (this-> x < point.x) {
            return true;
        } else if (this->x == point.x) {
            if (this->y < point.y) {
                return true;
            } else if (this->y == point.y) {
                if (this->z < point.z) {
                    return true;
                }
            }
        }
        return false;
    }
};

// 向量 同 点
using Normal = Point;

// 线
struct Line {
    Point p1, p2;
    Line(const Point &_p1, const Point &_p2) {
        // if (_p1 > _p2) {
        //     p1 = _p2;
        //     p2 = _p1;
        // } else {
        //     p1 = _p1;
        //     p2 = _p2;
        // }
        p1 = _p1;
        p2 = _p2;
    }
    bool operator == (const Line &line) const {
        return p1 == line.p1 && p2 == line.p2;
    }
    bool operator > (const Line &line) const {
        if (p1 > line.p1) {
            return true;
        } else if (p1 == line.p1 && p2 > line.p2) {
            return true;
        }
        return false;
    }
    bool operator < (const Line &line) const {
        if (p1 < line.p1) {
            return true;
        } else if (p1 == line.p1 && p2 < line.p2) {
            return true;
        }
        return false;
    }
};

// 面
struct Face {
    Point p1, p2, p3;
    Normal normal;
    Normal calNormal() const {
        return (p2 - p1).cross(p3 - p1);
    }
    bool above(const Point &p) const {
        return (p - p1) * calNormal() >= 0;
    }
    // float getAbove(const Point &p) const {
    //     return (p - p1) * calNormal();
    // }
    bool area() const {
        return calNormal().len() / 2;
    }
};

// vector
using Points = std::vector<Point>;
using Normals = std::vector<Normal>;
using Faces = std::vector<Face>;

using FaceIndex = std::vector<int>;
using FaceIndices = std::vector<FaceIndex>;

// stl 读取结果 结构体
struct StlData
{
    std::string FilePath;
    int faceNum;
    Faces faces;
};

// stl 相关 类
class StlHelper {
public:
    // 读取 stl 模型
    static int readStl(const std::string &FilePath, StlData &stlData);      // 读取为 stlData 类型
    static int readStl(const std::string &FilePath, std::vector< float > &stlData); // 读取 为 vector float
    // 打印 stl 模型 信息
    static void printStlData(const StlData &stlData);
    // stlData 转 vector float
    static void stlDataToFloatVector(const StlData &stlData, std::vector< float > &floatVector);

    // 求解 凸包
    static void solveConvexHull(const Points &rawPoints, Faces &convexFaces);   // 传入点 计算
    static void solveConvexHull(const Faces &faces, Faces &convexFaces);    // 传入 面 计算
    static void solveConvexHull(const Points &rawPoints, Points &convexPoints, FaceIndices &convexIndices); // 结果 为 凸包点 + 索引
    static void solveConvexHull(const Faces &faces, Points &convexPoints, FaceIndices &convexIndices); // 结果 为 凸包点 + 索引
    // 打印 面
    static void printFaces(const Faces &faces); // face 形式
    static void printFaces(const Points &points, const FaceIndices &indices);   // point + index 形式
    // 面 从 face 转为 point + index 格式
    static void FacesToPointsAndIndices(const Faces &faces, Points &points, FaceIndices &indices);
};



#endif // STL_HELPER_H
