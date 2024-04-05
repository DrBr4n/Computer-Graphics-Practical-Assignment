#ifndef ENGINE_H
#define ENGINE_H

#include <string>
#include <vector>

#include "tinyxml2.h"
#include <GL/glut.h>

using namespace tinyxml2;

struct Vector3D {
  GLfloat x;
  GLfloat y;
  GLfloat z;
};

struct Config {
  int winWidth;
  int winHeight;
  Vector3D camPos;
  Vector3D lookAt;
  Vector3D up;
  GLdouble fov, near, far;
  GLfloat alfa, beta, radius;
};

struct Group {
  std::vector<struct Group> children;
  Vector3D translate;
  Vector3D scale;
  Vector3D rotate;
  GLfloat angle;
  std::vector<int> orderOfTransformations;
  std::vector<std::string> models;
  std::vector<struct Vector3D> points;
};

void changeSize(int w, int h);

void renderScene(void);

void processSpecialKeys(int key, int xx, int yy);

void spherical2Cartesian();

int parseConfig(const char *config);

struct Group *parseGroup(XMLElement *pGroupElem);

std::vector<struct Vector3D> parseModels(std::vector<std::string> modelNames);

void drawAxes();

void drawGroup(struct Group *group);

#endif
