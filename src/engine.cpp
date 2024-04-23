#include "engine.h"

#include <fstream>
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
GLuint *gBuffers;

int main(int argc, char **argv) {
  parseConfig(argv[1]);
  getGroupModels(gpGroupRoot);
  // genVBOs();
  // radius is not correct to camera position
  gpConfigData->radius = 5.0f;
  // asin(xmlInfo.camPos.y / xmlInfo.radius);
  gpConfigData->beta = 0;
  // asin(xmlInfo.camPos.x / (xmlInfo.radius * cos(xmlInfo.beta)));
  gpConfigData->alfa = 0;

  // Init GLUT and the window
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
  glutInitWindowPosition(100, 100);
  glutInitWindowSize(gpConfigData->winWidth, gpConfigData->winHeight);
  glutCreateWindow(argv[1]);

  // Required callback registry
  glutDisplayFunc(renderScene);
  glutReshapeFunc(changeSize);
  glutSpecialFunc(processSpecialKeys);

  // OpenGL settings
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glEnableClientState(GL_VERTEX_ARRAY);

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

  glColor3f(1.0f, 1.0f, 1.0f);

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
      Vector3D newVector;
      pElem->QueryFloatAttribute("x", &newVector.x);
      pElem->QueryFloatAttribute("y", &newVector.y);
      pElem->QueryFloatAttribute("z", &newVector.z);
      const char *transformType = pElem->Name();
      if (std::strcmp(transformType, "translate") == 0) {
        pGroup->orderOfTransformations.emplace_back(1);
        pGroup->translate = newVector;
      } else if (std::strcmp(transformType, "rotate") == 0) {
        pGroup->orderOfTransformations.emplace_back(2);
        pElem->QueryFloatAttribute("angle", &pGroup->angle);
        pGroup->rotate = newVector;
      } else if (std::strcmp(transformType, "scale") == 0) {
        pGroup->orderOfTransformations.emplace_back(3);
        pGroup->scale = newVector;
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

  // Create a child for every group child of current group and emplace it in the
  // current group children vector
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
  // TODO: clear gModels duplicates

  glGenBuffers(gModels.size(), gBuffers);
  int i = 0;
  for (const auto &model : gModels) {
    std::vector<float> vertex;
    float x, y, z;
    std::ifstream File("../3d/" + model);
    std::string line;

    while (getline(File, line)) {
      std::istringstream lineStream(line);
      lineStream >> x;
      lineStream >> y;
      lineStream >> z;
      vertex.push_back(x);
      vertex.push_back(y);
      vertex.push_back(z);
    }

    File.close();

    glBindBuffer(GL_ARRAY_BUFFER, gBuffers[i++]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertex.size(), vertex.data(),
                 GL_STATIC_DRAW);
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
}

void drawGroup(struct Group *group) {
  for (const auto &order : group->orderOfTransformations) {
    switch (order) {
    case 1:
      glTranslatef(group->translate.x, group->translate.y, group->translate.z);
      break;
    case 2:
      glRotatef(group->angle, group->rotate.x, group->rotate.y,
                group->rotate.z);
      break;
    case 3:
      glScalef(group->scale.x, group->scale.y, group->scale.z);
      break;
    }
  }

  group->points = parseModels(group->models);

  glBegin(GL_TRIANGLES);
  for (const auto &point : group->points) {
    glVertex3f(point.x, point.y, point.z);
  }
  glEnd();

  for (struct Group child : group->children) {
    glPushMatrix();
    drawGroup(&child);
    glPopMatrix();
  }
}
