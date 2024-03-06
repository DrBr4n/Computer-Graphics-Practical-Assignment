#include "tinyxml2.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <algorithm>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace tinyxml2;
using namespace std;

struct Vector {
  GLfloat x;
  GLfloat y;
  GLfloat z;
};

struct State {
  Vector tVector;
  GLfloat alpha;
  GLfloat beta;
  GLfloat gamma;
  GLfloat scale;
  vector<Vector> vectors;
} state;

struct Camera {
  int width;
  int height;
  Vector position;
  Vector lookAt;
  Vector up;
  GLdouble fov;
  GLdouble near;
  GLdouble far;
} camera;

void initState() {
  state.tVector.x = state.tVector.y = state.tVector.z = 0.0f;
  state.alpha = state.beta = state.gamma = 0.0f;
  state.scale = 1.0f;
}

int readConfig(const char *config) {

  XMLDocument xmlDoc;
  if (xmlDoc.LoadFile(config) != 0)
    return XML_ERROR_FILE_COULD_NOT_BE_OPENED;

  XMLNode *pRoot = xmlDoc.FirstChild();
  if (pRoot == nullptr)
    return XML_ERROR_FILE_READ_ERROR;

  XMLElement *pElem = pRoot->FirstChildElement("window");
  pElem->QueryIntAttribute("width", &camera.width);
  pElem->QueryIntAttribute("height", &camera.height);

  pElem = pRoot->FirstChildElement("camera");
  XMLElement *pListElem = pElem->FirstChildElement("position");
  pListElem->QueryFloatAttribute("x", &camera.position.x);
  pListElem->QueryFloatAttribute("y", &camera.position.y);
  pListElem->QueryFloatAttribute("z", &camera.position.z);

  pListElem = pListElem->NextSiblingElement("lookAt");
  pListElem->QueryFloatAttribute("x", &camera.lookAt.x);
  pListElem->QueryFloatAttribute("y", &camera.lookAt.y);
  pListElem->QueryFloatAttribute("z", &camera.lookAt.z);

  pListElem = pListElem->NextSiblingElement("up");
  pListElem->QueryFloatAttribute("x", &camera.up.x);
  pListElem->QueryFloatAttribute("y", &camera.up.y);
  pListElem->QueryFloatAttribute("z", &camera.up.z);

  pListElem = pListElem->NextSiblingElement("projection");
  pListElem->QueryDoubleAttribute("fov", &camera.fov);
  pListElem->QueryDoubleAttribute("near", &camera.near);
  pListElem->QueryDoubleAttribute("far", &camera.far);

  // TODO: read group section

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
  gluPerspective(45.0f, ratio, 1.0f, 1000.0f);

  // Return to the model view matrix mode
  glMatrixMode(GL_MODELVIEW);
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

// Plane (a square in the XZ plane, centred in the origin, subdivided in both X
// and Z directions)
void drawPlane(float length, int divisions) {

  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glColor3f(1.0f, 1.0f, 1.0f);

  glBegin(GL_TRIANGLES);

  int fixDiv = 0;
  float ofset = 0;
  if (divisions % 2 == 1) {
    fixDiv = 1;
    ofset = length / 2;
  }
  for (int i = -divisions / 2 - fixDiv; i < divisions / 2; i++) {
    for (int j = -divisions / 2 - fixDiv; j < divisions / 2; j++) {
      glVertex3f(length * j + ofset, 0.0f, length * i + ofset);
      glVertex3f(length * j + ofset, 0.0f, length * (i + 1) + ofset);
      glVertex3f(length * (j + 1) + ofset, 0.0f, length * i + ofset);
      glVertex3f(length * j + ofset, 0.0f, length * (i + 1) + ofset);
      glVertex3f(length * (j + 1) + ofset, 0.0f, length * (i + 1) + ofset);
      glVertex3f(length * (j + 1) + ofset, 0.0f, length * i + ofset);
    }
  }
  glEnd();
}

void readModel(char *fileName) {

  ifstream File(fileName);
  string line;

  while (getline(File, line)) {
    istringstream lineStream(line);
    Vector vector3d;
    lineStream >> vector3d.x;
    lineStream >> vector3d.y;
    lineStream >> vector3d.z;
    state.vectors.push_back(vector3d);
  }

  File.close();
}

void renderScene(void) {
  // Clear buffers
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Set the camera
  glLoadIdentity();
  gluLookAt(camera.position.x, camera.position.y, camera.position.z,
            camera.lookAt.x, camera.lookAt.y, camera.lookAt.z, camera.up.x,
            camera.up.y, camera.up.z);
  // gluPerspective(camera.fov, camera.width * 1.0 / camera.height, camera.near,
  // camera.far);

  drawAxes();

  // Geometric Transformations
  glTranslatef(state.tVector.x, state.tVector.y, state.tVector.z);
  glRotatef(state.alpha, 1.0f, 0.0f, 0.0f);
  glRotatef(state.beta, 0.0f, 1.0f, 0.0f);
  glRotatef(state.gamma, 0.0f, 0.0f, 1.0f);
  glScalef(state.scale, state.scale, state.scale);

  glColor3f(1.0f, 1.0f, 1.0f);
  // Drawings
  // drawPlane(2, 3);
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glBegin(GL_TRIANGLES);
  for (Vector vectorToDraw : state.vectors) {
    glVertex3f(vectorToDraw.x, vectorToDraw.y, vectorToDraw.z);
  }
  glEnd();

  // End of frame
  glutSwapBuffers();
}

// Simple translations, rotations and scale
void processKeys(unsigned char key, int xx, int yy) {
  switch (key) {
  case 'a':
    state.tVector.x += 0.5f;
    break;
  case 'A':
    state.tVector.x -= 0.5f;
    break;
  case 's':
    state.tVector.y += 0.5f;
    break;
  case 'S':
    state.tVector.y -= 0.5f;
    break;
  case 'd':
    state.tVector.z += 0.5f;
    break;
  case 'D':
    state.tVector.z -= 0.5f;
    break;
  case 'j':
    state.alpha += 5.0f;
    break;
  case 'J':
    state.alpha -= 5.0f;
    break;
  case 'k':
    state.beta += 5.0f;
    break;
  case 'K':
    state.beta -= 5.0f;
    break;
  case 'l':
    state.gamma += 5.0f;
    break;
  case 'L':
    state.gamma -= 5.0f;
    break;
  case 'i':
    state.scale += 0.1f;
    break;
  case 'I':
    state.scale -= 0.1f;
    break;
  case 'r':
    state.alpha = state.beta = state.gamma = 0.0f;
    state.tVector.x = state.tVector.y = state.tVector.z = 0.0f;
    state.scale = 1.0f;
    break;
  }

  glutPostRedisplay();
}

int main(int argc, char **argv) {

  initState();
  readConfig("../tests/test_files_phase_1/test_1_5.xml");
  readModel(argv[1]);

  // Init GLUT and the window
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
  glutInitWindowPosition(100, 100);
  glutInitWindowSize(camera.width, camera.height);
  glutCreateWindow("CG@DI-UM");

  // Required callback registry
  glutDisplayFunc(renderScene);
  glutReshapeFunc(changeSize);

  glutKeyboardFunc(processKeys);

  //  OpenGL settings
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);

  // Enter GLUT's main cycle
  glutMainLoop();

  return 1;
}
