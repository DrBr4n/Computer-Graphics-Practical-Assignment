#include "engine.h"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#define _USE_MATH_DEFINES
#include <math.h>

#include <GL/glew.h>
#include <GL/glut.h>

#include "tinyxml2.h"

using namespace tinyxml2;

struct Config *gpConfigData = new Config();
struct Group *gpGroupRoot = NULL;
std::vector<std::string> gModels;
// NOTE: try to get gBuffers to be *gBuffers
GLuint gBuffers[20];
std::vector<struct VBOsInfo> gVBOsInfo;

float gPrevY[3] = {0, -1, 0};

int main(int argc, char **argv) {
  parseConfig(argv[1]);

  // Init GLUT and the window
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
  glutInitWindowPosition(100, 100);
  glutInitWindowSize(gpConfigData->winWidth, gpConfigData->winHeight);
  glutCreateWindow(argv[1]);

  // Required callback registry
  glutDisplayFunc(renderScene);
  glutIdleFunc(renderScene);
  glutReshapeFunc(changeSize);
  glutSpecialFunc(processSpecialKeys);

  glewInit();

  // OpenGL settings
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glEnableClientState(GL_VERTEX_ARRAY);

  getGroupModels(gpGroupRoot);
  genVBOs();

  // radius is not correct to camera position
  gpConfigData->radius = 5.0f;
  // asin(xmlInfo.camPos.y / xmlInfo.radius);
  gpConfigData->beta = 0;
  // asin(xmlInfo.camPos.x / (xmlInfo.radius * cos(xmlInfo.beta)));
  gpConfigData->alfa = 0;

  // Enter GLUT's main cycle
  glutMainLoop();

  return 0;
}

void changeSize(int w, int h) {
  // Prevent a divide by zero, when window is too short
  // (you cant make a window with zero width).
  if (h == 0)
    h = 1;

  // Compute window's aspect ratio
  float ratio = w * 1.0 / h;

  // Set the projection matrix as current
  glMatrixMode(GL_PROJECTION);
  // Load Identity Matrix
  glLoadIdentity();

  // Set the viewport to be the entire window
  glViewport(0, 0, w, h);

  // Set perspective
  gluPerspective(gpConfigData->fov,
                 gpConfigData->winWidth * 1.0 / gpConfigData->winHeight,
                 gpConfigData->near, gpConfigData->far);

  // Return to the model view matrix mode
  glMatrixMode(GL_MODELVIEW);
}

void renderScene(void) {
  // Clear buffers
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Set the camera
  glLoadIdentity();
  gluLookAt(gpConfigData->camPos.x, gpConfigData->camPos.y,
            gpConfigData->camPos.z, gpConfigData->lookAt.x,
            gpConfigData->lookAt.y, gpConfigData->lookAt.z, gpConfigData->up.x,
            gpConfigData->up.y, gpConfigData->up.z);

  // Draw
  drawAxes();

  drawGroup(gpGroupRoot);

  // End of frame
  glutSwapBuffers();
}

void processSpecialKeys(int key, int xx, int yy) {
  switch (key) {
  case GLUT_KEY_RIGHT:
    gpConfigData->alfa -= 0.1;
    break;
  case GLUT_KEY_LEFT:
    gpConfigData->alfa += 0.1;
    break;
  case GLUT_KEY_UP:
    gpConfigData->beta += 0.1f;
    if (gpConfigData->beta > 1.5f)
      gpConfigData->beta = 1.5f;
    break;
  case GLUT_KEY_DOWN:
    gpConfigData->beta -= 0.1f;
    if (gpConfigData->beta < -1.5f)
      gpConfigData->beta = -1.5f;
    break;
  case GLUT_KEY_PAGE_DOWN:
    gpConfigData->radius -= 0.1f;
    if (gpConfigData->radius < 0.1f)
      gpConfigData->radius = 0.1f;
    break;
  case GLUT_KEY_PAGE_UP:
    gpConfigData->radius += 0.1f;
    break;
  }
  spherical2Cartesian();
  glutPostRedisplay();
}

void spherical2Cartesian() {
  gpConfigData->camPos.y = gpConfigData->radius * sin(gpConfigData->beta);
  gpConfigData->camPos.x =
      gpConfigData->radius * cos(gpConfigData->beta) * sin(gpConfigData->alfa);
  gpConfigData->camPos.z =
      gpConfigData->radius * cos(gpConfigData->beta) * cos(gpConfigData->alfa);
}

int parseConfig(const char *config) {
  XMLDocument xmlDoc;
  if (xmlDoc.LoadFile(config) != 0)
    return XML_ERROR_FILE_COULD_NOT_BE_OPENED;

  XMLNode *pRoot = xmlDoc.FirstChild();
  if (pRoot == nullptr)
    return XML_ERROR_FILE_READ_ERROR;

  XMLElement *pElem = pRoot->FirstChildElement("window");
  pElem->QueryIntAttribute("width", &gpConfigData->winWidth);
  pElem->QueryIntAttribute("height", &gpConfigData->winHeight);

  pElem = pRoot->FirstChildElement("camera");
  XMLElement *pListElem = pElem->FirstChildElement("position");
  pListElem->QueryFloatAttribute("x", &gpConfigData->camPos.x);
  pListElem->QueryFloatAttribute("y", &gpConfigData->camPos.y);
  pListElem->QueryFloatAttribute("z", &gpConfigData->camPos.z);

  pListElem = pListElem->NextSiblingElement("lookAt");
  pListElem->QueryFloatAttribute("x", &gpConfigData->lookAt.x);
  pListElem->QueryFloatAttribute("y", &gpConfigData->lookAt.y);
  pListElem->QueryFloatAttribute("z", &gpConfigData->lookAt.z);

  pListElem = pListElem->NextSiblingElement("up");
  pListElem->QueryFloatAttribute("x", &gpConfigData->up.x);
  pListElem->QueryFloatAttribute("y", &gpConfigData->up.y);
  pListElem->QueryFloatAttribute("z", &gpConfigData->up.z);

  pListElem = pListElem->NextSiblingElement("projection");
  pListElem->QueryDoubleAttribute("fov", &gpConfigData->fov);
  pListElem->QueryDoubleAttribute("near", &gpConfigData->near);
  pListElem->QueryDoubleAttribute("far", &gpConfigData->far);

  XMLElement *pGroup = pRoot->FirstChildElement("group");
  if (pGroup != NULL) {
    gpGroupRoot = parseGroup(pGroup);
  }

  return 0;
}

struct Group *parseGroup(XMLElement *pGroupElem) {
  XMLElement *pElem = NULL;
  struct Group *pGroup = new Group();
  // Check if <transform> exists, if so use for loop to retrieve first
  // transform and iterate over its siblings
  if ((pElem = pGroupElem->FirstChildElement("transform")) != NULL) {
    for (pElem = pElem->FirstChildElement(); pElem != NULL;
         pElem = pElem->NextSiblingElement()) {

      const char *transformType = pElem->Name();
      if ((std::strcmp(transformType, "translate") == 0) &&
          pElem->FindAttribute("time") != 0) {
        pGroup->orderOfTransformations.emplace_back(1);
        pElem->QueryIntAttribute("time", &pGroup->translateTime);
        pElem->QueryBoolAttribute("align", &pGroup->align);
        for (XMLElement *pPointList = pElem->FirstChildElement();
             pPointList != NULL;
             pPointList = pPointList->NextSiblingElement()) {
          Vector3D newVector;
          pPointList->QueryFloatAttribute("x", &newVector.x);
          pPointList->QueryFloatAttribute("y", &newVector.y);
          pPointList->QueryFloatAttribute("z", &newVector.z);
          pGroup->curvePoints.push_back(newVector);
        }
      } else {
        Vector3D newVector;
        pElem->QueryFloatAttribute("x", &newVector.x);
        pElem->QueryFloatAttribute("y", &newVector.y);
        pElem->QueryFloatAttribute("z", &newVector.z);
        if (std::strcmp(transformType, "translate") == 0) {
          pGroup->orderOfTransformations.emplace_back(1);
          pGroup->translate = newVector;
        } else if (std::strcmp(transformType, "rotate") == 0) {
          pGroup->orderOfTransformations.emplace_back(2);
          if (pElem->FindAttribute("time") == 0)
            pElem->QueryFloatAttribute("angle", &pGroup->angle);
          else
            pElem->QueryIntAttribute("time", &pGroup->rotateTime);
          pGroup->rotate = newVector;
        } else if (std::strcmp(transformType, "scale") == 0) {
          pGroup->orderOfTransformations.emplace_back(3);
          pGroup->scale = newVector;
        }
      }
    }
  }

  // Check if <models> exists, if so use for loop to retrieve first model
  // and iterate over its siblings
  if ((pElem = pGroupElem->FirstChildElement("models")) != NULL) {
    for (pElem = pElem->FirstChildElement("model"); pElem != NULL;
         pElem = pElem->NextSiblingElement("model")) {
      pGroup->models.emplace_back(pElem->FindAttribute("file")->Value());
    }
  }

  // Create a child for every group child of current group and emplace it in
  // the current group children vector
  struct Group *pChild = NULL;
  for (pGroupElem = pGroupElem->FirstChildElement("group"); pGroupElem != NULL;
       pGroupElem = pGroupElem->NextSiblingElement("group")) {
    pChild = parseGroup(pGroupElem);
    pGroup->children.push_back(*pChild);
  }

  return pGroup;
}

std::vector<struct Vector3D> parseModels(std::vector<std::string> modelNames) {
  std::vector<struct Vector3D> points;
  // NOTE: Might be a bug
  Vector3D point;

  for (const auto &modelName : modelNames) {
    std::ifstream File("../3d/" + modelName);
    std::string line;

    while (getline(File, line)) {
      std::istringstream lineStream(line);
      lineStream >> point.x;
      lineStream >> point.y;
      lineStream >> point.z;
      points.push_back(point);
    }

    File.close();
  }
  return points;
}

void getGroupModels(struct Group *group) {
  for (const auto &model : group->models) {
    gModels.push_back(model);
  }
  for (struct Group child : group->children) {
    getGroupModels(&child);
  }
}

void genVBOs() {
  std::sort(gModels.begin(), gModels.end());
  gModels.erase(unique(gModels.begin(), gModels.end()), gModels.end());

  glGenBuffers(gModels.size(), gBuffers);

  int bufferIdx = 0;
  for (const auto &modelName : gModels) {

    std::vector<float> vertex;
    float x, y, z, vertexCounter = 0;

    std::ifstream File("../3d/" + modelName);
    std::string line;

    while (getline(File, line)) {
      std::istringstream lineStream(line);
      lineStream >> x;
      lineStream >> y;
      lineStream >> z;
      vertex.push_back(x);
      vertex.push_back(y);
      vertex.push_back(z);
      vertexCounter += 3;
    }
    File.close();

    glBindBuffer(GL_ARRAY_BUFFER, gBuffers[bufferIdx]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertex.size(), vertex.data(),
                 GL_STATIC_DRAW);

    struct VBOsInfo VBOsInfoNode;
    VBOsInfoNode.bufferIndex = bufferIdx;
    bufferIdx += 1;
    VBOsInfoNode.vertexCount = vertexCounter;
    VBOsInfoNode.modelName = modelName;
    gVBOsInfo.push_back(VBOsInfoNode);
  }
}

void drawAxes() {
  glBegin(GL_LINES);

  // X axis in red
  glColor3f(1.0f, 0.0f, 0.0f);
  glVertex3f(-100.0f, 0.0f, 0.0f);
  glVertex3f(100.0f, 0.0f, 0.0f);

  // Y axis in green
  glColor3f(0.0f, 1.0f, 0.0f);
  glVertex3f(0.0f, -100.0f, 0.0f);
  glVertex3f(0.0f, 100.0f, 0.0f);

  // Z axis in blue
  glColor3f(0.0f, 0.0f, 1.0f);
  glVertex3f(0.0f, 0.0f, -100.0f);
  glVertex3f(0.0f, 0.0f, 100.0f);
  glEnd();

  glColor3f(1.0f, 1.0f, 1.0f);
}

void drawGroup(struct Group *group) {
  for (const auto &order : group->orderOfTransformations) {
    switch (order) {
    case 1:
      if (group->translateTime == 0) {
        glTranslatef(group->translate.x, group->translate.y,
                     group->translate.z);
      } else {
        // walk on the curve
        float curTime = (glutGet(GLUT_ELAPSED_TIME) / ((float)1000));
        float pos[3];
        float deriv[3];
        float t = (group->curvePoints.size() + curTime) / group->translateTime;
        renderCatmullRomCurve(group->curvePoints);
        getGlobalCatmullRomPoint(t, pos, deriv, group->curvePoints);
        glTranslatef(pos[0], pos[1], pos[2]);

        // align with the curve
        if (group->align == 1) {
          float x[3], z[3];
          float y[3] = {0, 1, 0};
          x[0] = deriv[0];
          x[1] = deriv[1];
          x[2] = deriv[2];
          normalize(x);
          cross(x, gPrevY, z);
          normalize(z);
          cross(z, x, gPrevY);
          normalize(gPrevY);

          float m[4][4];
          buildRotMatrix(x, gPrevY, z, (float *)m);
          glMultMatrixf((float *)m);
        }
      }
      break;
    case 2:
      if (group->rotateTime == 0) {
        glRotatef(group->angle, group->rotate.x, group->rotate.y,
                  group->rotate.z);
      } else {
        float curTime = (glutGet(GLUT_ELAPSED_TIME)) / ((float)1000);
        glRotatef((curTime / group->rotateTime) * 360, group->rotate.x,
                  group->rotate.y, group->rotate.z);
      }
      break;
    case 3:
      glScalef(group->scale.x, group->scale.y, group->scale.z);
      break;
    }
  }

  for (const auto &modelName : group->models) {
    for (const auto &vboInfo : gVBOsInfo) {
      if (modelName == vboInfo.modelName) {
        glBindBuffer(GL_ARRAY_BUFFER, gBuffers[vboInfo.bufferIndex]);
        glVertexPointer(3, GL_FLOAT, 0, 0);
        glDrawArrays(GL_TRIANGLES, 0, vboInfo.vertexCount);
      }
    }
  }

  for (struct Group child : group->children) {
    glPushMatrix();
    drawGroup(&child);
    glPopMatrix();
  }
}

// Curves
float length(float *v) {
  float res = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
  return res;
}

void normalize(float *a) {
  float l = sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
  a[0] = a[0] / l;
  a[1] = a[1] / l;
  a[2] = a[2] / l;
}

void cross(float *a, float *b, float *res) {
  res[0] = a[1] * b[2] - a[2] * b[1];
  res[1] = a[2] * b[0] - a[0] * b[2];
  res[2] = a[0] * b[1] - a[1] * b[0];
}

void buildRotMatrix(float *x, float *y, float *z, float *m) {
  m[0] = x[0];
  m[1] = x[1];
  m[2] = x[2];
  m[3] = 0;
  m[4] = y[0];
  m[5] = y[1];
  m[6] = y[2];
  m[7] = 0;
  m[8] = z[0];
  m[9] = z[1];
  m[10] = z[2];
  m[11] = 0;
  m[12] = 0;
  m[13] = 0;
  m[14] = 0;
  m[15] = 1;
}

void multMatrixVector(float *m, float *v, float *res) {
  for (int j = 0; j < 4; ++j) {
    res[j] = 0;
    for (int k = 0; k < 4; ++k) {
      res[j] += v[k] * m[j * 4 + k];
    }
  }
}

void getCatmullRomPoint(float t, std::vector<struct Vector3D> pointsVector,
                        float *pos, float *deriv) {
  // catmull-rom matrix
  float m[4][4] = {{-0.5f, 1.5f, -1.5f, 0.5f},
                   {1.0f, -2.5f, 2.0f, -0.5f},
                   {-0.5f, 0.0f, 0.5f, 0.0f},
                   {0.0f, 1.0f, 0.0f, 0.0f}};

  float p[3][4] = {{pointsVector[0].x, pointsVector[1].x, pointsVector[2].x,
                    pointsVector[3].x},
                   {pointsVector[0].y, pointsVector[1].y, pointsVector[2].y,
                    pointsVector[3].y},
                   {pointsVector[0].z, pointsVector[1].z, pointsVector[2].z,
                    pointsVector[3].z}};
  for (int i = 0; i < 3; i++) {
    float a[4];
    // compute A = M * P
    multMatrixVector((float *)m, p[i], a);
    // compute pos = T * A
    pos[i] = pow(t, 3.0) * a[0] + pow(t, 2.0) * a[1] + t * a[2] + a[3];
    // compute deriv = T' * A
    deriv[i] = 3 * pow(t, 2.0) * a[0] + 2 * t * a[1] + a[2];
  }
}

void getGlobalCatmullRomPoint(float gt, float *pos, float *deriv,
                              std::vector<struct Vector3D> curvePoints) {
  int pointCount = curvePoints.size();
  float t = gt * pointCount; // this is the real global t
  int index = floor(t);      // which segment
  t = t - index;             // where within the segment

  // indexes store the points
  int indexes[4];
  indexes[0] = (index + pointCount - 1) % pointCount;
  indexes[1] = (indexes[0] + 1) % pointCount;
  indexes[2] = (indexes[1] + 1) % pointCount;
  indexes[3] = (indexes[2] + 1) % pointCount;

  // TODO: check this and create apropriate vector
  std::vector<struct Vector3D> pointsVector = {
      curvePoints[indexes[0]], curvePoints[indexes[1]], curvePoints[indexes[2]],
      curvePoints[indexes[3]]};

  getCatmullRomPoint(t, pointsVector, pos, deriv);
}

void renderCatmullRomCurve(std::vector<struct Vector3D> curvePoints) {
  // Draw curve using line segments with GL_LINE_LOOP
  float tesselation = 100;
  float pos[3];
  float deriv[3];

  glBegin(GL_LINE_LOOP);
  for (int gt = 0; gt < tesselation; gt++) {
    getGlobalCatmullRomPoint(gt / tesselation, pos, deriv, curvePoints);
    glVertex3f(pos[0], pos[1], pos[2]);
  }
  glEnd();
}
