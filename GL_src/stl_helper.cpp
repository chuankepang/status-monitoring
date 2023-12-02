
#include <fstream>
#include <iostream>

#include <string.h>
#include <map>

#include "GL_include/stl_helper.h"

using std::cout;
using std::endl;

inline
void StlHelper::stlDataToFloatVector(const StlData &stlData, std::vector< float > &floatVector) {
    // printStlData(stlData);
    for (auto it : stlData.faces) {
        floatVector.push_back(it.p1.x);
        floatVector.push_back(it.p1.y);
        floatVector.push_back(it.p1.z);
        floatVector.push_back(it.normal.x);
        floatVector.push_back(it.normal.y);
        floatVector.push_back(it.normal.z);

        floatVector.push_back(it.p2.x);
        floatVector.push_back(it.p2.y);
        floatVector.push_back(it.p2.z);
        floatVector.push_back(it.normal.x);
        floatVector.push_back(it.normal.y);
        floatVector.push_back(it.normal.z);

        floatVector.push_back(it.p3.x);
        floatVector.push_back(it.p3.y);
        floatVector.push_back(it.p3.z);
        floatVector.push_back(it.normal.x);
        floatVector.push_back(it.normal.y);
        floatVector.push_back(it.normal.z);
    }
}
int StlHelper::readStl(const std::string &FilePath, std::vector< float > &stlFloatVectorData) {
    std::vector< float >().swap(stlFloatVectorData);

    StlData stlData;
    int rtn = readStl(FilePath, stlData);
    if (0 != rtn) {
        return rtn;
    }
    stlDataToFloatVector(stlData, stlFloatVectorData);
    return 0;
}
// int StlHelper::readStl(const std::string &FilePath, StlDataWithIndex &stlPointAndIndexTypeData) {
//     StlData stlData;
//     int rtn = readStl(FilePath, stlData);
//     if (0 != rtn) {
//         return rtn;
//     }
//     stlDataToStlDataWithIndex(stlData, stlPointAndIndexTypeData);
//     return 0;
// }
int StlHelper::readStl(const std::string &FilePath, StlData &stlData) {
    std::fstream in(FilePath, std::ifstream::in | std::ifstream::binary);
    
    // file name
    char s[80];
    if (in) {
        in.read(s, 80);
        stlData.FilePath = s;
    } else {
        return -1;
    }
    // patch num
    if (in) {
        in.read(s, 4);
        // stlData.faceNum = s[0] | (s[1] << 8) | (s[2] << 16) | (s[3] << 24);
        memcpy(&stlData.faceNum, &s[ 0], 4);
    } else {
        return -2;
    }
    // patch
    cout << "stl helper: " << FilePath << "patch num: " << stlData.faceNum << endl;
    Face face;
    for (int i = 0; i < stlData.faceNum; ++i) {
        if (in.read(s, 50)) {
            // Face.normal.x = s[ 0] | (s[ 1] << 8) | (s[ 2] << 16) | (s[ 3] << 24);
            // Face.normal.y = s[ 4] | (s[ 5] << 8) | (s[ 6] << 16) | (s[ 7] << 24);
            // Face.normal.z = s[ 8] | (s[ 9] << 8) | (s[10] << 16) | (s[11] << 24);

            // Face.p1.x = s[12] | (s[13] << 8) | (s[14] << 16) | (s[15] << 24);
            // Face.p1.y = s[16] | (s[17] << 8) | (s[18] << 16) | (s[19] << 24);
            // Face.p1.z = s[20] | (s[21] << 8) | (s[22] << 16) | (s[23] << 24);

            // Face.p2.x = s[24] | (s[25] << 8) | (s[26] << 16) | (s[27] << 24);
            // Face.p2.y = s[28] | (s[29] << 8) | (s[30] << 16) | (s[31] << 24);
            // Face.p2.z = s[32] | (s[33] << 8) | (s[34] << 16) | (s[35] << 24);

            // Face.p3.x = s[36] | (s[37] << 8) | (s[38] << 16) | (s[39] << 24);
            // Face.p3.y = s[40] | (s[41] << 8) | (s[42] << 16) | (s[43] << 24);
            // Face.p3.z = s[44] | (s[45] << 8) | (s[46] << 16) | (s[47] << 24);

            // Face.attr = s[48] | (s[49] << 8);

            memcpy(&face.normal.x, &s[ 0], 4);
            memcpy(&face.normal.y, &s[ 4], 4);
            memcpy(&face.normal.z, &s[ 8], 4);

            memcpy(&face.p1.x, &s[12], 4);
            memcpy(&face.p1.y, &s[16], 4);
            memcpy(&face.p1.z, &s[20], 4);

            memcpy(&face.p2.x, &s[24], 4);
            memcpy(&face.p2.y, &s[28], 4);
            memcpy(&face.p2.z, &s[32], 4);

            memcpy(&face.p3.x, &s[36], 4);
            memcpy(&face.p3.y, &s[40], 4);
            memcpy(&face.p3.z, &s[44], 4);

            // memcpy(&Face.attr, &s[48], 2);
            
            stlData.faces.push_back(face);
        } else {
            return -3;
        }
    }
    // too more
    // if (in) {
    //     return -4;
    // }

    return 0;
}

void StlHelper::printStlData(const StlData &stlData) {
    cout << "mode name: " << stlData.FilePath << endl;
    cout << "patch num: " << stlData.faceNum << endl;
    for (auto it : stlData.faces) {
        cout << "normal: " << it.normal.x << "\t" << it.normal.y << "\t" << it.normal.z
             << "\tp1: " << it.p1.x << "\t" << it.p1.y << "\t" << it.p1.z
             << "\tp2: " << it.p2.x << "\t" << it.p2.y << "\t" << it.p2.z
             << "\tp3: " << it.p3.x << "\t" << it.p3.y << "\t" << it.p3.z
            //  << "\tattr: " << it.attr
             << endl;
    }
}
// void StlHelper::printStlData(const StlDataWithIndex &stlData) {
//     cout << "mode name: " << stlData.FilePath << endl;
//     cout << "patch num: " << stlData.faceNum << endl;
//     for (int i = 0; i < stlData.points.size(); ++i) {
//         cout << "point: " << stlData.points[i].x << ",\t" << stlData.points[i].y << ",\t" << stlData.points[i].z
//              << "\tnormal: " << stlData.normals[i].x << "\t" << stlData.normals[i].y << "\t" << stlData.normals[i].z
//              << endl;
//     }
//     for (auto it : stlData.indices) {
//         cout << "patch point index: " << it[0] << ", " << it[1] << ", " << it[2] << endl;
//         // cout << "patch point index: ";

//         // for (auto it2 : it) {
//         //     cout << it2 << "\t";
//         // }
//         // cout << endl;
//     }
// }

// void StlHelper::stlDataToStlDataWithIndex(const StlData &stlData, StlDataWithIndex &stlPointAndIndexTypeData) {
//     stlPointAndIndexTypeData.FilePath = stlData.FilePath;
//     stlPointAndIndexTypeData.faceNum = stlData.faceNum;

//     Points rawPoints;
//     Normals rawNormals;
//     for (auto &it : stlData.faces) {
//         rawPoints.push_back(it.p1);
//         rawPoints.push_back(it.p2);
//         rawPoints.push_back(it.p3);
//         rawNormals.push_back(it.normal);
//     }

//     std::map<Point, std::vector<int>> pointAllIndex;
//     for (int i = 0; i < rawPoints.size(); ++i) {
//         Point point = rawPoints[i];
//         auto it = pointAllIndex.find(point);
//         if (it == pointAllIndex.end()) {
//             pointAllIndex[point] = (std::vector<int>{i});
//         } else {
//             it->second.push_back(i);
//         }
//     }
//     Points().swap(stlPointAndIndexTypeData.points);
//     Normals().swap(stlPointAndIndexTypeData.normals);
//     FaceIndices().swap(stlPointAndIndexTypeData.indices);
//     std::map<Point, int> pointToIndex;
//     int index = 0;
//     for (auto it : pointAllIndex) {
//         Normal normal = {0, 0, 0};
//         for (auto it2 : it.second) {
//             normal = normal + rawNormals[it2 / 3];
//         }
//         normal = normal / it.second.size();
//         stlPointAndIndexTypeData.points.push_back(it.first);
//         stlPointAndIndexTypeData.normals.push_back(normal);
//         pointToIndex[it.first] = index;
//         ++index;

//         // cout << "(" << it.first.x << ", " << it.first.y << ", " << it.first.z << "): ";
//         // for (auto it2 : it.second) {
//         //     cout << it2 << " ";
//         // }
//         // cout << endl;

//     }
//     for (auto it : stlData.faces) {
//         FaceIndex patchPointIndex;
//         patchPointIndex.push_back(pointToIndex.at(it.p1));
//         patchPointIndex.push_back(pointToIndex.at(it.p2));
//         patchPointIndex.push_back(pointToIndex.at(it.p3));
//         stlPointAndIndexTypeData.indices.push_back(patchPointIndex);
//     }
    
// }

void StlHelper::FacesToPointsAndIndices(const Faces &faces, Points &points, FaceIndices &indices) {
    Points rawPoints;
    for (const auto &it : faces) {
        rawPoints.push_back(it.p1);
        rawPoints.push_back(it.p2);
        rawPoints.push_back(it.p3);
    }
    std::map<Point, std::vector<int>> AllIndexInPointMap;
    for (int i = 0; i < rawPoints.size(); ++i) {
        Point &point = rawPoints[i];
        auto it = AllIndexInPointMap.find(point);
        if (it == AllIndexInPointMap.end()) {
            AllIndexInPointMap[point] = std::vector<int>(i);
        } else {
            it->second.push_back(i);
        }
    }
    Points().swap(points);
    FaceIndices().swap(indices);
    std::map<Point, int> pointToIndex;
    int index = 0;
    for (const auto &it : AllIndexInPointMap) {
        points.push_back(it.first);
        pointToIndex[it.first] = index;
        ++index;
    }
    for (const auto &it : faces) {
        indices.push_back(FaceIndex{pointToIndex[it.p1], pointToIndex[it.p2], pointToIndex[it.p3]});
    }
}

// // 三维 凸包 算法
// void StlHelper::solveConvexHull(const Points &rawPoints, Faces &convexFaces) {
//     if (rawPoints.size() < 4) {
//         return;
//     }
//     Points shakePoints;
//     std::map<Point, int> pointIndicesMap;
//     for (int i = 0; i < rawPoints.size(); ++i) {
//         // shakePoints.push_back(rawPoints[i]);
//         shakePoints.push_back(rawPoints[i].getShakePoint());
//         pointIndicesMap[shakePoints[i]] = i;
//         // cout << shakePoints[i] << endl;
//     }
//     convexFaces.push_back(Face{shakePoints[0], shakePoints[1], shakePoints[2]});
//     convexFaces.push_back(Face{shakePoints[2], shakePoints[1], shakePoints[0]});
//     for (int i = 3; i < shakePoints.size(); ++i) {
//         const Point &point = shakePoints[i];
//         Faces tmpFace;
//         int n = convexFaces.size() * 3;
//         bool *pIsSee = new bool[n * n];
//         for (const auto &it : convexFaces) {
//             bool isAbove = it.above(point);
//             // cout << "place normal " << it.calNormal();
//             // cout << "point " << point - it.p1 << 
//             // cout << "above: " << it.getAbove(point) << endl;
//             if (!isAbove) {
//                 tmpFace.push_back(it);
//                 // cout << "not above " << &it << endl;
//             }
            
//             int n1 = pointIndicesMap[it.p1],
//                 n2 = pointIndicesMap[it.p2],
//                 n3 = pointIndicesMap[it.p3];
//             pIsSee[n1 * n + n2] = pIsSee[n2 * n + n3] = pIsSee[n3 * n + n1] = isAbove;
//         }
//         for (const auto &it : convexFaces) {
//             int n1 = pointIndicesMap[it.p1],
//                 n2 = pointIndicesMap[it.p2],
//                 n3 = pointIndicesMap[it.p3];
//             if (pIsSee[n1 * n + n2] && !pIsSee[n2 * n + n1]) {
//                 tmpFace.push_back(Face{shakePoints[n1], shakePoints[n2], shakePoints[i]});
//             }
//             if (pIsSee[n2 * n + n3] && !pIsSee[n3 * n + n2]) {
//                 tmpFace.push_back(Face{shakePoints[n2], shakePoints[n3], shakePoints[i]});
//             }
//             if (pIsSee[n3 * n + n1] && !pIsSee[n1 * n + n3]) {
//                 tmpFace.push_back(Face{shakePoints[n3], shakePoints[n1], shakePoints[i]});
//             }
//         }
//         delete[] pIsSee;
//         convexFaces.swap(tmpFace);
//     }
// }

// 三维 凸包 算法
void StlHelper::solveConvexHull(const Points &rawPoints, Faces &convexFaces) {
    if (rawPoints.size() < 4) {
        return;
    }
    Points shakePoints;
    std::map<Point, int> pointIndicesMap;
    for (int i = 0; i < rawPoints.size(); ++i) {
        // shakePoints.push_back(rawPoints[i]);
        shakePoints.push_back(rawPoints[i].getShakePoint());
        pointIndicesMap[shakePoints[i]] = i;
        // cout << shakePoints[i] << endl;
    }
    // cout << "point num: " << shakePoints.size() << endl;
    convexFaces.push_back(Face{shakePoints[0], shakePoints[1], shakePoints[2]});
    convexFaces.push_back(Face{shakePoints[2], shakePoints[1], shakePoints[0]});
    for (int i = 3; i < shakePoints.size(); ++i) {
        const Point &point = shakePoints[i];
        Faces tmpFace;
        int n = convexFaces.size();
        // cout << "n: " << n << endl;
        // bool *pIsSee = new bool[n * n];
        // std::vector<bool> isSee(3 * n);
        std::map<Line, bool> isSeeMap;
        for (const auto &it : convexFaces) {
            bool isAbove = it.above(point);
            // cout << "place normal " << it.calNormal();
            // cout << "point " << point - it.p1 << 
            // cout << "above: " << it.getAbove(point) << endl;
            if (!isAbove) {
                tmpFace.push_back(it);
                // cout << "not above " << &it << endl;
            }
            
            int n1 = pointIndicesMap[it.p1],
                n2 = pointIndicesMap[it.p2],
                n3 = pointIndicesMap[it.p3];
            // pIsSee[n1 * n + n2] = pIsSee[n2 * n + n3] = pIsSee[n3 * n + n1] = isAbove;
            // isSee[n1 * n + n2] = isSee[n2 * n + n3] = isSee[n3 * n + n1] = isAbove;
            isSeeMap[Line{it.p1, it.p2}] = isAbove;
            isSeeMap[Line{it.p2, it.p3}] = isAbove;
            isSeeMap[Line{it.p3, it.p1}] = isAbove;
        }
        for (const auto &it : convexFaces) {
            int n1 = pointIndicesMap[it.p1],
                n2 = pointIndicesMap[it.p2],
                n3 = pointIndicesMap[it.p3];
            // if (pIsSee[n1 * n + n2] && !pIsSee[n2 * n + n1]) {
            //     tmpFace.push_back(Face{shakePoints[n1], shakePoints[n2], shakePoints[i]});
            // }
            // if (pIsSee[n2 * n + n3] && !pIsSee[n3 * n + n2]) {
            //     tmpFace.push_back(Face{shakePoints[n2], shakePoints[n3], shakePoints[i]});
            // }
            // if (pIsSee[n3 * n + n1] && !pIsSee[n1 * n + n3]) {
            //     tmpFace.push_back(Face{shakePoints[n3], shakePoints[n1], shakePoints[i]});
            // }
            if (isSeeMap[Line{it.p1, it.p2}] && !isSeeMap[Line{it.p2, it.p1}]) {
                tmpFace.push_back(Face{it.p1, it.p2, shakePoints[i]});
            }
            if (isSeeMap[Line{it.p2, it.p3}] && !isSeeMap[Line{it.p3, it.p2}]) {
                tmpFace.push_back(Face{it.p2, it.p3, shakePoints[i]});
            }
            if (isSeeMap[Line{it.p3, it.p1}] && !isSeeMap[Line{it.p1, it.p3}]) {
                tmpFace.push_back(Face{it.p3, it.p1, shakePoints[i]});
            }
        }
        convexFaces.swap(tmpFace);
        // isSee.~vector();
        // delete[] pIsSee;
        // cout << "delete [] over" << endl;
    }
}
void StlHelper::solveConvexHull(const Faces &faces, Faces &convexFaces) {
    Points rawFacePoints;
    FaceIndices rawFaceIndex;
    FacesToPointsAndIndices(faces, rawFacePoints, rawFaceIndex);
    solveConvexHull(rawFacePoints, convexFaces);
}
void StlHelper::solveConvexHull(const Points &rawPoints, Points &convexPoints, FaceIndices &convexIndices) {
    Faces convexFaces;
    solveConvexHull(rawPoints, convexFaces);
    FacesToPointsAndIndices(convexFaces, convexPoints, convexIndices);
}
void StlHelper::solveConvexHull(const Faces &faces, Points &convexPoints, FaceIndices &convexIndices) {
    Points rawFacePoints;
    FaceIndices rawFaceIndex;
    FacesToPointsAndIndices(faces, rawFacePoints, rawFaceIndex);
    Faces convexFaces;
    solveConvexHull(rawFacePoints, convexFaces);
    FacesToPointsAndIndices(convexFaces, convexPoints, convexIndices);
}


void StlHelper::printFaces(const Faces &faces) {
    for (const auto &it : faces) {
        cout << "face : " << it.p1 << "\t" << it.p2 << "\t" << it.p3 << endl;
    }
}

void StlHelper::printFaces(const Points &points, const FaceIndices &indices) {
    for (const auto &it : indices) {
        cout << "face : " << points[it[0]] << "\t" << points[it[0]] << "\t" << points[it[0]] << endl;
    }
}
