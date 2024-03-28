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

struct XMLInfo {
  int winWidth;
  int winHeight;
  Point3D camPos;
  Point3D lookAt;
  Point3D up;
  GLdouble fov, near, far;
  GLfloat alfa, beta, radius;
};

struct Group {
  Point3D translate = {0.0f, 0.0f, 0.0f};
  Point3D rotate = {0.0f, 0.0f, 0.0f};
  Point3D scale = {0.0f, 0.0f, 0.0f};
  GLfloat angle = 0.0f;
  std::vector<std::string> models;
  std::vector<Point3D> points;
  std::vector<struct Group> children;
};

struct XMLInfo *xmlInfo = new XMLInfo();
struct Group *groupTree = NULL;

void initCamera() {
  xmlInfo->radius = 5.0f;
  xmlInfo->beta = 0; // asin(xmlInfo.camPos.y / xmlInfo.radius);
  xmlInfo->alfa =
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

struct Group *parseGroups(XMLElement *pGroupElem) {
  XMLElement *pElem = NULL;
  struct Group *pGroup = new Group();
  // Check if <transform> exists, if so use for loop to retrieve first
  // transform and iterate over its siblings
  if ((pElem = pGroupElem->FirstChildElement("transform")) != NULL) {
    for (pElem = pElem->FirstChildElement(); pElem != NULL;
         pElem = pElem->NextSiblingElement()) {
      Point3D newPoint;
      pElem->QueryFloatAttribute("x", &newPoint.x);
      pElem->QueryFloatAttribute("y", &newPoint.y);
      pElem->QueryFloatAttribute("z", &newPoint.z);
      const char *transformType = pElem->Name();
      if (std::strcmp(transformType, "translate") == 0) {
        pGroup->translate = newPoint;
      } else if (std::strcmp(transformType, "rotate") == 0) {
        pElem->QueryFloatAttribute("angle", &pGroup->angle);
        pGroup->rotate = newPoint;
      } else if (std::strcmp(transformType, "scale") == 0) {
        pGroup->scale = newPoint;
      }
    }
  }

  // Check if <models> exists, if so use for loop to retrieve first model
  // and iterate over its siblings
  if ((pElem = pGroupElem->FirstChildElement("models")) != NULL) {
    for (pElem = pElem->FirstChildElement("model"); pElem != NULL;
         pElem = pElem->NextSiblingElement("model")) {
      std::string modelName = pElem->FindAttribute("file")->Value();
      pGroup->models.emplace_back(modelName);
    }
  }

  // Create a child for every group child of current group and emplace it in the
  // current group children vector
  struct Group *pChild = NULL;
  for (pGroupElem = pGroupElem->FirstChildElement("group"); pGroupElem != NULL;
       pGroupElem = pGroupElem->NextSiblingElement("group")) {
    pChild = parseGroups(pGroupElem);
    pGroup->children.emplace_back(*pChild);
  }

  return pGroup;
}

int readConfig(const char *config) {
  XMLDocument xmlDoc;
  if (xmlDoc.LoadFile(config) != 0)
    return XML_ERROR_FILE_COULD_NOT_BE_OPENED;

  XMLNode *pRoot = xmlDoc.FirstChild();
  if (pRoot == nullptr)
    return XML_ERROR_FILE_READ_ERROR;

  XMLElement *pElem = pRoot->FirstChildElement("window");
  pElem->QueryIntAttribute("width", &xmlInfo->winWidth);
  pElem->QueryIntAttribute("height", &xmlInfo->winHeight);

  pElem = pRoot->FirstChildElement("camera");
  XMLElement *pListElem = pElem->FirstChildElement("position");
  pListElem->QueryFloatAttribute("x", &xmlInfo->camPos.x);
  pListElem->QueryFloatAttribute("y", &xmlInfo->camPos.y);
  pListElem->QueryFloatAttribute("z", &xmlInfo->camPos.z);

  pListElem = pListElem->NextSiblingElement("lookAt");
  pListElem->QueryFloatAttribute("x", &xmlInfo->lookAt.x);
  pListElem->QueryFloatAttribute("y", &xmlInfo->lookAt.y);
  pListElem->QueryFloatAttribute("z", &xmlInfo->lookAt.z);

  pListElem = pListElem->NextSiblingElement("up");
  pListElem->QueryFloatAttribute("x", &xmlInfo->up.x);
  pListElem->QueryFloatAttribute("y", &xmlInfo->up.y);
  pListElem->QueryFloatAttribute("z", &xmlInfo->up.z);

  pListElem = pListElem->NextSiblingElement("projection");
  pListElem->QueryDoubleAttribute("fov", &xmlInfo->fov);
  pListElem->QueryDoubleAttribute("near", &xmlInfo->near);
  pListElem->QueryDoubleAttribute("far", &xmlInfo->far);

  XMLElement *pGroup = pRoot->FirstChildElement("group");
  if (pGroup != NULL) {
    groupTree = parseGroups(pGroup);
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
    // FIXME:
    // xmlInfo.points.push_back(vector3d);
  }

  File.close();
}

void spherical2Cartesian() {
  xmlInfo->camPos.x = xmlInfo->radius * cos(xmlInfo->beta) * sin(xmlInfo->alfa);
  xmlInfo->camPos.y = xmlInfo->radius * sin(xmlInfo->beta);
  xmlInfo->camPos.z = xmlInfo->radius * cos(xmlInfo->beta) * cos(xmlInfo->alfa);
}

void processSpecialKeys(int key, int xx, int yy) {
  switch (key) {
  case GLUT_KEY_RIGHT:
    xmlInfo->alfa -= 0.1;
    break;
  case GLUT_KEY_LEFT:
    xmlInfo->alfa += 0.1;
    break;
  case GLUT_KEY_UP:
    xmlInfo->beta += 0.1f;
    if (xmlInfo->beta > 1.5f)
      xmlInfo->beta = 1.5f;
    break;
  case GLUT_KEY_DOWN:
    xmlInfo->beta -= 0.1f;
    if (xmlInfo->beta < -1.5f)
      xmlInfo->beta = -1.5f;
    break;
  case GLUT_KEY_PAGE_DOWN:
    xmlInfo->radius -= 0.1f;
    if (xmlInfo->radius < 0.1f)
      xmlInfo->radius = 0.1f;
    break;
  case GLUT_KEY_PAGE_UP:
    xmlInfo->radius += 0.1f;
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
  gluPerspective(xmlInfo->fov, xmlInfo->winWidth * 1.0 / xmlInfo->winHeight,
                 xmlInfo->near, xmlInfo->far);

  // Return to the model view matrix mode
  glMatrixMode(GL_MODELVIEW);
}

void renderScene(void) {
  // Clear buffers
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Set the camera
  glLoadIdentity();
  gluLookAt(xmlInfo->camPos.x, xmlInfo->camPos.y, xmlInfo->camPos.z,
            xmlInfo->lookAt.x, xmlInfo->lookAt.y, xmlInfo->lookAt.z,
            xmlInfo->up.x, xmlInfo->up.y, xmlInfo->up.z);

  // Draw
  drawAxes();

  glColor3f(1.0f, 1.0f, 1.0f);

  glBegin(GL_TRIANGLES);
  // for (point3d point : xmlinfo.points) {
  //   glvertex3f(point.x, point.y, point.z);
  // }
  glEnd();

  // End of frame
  glutSwapBuffers();
}

int main(int argc, char **argv) {

  readConfig(argv[1]);
  initCamera();
  // for (string modelName : xmlInfo.models) {
  //   readModel(modelName);
  // }

  // Init GLUT and the window
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
  glutInitWindowPosition(100, 100);
  glutInitWindowSize(xmlInfo->winWidth, xmlInfo->winHeight);
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
