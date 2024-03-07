#include "tinyxml2.h"
#include <GL/gl.h>
#include <GL/glut.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
// guide: https://shilohjames.wordpress.com/2014/04/27/tinyxml2-tutorial/
using namespace tinyxml2;
using namespace std;

#define _USE_MATH_DEFINES
#include <math.h>

struct Point3D {
  GLfloat x;
  GLfloat y;
  GLfloat z;
};

void genPlane(float length, int divisions) {
  ostringstream fileName;
  fileName << "../3d/plane_" << length << "_" << divisions << ".3d";
  ofstream File(fileName.str(), ios::trunc);

  int fixDiv = 0;
  float ofset = 0;
  length = length / divisions;
  if (divisions % 2 == 1) {
    fixDiv = 1;
    ofset = length / 2;
  }

  for (int i = -divisions / 2 - fixDiv; i < divisions / 2; i++) {
    for (int j = -divisions / 2 - fixDiv; j < divisions / 2; j++) {
      File << length * j + ofset << " " << 0.0f << " " << length * i + ofset
           << endl;
      File << length * j + ofset << " " << 0.0f << " "
           << length * (i + 1) + ofset << endl;
      File << length * (j + 1) + ofset << " " << 0.0f << " "
           << length * i + ofset << endl;
      File << length * j + ofset << " " << 0.0f << " "
           << length * (i + 1) + ofset << endl;
      File << length * (j + 1) + ofset << " " << 0.0f << " "
           << length * (i + 1) + ofset << endl;
      File << length * (j + 1) + ofset << " " << 0.0f << " "
           << length * i + ofset << endl;
    }
  }
  File.close();
}

void genBox(float length, int divisions) {
  ostringstream fileName;
  fileName << "../3d/box_" << length << "_" << divisions << ".3d";
  ofstream File(fileName.str(), ios::trunc);

  float start = -length / 2.0f;
  // BASE
  for (int i = 0; i < divisions; i++) {
    for (int j = 0; j < divisions; j++) {
      File << j * (length / divisions) + start << " " << start << " "
           << (i + 1) * (length / divisions) + start << endl;
      File << j * (length / divisions) + start << " " << start << " "
           << i * (length / divisions) + start << endl;
      File << (j + 1) * (length / divisions) + start << " " << start << " "
           << i * (length / divisions) + start << endl;
      File << (j + 1) * (length / divisions) + start << " " << start << " "
           << (i + 1) * (length / divisions) + start << endl;
      File << j * (length / divisions) + start << " " << start << " "
           << (i + 1) * (length / divisions) + start << endl;
      File << (j + 1) * (length / divisions) + start << " " << start << " "
           << i * (length / divisions) + start << endl;
    }
  }

  // TOPO
  for (int i = 0; i < divisions; i++) {
    for (int j = 0; j < divisions; j++) {
      File << j * (length / divisions) + start << " " << -start << " "
           << i * (length / divisions) + start << endl;
      File << j * (length / divisions) + start << " " << -start << " "
           << (i + 1) * (length / divisions) + start << endl;
      File << (j + 1) * (length / divisions) + start << " " << -start << " "
           << i * (length / divisions) + start << endl;
      File << j * (length / divisions) + start << " " << -start << " "
           << (i + 1) * (length / divisions) + start << endl;
      File << (j + 1) * (length / divisions) + start << " " << -start << " "
           << (i + 1) * (length / divisions) + start << endl;
      File << (j + 1) * (length / divisions) + start << " " << -start << " "
           << i * (length / divisions) + start << endl;
    }
  }

  // Lateral Frontal
  for (int i = 0; i < divisions; i++) {
    for (int j = 0; j < divisions; j++) {
      File << j * (length / divisions) + start << " "
           << i * (length / divisions) + start << " " << -start << endl;
      File << (j + 1) * (length / divisions) + start << " "
           << i * (length / divisions) + start << " " << -start << endl;
      File << (j + 1) * (length / divisions) + start << " "
           << (i + 1) * (length / divisions) + start << " " << -start << endl;
      File << j * (length / divisions) + start << " "
           << i * (length / divisions) + start << " " << -start << endl;
      File << (j + 1) * (length / divisions) + start << " "
           << (i + 1) * (length / divisions) + start << " " << -start << endl;
      File << j * (length / divisions) + start << " "
           << (i + 1) * (length / divisions) + start << " " << -start << endl;
    }
  }

  // Lateral Traseiro
  for (int i = 0; i < divisions; i++) {
    for (int j = 0; j < divisions; j++) {
      File << (j + 1) * (length / divisions) + start << " "
           << i * (length / divisions) + start << " " << start << endl;
      File << j * (length / divisions) + start << " "
           << i * (length / divisions) + start << " " << start << endl;
      File << (j + 1) * (length / divisions) + start << " "
           << (i + 1) * (length / divisions) + start << " " << start << endl;
      File << (j + 1) * (length / divisions) + start << " "
           << (i + 1) * (length / divisions) + start << " " << start << endl;
      File << j * (length / divisions) + start << " "
           << i * (length / divisions) + start << " " << start << endl;
      File << j * (length / divisions) + start << " "
           << (i + 1) * (length / divisions) + start << " " << start << endl;
    }
  }

  // Lateral Direito
  for (int i = 0; i < divisions; i++) {
    for (int j = 0; j < divisions; j++) {
      File << -start << " " << j * (length / divisions) + start << " "
           << i * (length / divisions) + start << endl;
      File << -start << " " << (j + 1) * (length / divisions) + start << " "
           << i * (length / divisions) + start << endl;
      File << -start << " " << (j + 1) * (length / divisions) + start << " "
           << (i + 1) * (length / divisions) + start << endl;
      File << -start << " " << j * (length / divisions) + start << " "
           << i * (length / divisions) + start << endl;
      File << -start << " " << (j + 1) * (length / divisions) + start << " "
           << (i + 1) * (length / divisions) + start << endl;
      File << -start << " " << j * (length / divisions) + start << " "
           << (i + 1) * (length / divisions) + start << endl;
    }
  }

  // Lateral Esquerdo
  for (int i = 0; i < divisions; i++) {
    for (int j = 0; j < divisions; j++) {
      File << start << " " << (j + 1) * (length / divisions) + start << " "
           << i * (length / divisions) + start << endl;
      File << start << " " << j * (length / divisions) + start << " "
           << i * (length / divisions) + start << endl;
      File << start << " " << (j + 1) * (length / divisions) + start << " "
           << (i + 1) * (length / divisions) + start << endl;
      File << start << " " << (j + 1) * (length / divisions) + start << " "
           << (i + 1) * (length / divisions) + start << endl;
      File << start << " " << j * (length / divisions) + start << " "
           << i * (length / divisions) + start << endl;
      File << start << " " << j * (length / divisions) + start << " "
           << (i + 1) * (length / divisions) + start << endl;
    }
  }
}

void genCone(float radius, float height, int slices, int stacks) {

  ostringstream fileName;
  fileName << "../3d/cone_" << radius << "_" << height << "_" << slices << "_"
           << stacks << ".3d";
  ofstream File(fileName.str(), ios::trunc);

  int i;
  float step;
  step = 360.0 / slices;

  // Bottom
  for (i = 0; i < slices; i++) {
    File << 0.0f << " " << 0.0f << " " << 0.0f << endl;
    File << cos((i + 1) * step * M_PI / 180.0) * radius << " " << 0.0f << " "
         << -sin((i + 1) * step * M_PI / 180.0) * radius << endl;
    File << cos(i * step * M_PI / 180.0) * radius << " " << 0.0f << " "
         << -sin(i * step * M_PI / 180.0) * radius << endl;
  }

  // Body start
  int j;
  for (j = 1; j < stacks; j++) {
    for (i = 0; i <= slices; i++) {
      File << cos(i * step * M_PI / 180.0) * (radius * (stacks - j) / stacks)
           << " " << height * j / stacks << " "
           << -sin(i * step * M_PI / 180.0) * (radius * (stacks - j) / stacks)
           << endl;
      File << cos(i * step * M_PI / 180.0) *
                  (radius * (stacks - (j - 1)) / stacks)
           << " " << height * (j - 1) / stacks << " "
           << -sin(i * step * M_PI / 180.0) *
                  (radius * (stacks - (j - 1)) / stacks)
           << endl;
      File << cos((i + 1) * step * M_PI / 180.0) *
                  (radius * (stacks - j) / stacks)
           << " " << height * j / stacks << " "
           << -sin((i + 1) * step * M_PI / 180.0) *
                  (radius * (stacks - j) / stacks)
           << endl;

      File << cos((i + 1) * step * M_PI / 180.0) *
                  (radius * (stacks - j) / stacks)
           << " " << height * j / stacks << " "
           << -sin((i + 1) * step * M_PI / 180.0) *
                  (radius * (stacks - j) / stacks)
           << endl;
      File << cos(i * step * M_PI / 180.0) *
                  (radius * (stacks - (j - 1)) / stacks)
           << " " << height * (j - 1) / stacks << " "
           << -sin(i * step * M_PI / 180.0) *
                  (radius * (stacks - (j - 1)) / stacks)
           << endl;
      File << cos((i + 1) * step * M_PI / 180.0) *
                  (radius * (stacks - (j - 1)) / stacks)
           << " " << height * (j - 1) / stacks << " "
           << -sin((i + 1) * step * M_PI / 180.0) *
                  (radius * (stacks - (j - 1)) / stacks)
           << endl;
    }
  }

  // Body final
  for (i = 0; i <= slices; i++) {
    File << cos((i + 1) * step * M_PI / 180.0) *
                (radius * (stacks - j) / stacks)
         << " " << height << " "
         << -sin((i + 1) * step * M_PI / 180.0) *
                (radius * (stacks - j) / stacks)
         << endl;
    File << cos(i * step * M_PI / 180.0) *
                (radius * (stacks - (j - 1)) / stacks)
         << " " << height * (j - 1) / stacks << " "
         << -sin(i * step * M_PI / 180.0) *
                (radius * (stacks - (j - 1)) / stacks)
         << endl;
    File << cos((i + 1) * step * M_PI / 180.0) *
                (radius * (stacks - (j - 1)) / stacks)
         << " " << height * (j - 1) / stacks << " "
         << -sin((i + 1) * step * M_PI / 180.0) *
                (radius * (stacks - (j - 1)) / stacks)
         << endl;
  }
}

void genSphere(float radius, int slices, int stacks) {

  ostringstream fileName;
  fileName << "../3d/sphere_" << radius << "_" << slices << "_" << stacks
           << ".3d";
  ofstream File(fileName.str(), ios::trunc);

  vector<Point3D> spherePoints;
  float sliceAngle, stackAngle;
  float sliceStep = 2 * M_PI / slices;
  float stackStep = M_PI / stacks;
  Point3D point;

  for (int i = 0; i <= stacks; i++) {
    stackAngle = M_PI / 2 - i * stackStep;
    point.y = radius * sinf(stackAngle);
    for (int j = 0; j <= slices; j++) {
      sliceAngle = j * sliceStep;
      point.x = (radius * cosf(stackAngle)) * sinf(sliceAngle);
      point.z = (radius * cosf(stackAngle)) * cosf(sliceAngle);
      spherePoints.push_back(point);
    }
  }

  Point3D p1, p2, p3, p4;
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

      if (i == 0) { // Top stack
        File << p1.x << " " << p1.y << " " << p1.z << endl;
        File << p2.x << " " << p2.y << " " << p2.z << endl;
        File << p4.x << " " << p4.y << " " << p4.z << endl;
      } else if (i == (stacks - 1)) { // Bottom Stack
        File << p1.x << " " << p1.y << " " << p1.z << endl;
        File << p2.x << " " << p2.y << " " << p2.z << endl;
        File << p3.x << " " << p3.y << " " << p3.z << endl;
      } else { // The other stacks need 2 triangles
        File << p3.x << " " << p3.y << " " << p3.z << endl;
        File << p1.x << " " << p1.y << " " << p1.z << endl;
        File << p2.x << " " << p2.y << " " << p2.z << endl;

        File << p3.x << " " << p3.y << " " << p3.z << endl;
        File << p2.x << " " << p2.y << " " << p2.z << endl;
        File << p4.x << " " << p4.y << " " << p4.z << endl;
      }
    }
  }
}

int main(int argc, char *argv[]) {

  // generateConfig();

  if (strcmp(argv[1], "plane") == 0) {
    if (stof(argv[2]) > 0 && stoi(argv[3]) > 0) {
      genPlane(stof(argv[2]), stoi(argv[3]));
    } else
      cout << "Invalid length(float) or divisions(int)." << endl;

  } else if (strcmp(argv[1], "box") == 0) {
    if (stof(argv[2]) > 0 && stoi(argv[3]) > 0) {
      genBox(stof(argv[2]), stoi(argv[3]));
    } else
      cout << "Invalid length(float) or divisions(int)." << endl;

  } else if (strcmp(argv[1], "cone") == 0) {
    genCone(stof(argv[2]), stoi(argv[3]), stoi(argv[4]), stoi(argv[5]));

  } else if (strcmp(argv[1], "sphere") == 0) {
    genSphere(stof(argv[2]), stoi(argv[3]), stoi(argv[4]));

    return 1;
  }
}
