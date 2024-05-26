#ifndef ENGINE_H
#define ENGINE_H

#include <string>
#include <vector>

#include <GL/glew.h>
#include <GL/glut.h>

#include "tinyxml2.h"

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
  std::vector<int> modelIds;
  int translateTime;
  int rotateTime;
  bool align;
  std::vector<struct Vector3D> curvePoints;
};

struct VBOsInfo {
  int vertexBufferIndex;
  int normalBufferIndex;
  int texBufferIndex;
  int vertexCount;
  std::string modelName;
};

struct Light {
  std::string type;
  Vector3D pos;
  Vector3D dir;
  int cutoff;
};

struct Model {
  int id;
  std::string name;
  std::string texture;
  unsigned int textureId;
  // [diffuse, ambient, specular, emissive]
  GLfloat lightComp[4][3];
  GLfloat shininess;
};

void changeSize(int w, int h);

void renderScene(void);

void processSpecialKeys(int key, int xx, int yy);

void spherical2Cartesian();

int parseConfig(const char *config);

struct Group *parseGroup(XMLElement *pGroupElem);

void getGroupModels(struct Group *group);

std::vector<struct Vector3D> parseModels(std::vector<std::string> modelNames);

void genVBOs();

void drawAxes();

void drawGroup(struct Group *group);

float length(float *v);

void normalize(float *a);

void cross(float *a, float *b, float *res);

void buildRotMatrix(float *x, float *y, float *z, float *m);

void multMatrixVector(float *m, float *v, float *res);

void getCatmullRomPoint(float t, std::vector<struct Vector3D> pointsVector,
                        float *pos, float *deriv);

void getGlobalCatmullRomPoint(float gt, float *pos, float *deriv,
                              std::vector<struct Vector3D> curvePoints);

void renderCatmullRomCurve(std::vector<struct Vector3D> curvePoints);

void initLights();

void setupLights();

void initAndLoadTextures();

int loadTexture(std::string file);

#endif
