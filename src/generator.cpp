#include "engine.h"
#include <algorithm>
#include <iostream>
#include <utility>
#ifdef _WIN32
#include <string>
#else
#include <cstring>
#endif

#include <fstream>
#include <sstream>
#include <vector>

#define _USE_MATH_DEFINES
#include <math.h>

#include <GL/glut.h>

struct Point3D {
  GLfloat x;
  GLfloat y;
  GLfloat z;
};

void genPlane(float length, int divisions, char *fileName) {
  std::ofstream File(fileName, std::ios::trunc);

  int fixDiv = 0;
  float ofset = 0;
  length = length / divisions;
  if (divisions % 2 == 1) {
    fixDiv = 1;
    ofset = length / 2;
  }
  const char *normal = " 1 0 0 ";
  const char *coord = {};
  float divInv = 1 / (float)divisions;

  std::vector<float> texCoords;
  for (int i = 0; i < divisions; i++) {
    for (int j = 0; j < divisions; j++) {
      texCoords.push_back(j * divInv);
      texCoords.push_back(i * divInv);
      texCoords.push_back(j * divInv);
      texCoords.push_back((i + 1) * divInv);
      texCoords.push_back((j + 1) * divInv);
      texCoords.push_back(i * divInv);
      texCoords.push_back(j * divInv);
      texCoords.push_back((i + 1) * divInv);
      texCoords.push_back((j + 1) * divInv);
      texCoords.push_back((i + 1) * divInv);
      texCoords.push_back((j + 1) * divInv);
      texCoords.push_back(i * divInv);
    }
  }

  int idx = 0;
  for (int i = -divisions / 2 - fixDiv; i < divisions / 2; i++) {
    for (int j = -divisions / 2 - fixDiv, idx = 0; j < divisions / 2; j++) {

      File << length * j + ofset << " " << 0.0f << " " << length * i + ofset;
      File << normal;
      File << texCoords[idx++] << " " << texCoords[idx++] << "\n";

      File << length * j + ofset << " " << 0.0f << " "
           << length * (i + 1) + ofset;
      File << normal;
      File << texCoords[idx++] << " " << texCoords[idx++] << "\n";

      File << length * (j + 1) + ofset << " " << 0.0f << " "
           << length * i + ofset;
      File << normal;
      File << texCoords[idx++] << " " << texCoords[idx++] << "\n";

      File << length * j + ofset << " " << 0.0f << " "
           << length * (i + 1) + ofset;
      File << normal;
      File << texCoords[idx++] << " " << texCoords[idx++] << "\n";

      File << length * (j + 1) + ofset << " " << 0.0f << " "
           << length * (i + 1) + ofset;
      File << normal;
      File << texCoords[idx++] << " " << texCoords[idx++] << "\n";

      File << length * (j + 1) + ofset << " " << 0.0f << " "
           << length * i + ofset;
      File << normal;
      File << texCoords[idx++] << " " << texCoords[idx++] << "\n";
    }
  }
  File.close();
}

void genBox(float length, int divisions, char *fileName) {
  std::ofstream File(fileName, std::ios::trunc);
  const char *normals[6] = {" 0 -1 0", " 0 1 0", " 0 0 1",
                            " 0 0 -1", " 1 0 0", " -1 0 0"};
  float start = -length / 2.0f;
  // BASE
  for (int i = 0; i < divisions; i++) {
    for (int j = 0; j < divisions; j++) {
      File << j * (length / divisions) + start << " " << start << " "
           << (i + 1) * (length / divisions) + start;
      File << normals[0] << "\n";
      File << j * (length / divisions) + start << " " << start << " "
           << i * (length / divisions) + start;
      File << normals[0] << "\n";
      File << (j + 1) * (length / divisions) + start << " " << start << " "
           << i * (length / divisions) + start;
      File << normals[0] << "\n";
      File << (j + 1) * (length / divisions) + start << " " << start << " "
           << (i + 1) * (length / divisions) + start;
      File << normals[0] << "\n";
      File << j * (length / divisions) + start << " " << start << " "
           << (i + 1) * (length / divisions) + start;
      File << normals[0] << "\n";
      File << (j + 1) * (length / divisions) + start << " " << start << " "
           << i * (length / divisions) + start;
      File << normals[0] << "\n";
    }
  }

  // TOPO
  for (int i = 0; i < divisions; i++) {
    for (int j = 0; j < divisions; j++) {
      File << j * (length / divisions) + start << " " << -start << " "
           << i * (length / divisions) + start;
      File << normals[1] << "\n";
      File << j * (length / divisions) + start << " " << -start << " "
           << (i + 1) * (length / divisions) + start;
      File << normals[1] << "\n";
      File << (j + 1) * (length / divisions) + start << " " << -start << " "
           << i * (length / divisions) + start;
      File << normals[1] << "\n";
      File << j * (length / divisions) + start << " " << -start << " "
           << (i + 1) * (length / divisions) + start;
      File << normals[1] << "\n";
      File << (j + 1) * (length / divisions) + start << " " << -start << " "
           << (i + 1) * (length / divisions) + start;
      File << normals[1] << "\n";
      File << (j + 1) * (length / divisions) + start << " " << -start << " "
           << i * (length / divisions) + start;
      File << normals[1] << "\n";
    }
  }

  // Lateral Frontal
  for (int i = 0; i < divisions; i++) {
    for (int j = 0; j < divisions; j++) {
      File << j * (length / divisions) + start << " "
           << i * (length / divisions) + start << " " << -start;
      File << normals[2] << "\n";
      File << (j + 1) * (length / divisions) + start << " "
           << i * (length / divisions) + start << " " << -start;
      File << normals[2] << "\n";
      File << (j + 1) * (length / divisions) + start << " "
           << (i + 1) * (length / divisions) + start << " " << -start;
      File << normals[2] << "\n";
      File << j * (length / divisions) + start << " "
           << i * (length / divisions) + start << " " << -start;
      File << normals[2] << "\n";
      File << (j + 1) * (length / divisions) + start << " "
           << (i + 1) * (length / divisions) + start << " " << -start;
      File << normals[2] << "\n";
      File << j * (length / divisions) + start << " "
           << (i + 1) * (length / divisions) + start << " " << -start;
      File << normals[2] << "\n";
    }
  }

  // Lateral Traseiro
  for (int i = 0; i < divisions; i++) {
    for (int j = 0; j < divisions; j++) {
      File << (j + 1) * (length / divisions) + start << " "
           << i * (length / divisions) + start << " " << start;
      File << normals[3] << "\n";
      File << j * (length / divisions) + start << " "
           << i * (length / divisions) + start << " " << start;
      File << normals[3] << "\n";
      File << (j + 1) * (length / divisions) + start << " "
           << (i + 1) * (length / divisions) + start << " " << start;
      File << normals[3] << "\n";
      File << (j + 1) * (length / divisions) + start << " "
           << (i + 1) * (length / divisions) + start << " " << start;
      File << normals[3] << "\n";
      File << j * (length / divisions) + start << " "
           << i * (length / divisions) + start << " " << start;
      File << normals[3] << "\n";
      File << j * (length / divisions) + start << " "
           << (i + 1) * (length / divisions) + start << " " << start;
      File << normals[3] << "\n";
    }
  }

  // Lateral Direito
  for (int i = 0; i < divisions; i++) {
    for (int j = 0; j < divisions; j++) {
      File << -start << " " << j * (length / divisions) + start << " "
           << i * (length / divisions) + start;
      File << normals[4] << "\n";
      File << -start << " " << (j + 1) * (length / divisions) + start << " "
           << i * (length / divisions) + start;
      File << normals[4] << "\n";
      File << -start << " " << (j + 1) * (length / divisions) + start << " "
           << (i + 1) * (length / divisions) + start;
      File << normals[4] << "\n";
      File << -start << " " << j * (length / divisions) + start << " "
           << i * (length / divisions) + start;
      File << normals[4] << "\n";
      File << -start << " " << (j + 1) * (length / divisions) + start << " "
           << (i + 1) * (length / divisions) + start;
      File << normals[4] << "\n";
      File << -start << " " << j * (length / divisions) + start << " "
           << (i + 1) * (length / divisions) + start;
      File << normals[4] << "\n";
    }
  }

  // Lateral Esquerdo
  for (int i = 0; i < divisions; i++) {
    for (int j = 0; j < divisions; j++) {
      File << start << " " << (j + 1) * (length / divisions) + start << " "
           << i * (length / divisions) + start;
      File << normals[5] << "\n";
      File << start << " " << j * (length / divisions) + start << " "
           << i * (length / divisions) + start;
      File << normals[5] << "\n";
      File << start << " " << (j + 1) * (length / divisions) + start << " "
           << (i + 1) * (length / divisions) + start;
      File << normals[5] << "\n";
      File << start << " " << (j + 1) * (length / divisions) + start << " "
           << (i + 1) * (length / divisions) + start;
      File << normals[5] << "\n";
      File << start << " " << j * (length / divisions) + start << " "
           << i * (length / divisions) + start;
      File << normals[5] << "\n";
      File << start << " " << j * (length / divisions) + start << " "
           << (i + 1) * (length / divisions) + start;
      File << normals[5] << "\n";
    }
  }
}

void genCone(float radius, float height, int slices, int stacks,
             char *fileName) {
  std::ofstream File(fileName, std::ios::trunc);

  int i;
  float step;
  step = 360.0 / slices;

  // Bottom
  for (i = 0; i < slices; i++) {
    File << 0.0f << " " << 0.0f << " " << 0.0f << std::endl;
    File << cos((i + 1) * step * M_PI / 180.0) * radius << " " << 0.0f << " "
         << -sin((i + 1) * step * M_PI / 180.0) * radius << std::endl;
    File << cos(i * step * M_PI / 180.0) * radius << " " << 0.0f << " "
         << -sin(i * step * M_PI / 180.0) * radius << std::endl;
  }

  // Body start
  int j;
  for (j = 1; j < stacks; j++) {
    for (i = 0; i <= slices; i++) {
      File << cos(i * step * M_PI / 180.0) * (radius * (stacks - j) / stacks)
           << " " << height * j / stacks << " "
           << -sin(i * step * M_PI / 180.0) * (radius * (stacks - j) / stacks)
           << std::endl;
      File << cos(i * step * M_PI / 180.0) *
                  (radius * (stacks - (j - 1)) / stacks)
           << " " << height * (j - 1) / stacks << " "
           << -sin(i * step * M_PI / 180.0) *
                  (radius * (stacks - (j - 1)) / stacks)
           << std::endl;
      File << cos((i + 1) * step * M_PI / 180.0) *
                  (radius * (stacks - j) / stacks)
           << " " << height * j / stacks << " "
           << -sin((i + 1) * step * M_PI / 180.0) *
                  (radius * (stacks - j) / stacks)
           << std::endl;

      File << cos((i + 1) * step * M_PI / 180.0) *
                  (radius * (stacks - j) / stacks)
           << " " << height * j / stacks << " "
           << -sin((i + 1) * step * M_PI / 180.0) *
                  (radius * (stacks - j) / stacks)
           << std::endl;
      File << cos(i * step * M_PI / 180.0) *
                  (radius * (stacks - (j - 1)) / stacks)
           << " " << height * (j - 1) / stacks << " "
           << -sin(i * step * M_PI / 180.0) *
                  (radius * (stacks - (j - 1)) / stacks)
           << std::endl;
      File << cos((i + 1) * step * M_PI / 180.0) *
                  (radius * (stacks - (j - 1)) / stacks)
           << " " << height * (j - 1) / stacks << " "
           << -sin((i + 1) * step * M_PI / 180.0) *
                  (radius * (stacks - (j - 1)) / stacks)
           << std::endl;
    }
  }

  // Body final
  for (i = 0; i <= slices; i++) {
    File << cos((i + 1) * step * M_PI / 180.0) *
                (radius * (stacks - j) / stacks)
         << " " << height << " "
         << -sin((i + 1) * step * M_PI / 180.0) *
                (radius * (stacks - j) / stacks)
         << std::endl;
    File << cos(i * step * M_PI / 180.0) *
                (radius * (stacks - (j - 1)) / stacks)
         << " " << height * (j - 1) / stacks << " "
         << -sin(i * step * M_PI / 180.0) *
                (radius * (stacks - (j - 1)) / stacks)
         << std::endl;
    File << cos((i + 1) * step * M_PI / 180.0) *
                (radius * (stacks - (j - 1)) / stacks)
         << " " << height * (j - 1) / stacks << " "
         << -sin((i + 1) * step * M_PI / 180.0) *
                (radius * (stacks - (j - 1)) / stacks)
         << std::endl;
  }
}

void genSphere(float radius, int slices, int stacks, char *fileName) {
  std::ofstream File(fileName, std::ios::trunc);

  std::vector<Point3D> spherePoints, normals;
  std::vector<std::pair<float, float>> texCoords;
  float sliceAngle, stackAngle, s, t;
  float sliceStep = 2 * M_PI / slices;
  float stackStep = M_PI / stacks;
  Point3D point, normal;
  float lenInv = 1.0f / radius;

  for (int i = 0; i <= stacks; i++) {
    stackAngle = M_PI / 2 - i * stackStep;
    point.y = radius * sinf(stackAngle);
    normal.y = point.y * lenInv;
    for (int j = 0; j <= slices; j++) {
      sliceAngle = j * sliceStep;
      point.x = (radius * cosf(stackAngle)) * sinf(sliceAngle);
      point.z = (radius * cosf(stackAngle)) * cosf(sliceAngle);
      spherePoints.push_back(point);

      normal.x = point.x * lenInv;
      normal.z = point.z * lenInv;
      normals.push_back(normal);

      s = (float)j / slices;
      t = (float)i / stacks;
      texCoords.push_back(std::make_pair(s, t));
    }
  }

  Point3D p1, p2, p3, p4, n1, n2, n3, n4;
  std::pair<float, float> t1, t2, t3, t4;
  std::reverse(texCoords.begin(), texCoords.end());
  int pi1, pi2;
  for (int i = 0; i < stacks; i++) {
    pi1 = i * (slices + 1);
    pi2 = (i + 1) * (slices + 1);
    for (int j = 0; j < slices; j++, pi1++, pi2++) {
      // Get 4 points per slice
      // p1 - p3
      // p2 - p4
      p1 = spherePoints[pi1];
      p2 = spherePoints[pi2];
      p3 = spherePoints[pi1 + 1];
      p4 = spherePoints[pi2 + 1];
      n1 = normals[pi1];
      n2 = normals[pi2];
      n3 = normals[pi1 + 1];
      n4 = normals[pi2 + 1];
      t1 = texCoords[pi1];
      t2 = texCoords[pi2];
      t3 = texCoords[pi1 + 1];
      t4 = texCoords[pi2 + 1];

      if (i == 0) { // Top stack
        File << p1.x << " " << p1.y << " " << p1.z;
        File << " " << n1.x << " " << n1.y << " " << n1.z;
        File << " " << t1.first << " " << t1.second << "\n";
        File << p2.x << " " << p2.y << " " << p2.z;
        File << " " << n2.x << " " << n2.y << " " << n2.z;
        File << " " << t2.first << " " << t2.second << "\n";
        File << p4.x << " " << p4.y << " " << p4.z;
        File << " " << n4.x << " " << n4.y << " " << n4.z;
        File << " " << t4.first << " " << t4.second << "\n";
      } else if (i == (stacks - 1)) { // Bottom Stack
        File << p1.x << " " << p1.y << " " << p1.z;
        File << " " << n1.x << " " << n1.y << " " << n1.z;
        File << " " << t1.first << " " << t1.second << "\n";
        File << p2.x << " " << p2.y << " " << p2.z;
        File << " " << n2.x << " " << n2.y << " " << n2.z;
        File << " " << t2.first << " " << t2.second << "\n";
        File << p3.x << " " << p3.y << " " << p3.z;
        File << " " << n3.x << " " << n3.y << " " << n3.z;
        File << " " << t3.first << " " << t3.second << "\n";
      } else { // The other stacks need 2 triangles
        File << p3.x << " " << p3.y << " " << p3.z;
        File << " " << n3.x << " " << n3.y << " " << n3.z;
        File << " " << t3.first << " " << t3.second << "\n";
        File << p1.x << " " << p1.y << " " << p1.z;
        File << " " << n1.x << " " << n1.y << " " << n1.z;
        File << " " << t1.first << " " << t1.second << "\n";
        File << p2.x << " " << p2.y << " " << p2.z;
        File << " " << n2.x << " " << n2.y << " " << n2.z;
        File << " " << t2.first << " " << t2.second << "\n";

        File << p3.x << " " << p3.y << " " << p3.z;
        File << " " << n3.x << " " << n3.y << " " << n3.z;
        File << " " << t3.first << " " << t3.second << "\n";
        File << p2.x << " " << p2.y << " " << p2.z;
        File << " " << n2.x << " " << n2.y << " " << n2.z;
        File << " " << t2.first << " " << t2.second << "\n";
        File << p4.x << " " << p4.y << " " << p4.z;
        File << " " << n4.x << " " << n4.y << " " << n4.z;
        File << " " << t4.first << " " << t4.second << "\n";
      }
    }
  }
}

struct Vector3D bezierFormula(float t, std::vector<struct Vector3D> p) {
  struct Vector3D result;
  // Bernstein Polinomials
  float bp03 = (1 - t) * (1 - t) * (1 - t);
  float bp13 = 3 * t * ((1 - t) * (1 - t));
  float bp23 = 3 * (t * t) * (1 - t);
  float bp33 = t * t * t;

  // Cubic Bezier curve formula
  result.x = bp33 * p[3].x + bp23 * p[2].x + bp13 * p[1].x + bp03 * p[0].x;
  result.y = bp33 * p[3].y + bp23 * p[2].y + bp13 * p[1].y + bp03 * p[0].y;
  result.z = bp33 * p[3].z + bp23 * p[2].z + bp13 * p[1].z + bp03 * p[0].z;
  return result;
}

struct Vector3D bezierPatch(float u, float v, std::vector<int> indexes,
                            std::vector<struct Vector3D> points) {
  struct Vector3D result;
  result.x = 0;
  result.y = 0;
  result.z = 0;
  std::vector<struct Vector3D> controlPoints;
  std::vector<struct Vector3D> bezierPoints;

  for (int i = 0; i < 16; i++) {
    // loadControl
    controlPoints.push_back(points[indexes[i]]);

    if (controlPoints.size() == 4) {
      bezierPoints.push_back(bezierFormula(u, controlPoints));
      controlPoints.clear();
    }
  }
  result = bezierFormula(v, bezierPoints);
  return result;
}

void genBezier(char *inFileName, int tesselation, char *outFileName) {

  std::ifstream inFile(inFileName);
  std::ofstream outFile(outFileName, std::ios::trunc);
  std::string line;

  std::vector<std::vector<int>> indexesVector;
  std::vector<struct Vector3D> points;

  getline(inFile, line);
  std::istringstream nPatchesStream(line);
  int nPatches = 0;
  nPatchesStream >> nPatches;

  for (int i = 0; i < nPatches; i++) {
    getline(inFile, line);
    std::replace(line.begin(), line.end(), ',', ' ');
    std::istringstream indexStream(line);
    std::vector<int> indexes;
    for (int j = 0; j < 16; j++) {
      int index;
      indexStream >> index;
      indexes.push_back(index);
    }
    indexesVector.push_back(indexes);
  }

  getline(inFile, line);
  std::istringstream nPointsStream(line);
  int nPoints = 0;
  nPointsStream >> nPoints;

  for (int i = 0; i < nPoints; i++) {
    getline(inFile, line);
    std::replace(line.begin(), line.end(), ',', ' ');
    std::istringstream pointStream(line);
    struct Vector3D newPoint;
    pointStream >> newPoint.x;
    pointStream >> newPoint.y;
    pointStream >> newPoint.z;
    points.push_back(newPoint);
  }

  for (int patch = 0; patch < nPatches; patch++) {
    for (int lin = 0; lin < tesselation; lin++) {
      for (int col = 0; col < tesselation; col++) {

        float x1 = (float)lin / tesselation;
        float y1 = (float)col / tesselation;
        float x2 = (float)(lin + 1) / tesselation;
        float y2 = (float)(col + 1) / tesselation;

        struct Vector3D end1, end2, end3, end4;
        end1 = bezierPatch(x1, y1, indexesVector[patch], points);
        end2 = bezierPatch(x1, y2, indexesVector[patch], points);
        end3 = bezierPatch(x2, y1, indexesVector[patch], points);
        end4 = bezierPatch(x2, y2, indexesVector[patch], points);

        outFile << end1.x << " " << end1.y << " " << end1.z << "\n";
        outFile << end2.x << " " << end2.y << " " << end2.z << "\n";
        outFile << end4.x << " " << end4.y << " " << end4.z << "\n";

        outFile << end1.x << " " << end1.y << " " << end1.z << "\n";
        outFile << end4.x << " " << end4.y << " " << end4.z << "\n";
        outFile << end3.x << " " << end3.y << " " << end3.z << "\n";
      }
    }
  }

  inFile.close();
  outFile.close();
}

int main(int argc, char *argv[]) {

  if (strcmp(argv[1], "plane") == 0) {
    genPlane(std::stof(argv[2]), std::stoi(argv[3]), argv[4]);
  } else if (strcmp(argv[1], "box") == 0) {
    genBox(std::stof(argv[2]), std::stoi(argv[3]), argv[4]);
  } else if (strcmp(argv[1], "cone") == 0) {
    genCone(std::stof(argv[2]), std::stof(argv[3]), std::stoi(argv[4]),
            std::stoi(argv[5]), argv[6]);
  } else if (strcmp(argv[1], "sphere") == 0) {
    genSphere(std::stof(argv[2]), std::stoi(argv[3]), std::stoi(argv[4]),
              argv[5]);
  } else if (strcmp(argv[1], "bezier") == 0) {
    genBezier(argv[2], std::stoi(argv[3]), argv[4]);
  }
  return 0;
}
