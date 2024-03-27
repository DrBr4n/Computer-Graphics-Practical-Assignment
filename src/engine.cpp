#include "tinyxml2.h"
#include <GL/glut.h>
#include <cmath>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

using namespace tinyxml2;
using namespace std;

struct Point3D {
  GLfloat x;
  GLfloat y;
  GLfloat z;
};

// FIXME: wrong data structure must be a tree or some other data structure
// that accepts same level objects
struct Group {
  Point3D translate = {0.0f, 0.0f, 0.0f};
  float angle = 0.0f;
  Point3D rotate = {0.0f, 0.0f, 0.0f};
  Point3D scale = {0.0f, 0.0f, 0.0f};
  std::vector<std::string> models;
  std::vector<Point3D> points;
  struct Group *next;
};

struct XMLInfo {
  int winWidth;
  int winHeight;
  Point3D camPos;
  Point3D lookAt;
  Point3D up;
  double fov, near, far;
  float alfa, beta, radius;
} xmlInfo;

void initCamera() {
  xmlInfo.radius = 5.0f;
  xmlInfo.beta = 0; // asin(xmlInfo.camPos.y / xmlInfo.radius);
  xmlInfo.alfa =
      0; // asin(xmlInfo.camPos.x / (xmlInfo.radius * cos(xmlInfo.beta)));
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

int readConfig(const char *config) {
  XMLDocument xmlDoc;
  if (xmlDoc.LoadFile(config) != 0)
    return XML_ERROR_FILE_COULD_NOT_BE_OPENED;

  XMLNode *pRoot = xmlDoc.FirstChild();
  if (pRoot == nullptr)
    return XML_ERROR_FILE_READ_ERROR;

  XMLElement *pElem = pRoot->FirstChildElement("window");
  pElem->QueryIntAttribute("width", &xmlInfo.winWidth);
  pElem->QueryIntAttribute("height", &xmlInfo.winHeight);

  // Get first child element of camera so we can iterate by his siblings after
  pElem = pRoot->FirstChildElement("camera")->FirstChildElement("position");
  pElem->QueryFloatAttribute("x", &xmlInfo.camPos.x);
  pElem->QueryFloatAttribute("y", &xmlInfo.camPos.y);
  pElem->QueryFloatAttribute("z", &xmlInfo.camPos.z);

  pElem = pElem->NextSiblingElement("lookAt");
  pElem->QueryFloatAttribute("x", &xmlInfo.lookAt.x);
  pElem->QueryFloatAttribute("y", &xmlInfo.lookAt.y);
  pElem->QueryFloatAttribute("z", &xmlInfo.lookAt.z);

  pElem = pElem->NextSiblingElement("up");
  pElem->QueryFloatAttribute("x", &xmlInfo.up.x);
  pElem->QueryFloatAttribute("y", &xmlInfo.up.y);
  pElem->QueryFloatAttribute("z", &xmlInfo.up.z);

  pElem = pElem->NextSiblingElement("projection");
  pElem->QueryDoubleAttribute("fov", &xmlInfo.fov);
  pElem->QueryDoubleAttribute("near", &xmlInfo.near);
  pElem->QueryDoubleAttribute("far", &xmlInfo.far);

  XMLElement *pGroup = pRoot->FirstChildElement("group");
  while (pGroup != NULL) {
    Group *group = new Group();
    // Get first child element of transform so we can iterate by his siblings
    // after
    pElem = pGroup->FirstChildElement("transform")->FirstChildElement();
    while (pElem != NULL) {
      Point3D newPoint;
      float angle;
      pElem->QueryFloatAttribute("x", &newPoint.x);
      pElem->QueryFloatAttribute("y", &newPoint.y);
      pElem->QueryFloatAttribute("z", &newPoint.z);
      pElem->QueryFloatAttribute("angle", &angle);
      const char *transformType = pElem->Name();
      if (std::strcmp(transformType, "translate") == 0) {
        group->translate = newPoint;
      } else if (std::strcmp(transformType, "rotate") == 0) {
        group->angle = angle;
        group->rotate = newPoint;
      } else if (std::strcmp(transformType, "scale") == 0) {
        group->scale = newPoint;
      }
      pElem = pElem->NextSiblingElement();
    }

    // Get first child element of models so we can iterate by his siblings after
    pElem = pGroup->FirstChildElement("models")->FirstChildElement("model");
    while (pElem != NULL) {
      std::string modelName = pElem->FindAttribute("file")->Value();
      group->models.emplace_back(modelName);
      pElem = pElem->NextSiblingElement("model");
    }
    // TODO: Add current group to the last level of a tree
    // TODO: Figure out how to traverse, NextSibling stays on the same level,
    // FirstChildElement dips a level
    pGroup = pGroup->FirstChildElement("group");
  }
  return 0;
}

void readModel(string fileName) {
  ifstream File("../3d/" + fileName);
  string line;

  while (getline(File, line)) {
    istringstream lineStream(line);
    Point3D vector3d;
    lineStream >> vector3d.x;
    lineStream >> vector3d.y;
    lineStream >> vector3d.z;
    // FIXME:: local group points?
    // xmlInfo.points.push_back(vector3d);
  }

  File.close();
}

void spherical2Cartesian() {
  xmlInfo.camPos.x = xmlInfo.radius * cos(xmlInfo.beta) * sin(xmlInfo.alfa);
  xmlInfo.camPos.y = xmlInfo.radius * sin(xmlInfo.beta);
  xmlInfo.camPos.z = xmlInfo.radius * cos(xmlInfo.beta) * cos(xmlInfo.alfa);
}

void processSpecialKeys(int key, int xx, int yy) {
  switch (key) {
  case GLUT_KEY_RIGHT:
    xmlInfo.alfa -= 0.1;
    break;
  case GLUT_KEY_LEFT:
    xmlInfo.alfa += 0.1;
    break;
  case GLUT_KEY_UP:
    xmlInfo.beta += 0.1f;
    if (xmlInfo.beta > 1.5f)
      xmlInfo.beta = 1.5f;
    break;
  case GLUT_KEY_DOWN:
    xmlInfo.beta -= 0.1f;
    if (xmlInfo.beta < -1.5f)
      xmlInfo.beta = -1.5f;
    break;
  case GLUT_KEY_PAGE_DOWN:
    xmlInfo.radius -= 0.1f;
    if (xmlInfo.radius < 0.1f)
      xmlInfo.radius = 0.1f;
    break;
  case GLUT_KEY_PAGE_UP:
    xmlInfo.radius += 0.1f;
    break;
  }
  spherical2Cartesian();
  glutPostRedisplay();
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
  gluPerspective(xmlInfo.fov, xmlInfo.winWidth * 1.0 / xmlInfo.winHeight,
                 xmlInfo.near, xmlInfo.far);

  // Return to the model view matrix mode
  glMatrixMode(GL_MODELVIEW);
}

void renderScene(void) {
  // Clear buffers
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Set the camera
  glLoadIdentity();
  gluLookAt(xmlInfo.camPos.x, xmlInfo.camPos.y, xmlInfo.camPos.z,
            xmlInfo.lookAt.x, xmlInfo.lookAt.y, xmlInfo.lookAt.z, xmlInfo.up.x,
            xmlInfo.up.y, xmlInfo.up.z);

  // Draw
  drawAxes();

  glColor3f(1.0f, 1.0f, 1.0f);

  glBegin(GL_TRIANGLES);
  // FIXME: for group draw local group points?
  // for (Point3D point : xmlInfo.points) {
  //   glVertex3f(point.x, point.y, point.z);
  // }
  glEnd();

  // End of frame
  glutSwapBuffers();
}

int main(int argc, char **argv) {

  readConfig(argv[1]);
  initCamera();
  // FIXME: read models inside renderScene function when iterating over groups?
  // for (string modelName : xmlInfo.models) {
  //   readModel(modelName);
  // }

  // Init GLUT and the window
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
  glutInitWindowPosition(100, 100);
  glutInitWindowSize(xmlInfo.winWidth, xmlInfo.winHeight);
  glutCreateWindow(argv[1]);

  // Required callback registry
  glutDisplayFunc(renderScene);
  glutReshapeFunc(changeSize);
  glutSpecialFunc(processSpecialKeys);

  // OpenGL settings
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  // Enter GLUT's main cycle
  glutMainLoop();

  return 0;
}
