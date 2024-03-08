#include "tinyxml2.h"
#include <GL/glut.h>
#include <cmath>
#include <fstream>
#include <iostream>
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

struct State {
  int winWidth;
  int winHeight;

  Point3D camPos;
  Point3D lookAt;
  Point3D up;
  GLdouble fov, near, far;

  GLfloat alfa, beta, radius;

  vector<Point3D> points;
  vector<string> models;
} state;

void initState() {
  state.alfa = 0.0f;
  state.beta = 0.0f;
  state.radius = 5.0f;
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
  pElem->QueryIntAttribute("width", &state.winWidth);
  pElem->QueryIntAttribute("height", &state.winHeight);

  pElem = pRoot->FirstChildElement("camera");
  XMLElement *pListElem = pElem->FirstChildElement("position");
  pListElem->QueryFloatAttribute("x", &state.camPos.x);
  pListElem->QueryFloatAttribute("y", &state.camPos.y);
  pListElem->QueryFloatAttribute("z", &state.camPos.z);

  pListElem = pListElem->NextSiblingElement("lookAt");
  pListElem->QueryFloatAttribute("x", &state.lookAt.x);
  pListElem->QueryFloatAttribute("y", &state.lookAt.y);
  pListElem->QueryFloatAttribute("z", &state.lookAt.z);

  pListElem = pListElem->NextSiblingElement("up");
  pListElem->QueryFloatAttribute("x", &state.up.x);
  pListElem->QueryFloatAttribute("y", &state.up.y);
  pListElem->QueryFloatAttribute("z", &state.up.z);

  pListElem = pListElem->NextSiblingElement("projection");
  pListElem->QueryDoubleAttribute("fov", &state.fov);
  pListElem->QueryDoubleAttribute("near", &state.near);
  pListElem->QueryDoubleAttribute("far", &state.far);

  pElem = pRoot->FirstChildElement("group");
  pElem = pElem->FirstChildElement("models");
  pListElem = pElem->FirstChildElement("model");
  while (pListElem != NULL) {
    string modelName = pListElem->FindAttribute("file")->Value();
    state.models.push_back(modelName);
    pListElem = pListElem->NextSiblingElement("model");
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
    state.points.push_back(vector3d);
  }

  File.close();
}

void spherical2Cartesian() {
  state.camPos.x = state.radius * cos(state.beta) * sin(state.alfa);
  state.camPos.y = state.radius * sin(state.beta);
  state.camPos.z = state.radius * cos(state.beta) * cos(state.alfa);
}

void processSpecialKeys(int key, int xx, int yy) {
  switch (key) {
  case GLUT_KEY_RIGHT:
    state.alfa -= 0.1;
    break;
  case GLUT_KEY_LEFT:
    state.alfa += 0.1;
    break;
  case GLUT_KEY_UP:
    state.beta += 0.1f;
    if (state.beta > 1.5f)
      state.beta = 1.5f;
    break;
  case GLUT_KEY_DOWN:
    state.beta -= 0.1f;
    if (state.beta < -1.5f)
      state.beta = -1.5f;
    break;
  case GLUT_KEY_PAGE_DOWN:
    state.radius -= 0.1f;
    if (state.radius < 0.1f)
      state.radius = 0.1f;
    break;
  case GLUT_KEY_PAGE_UP:
    state.radius += 0.1f;
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
  gluPerspective(state.fov, state.winWidth * 1.0 / state.winHeight, state.near,
                 state.far);

  // Return to the model view matrix mode
  glMatrixMode(GL_MODELVIEW);
}

void renderScene(void) {
  // Clear buffers
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Set the camera
  glLoadIdentity();
  gluLookAt(state.camPos.x, state.camPos.y, state.camPos.z, state.lookAt.x,
            state.lookAt.y, state.lookAt.z, state.up.x, state.up.y, state.up.z);

  // Draw
  drawAxes();

  glColor3f(1.0f, 1.0f, 1.0f);

  glBegin(GL_TRIANGLES);
  for (Point3D point : state.points) {
    glVertex3f(point.x, point.y, point.z);
  }
  glEnd();

  // End of frame
  glutSwapBuffers();
}

int main(int argc, char **argv) {

  initState();
  readConfig(argv[1]);
  for (string modelName : state.models) {
    readModel(modelName);
  }

  // Init GLUT and the window
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
  glutInitWindowPosition(100, 100);
  glutInitWindowSize(state.winWidth, state.winHeight);
  glutCreateWindow("CG@DI-UM");

  // Required callback registry
  glutDisplayFunc(renderScene);
  glutReshapeFunc(changeSize);

  // glutSpecialFunc(processSpecialKeys);

  //  OpenGL settings
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  // Enter GLUT's main cycle
  glutMainLoop();

  return 1;
}
