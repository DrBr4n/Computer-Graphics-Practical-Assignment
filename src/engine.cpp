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

#include <IL/il.h>

#include <GL/glew.h>
#include <GL/glut.h>

#include "tinyxml2.h"

using namespace tinyxml2;

struct Config *gpConfigData = new Config();
struct Group *gpGroupRoot = NULL;
GLuint gBuffers[20];
std::vector<struct VBOsInfo> gVBOsInfo;
std::vector<struct Model> gModels;
std::vector<struct Light> gLights;

float gPrevY[3] = {0, 1, 0};
int gModelCounter = 0;

int main(int argc, char **argv) {
  parseConfig(argv[1]);

  // Init GLUT and the window
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
  glutInitWindowPosition(100, 1200);
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
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_NORMAL_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  // glEnable(GL_RESCALE_NORMAL);

  genVBOs();

  // radius is not correct to camera position
  gpConfigData->radius = 5.0f;
  // asin(xmlInfo.camPos.y / xmlInfo.radius);
  gpConfigData->beta = 0;
  // asin(xmlInfo.camPos.x / (xmlInfo.radius * cos(xmlInfo.beta)));
  gpConfigData->alfa = 0;

  initLights();
  initAndLoadTextures();

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

  setupLights();

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

  Vector3D posVector = {0.0, 0.0, 0.0};
  Vector3D dirVector = {0.0, 0.0, 0.0};
  pElem = pRoot->FirstChildElement("lights");
  pListElem = pElem->FirstChildElement("light");
  for (; pListElem != NULL; pListElem = pListElem->NextSiblingElement()) {
    struct Light pNewLight;
    const char *type = nullptr;
    pListElem->QueryStringAttribute("type", &type);
    pNewLight.type = type;
    if (std::strcmp(pNewLight.type.data(), "point") == 0 ||
        std::strcmp(pNewLight.type.data(), "spot") == 0) {
      pListElem->QueryFloatAttribute("posx", &posVector.x);
      pListElem->QueryFloatAttribute("posy", &posVector.y);
      pListElem->QueryFloatAttribute("posz", &posVector.z);
      pNewLight.pos = posVector;
    }
    if (std::strcmp(pNewLight.type.data(), "directional") == 0 ||
        std::strcmp(pNewLight.type.data(), "spot") == 0) {
      pListElem->QueryFloatAttribute("dirx", &dirVector.x);
      pListElem->QueryFloatAttribute("diry", &dirVector.y);
      pListElem->QueryFloatAttribute("dirz", &dirVector.z);
      pNewLight.dir = dirVector;
    }
    if (std::strcmp(pNewLight.type.data(), "spot") == 0) {
      pListElem->QueryIntAttribute("cutoff", &pNewLight.cutoff);
    }
    gLights.push_back(pNewLight);
  }

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
    XMLElement *pListElem, *pColorElem;
    const char *lightComp[4] = {"diffuse", "ambient", "specular", "emissive"};

    for (pElem = pElem->FirstChildElement("model"); pElem != NULL;
         pElem = pElem->NextSiblingElement("model")) {
      pGroup->modelIds.emplace_back(gModelCounter);

      struct Model *pNewModel = new Model();
      pNewModel->id = gModelCounter++;
      const char *name = nullptr;
      pElem->QueryStringAttribute("file", &name);
      pNewModel->name = name;

      if (pElem->FirstChildElement("texture") != NULL) {
        pListElem = pElem->FirstChildElement("texture");
        const char *texture = nullptr;
        pListElem->QueryStringAttribute("file", &texture);
        pNewModel->texture = texture;
      } else {
        pNewModel->texture = "EMPTY";
      }

      if (pElem->FirstChildElement("color") != NULL) {
        pListElem = pElem->FirstChildElement("color");
        for (int i = 0; i < 4; i++) {
          pColorElem = pListElem->FirstChildElement(lightComp[i]);
          pColorElem->QueryFloatAttribute("R", &pNewModel->lightComp[i][0]);
          pColorElem->QueryFloatAttribute("G", &pNewModel->lightComp[i][1]);
          pColorElem->QueryFloatAttribute("B", &pNewModel->lightComp[i][2]);
        }
        pColorElem = pListElem->FirstChildElement("shininess");
        pColorElem->QueryFloatAttribute("value", &pNewModel->shininess);
      } else {
        GLfloat defLightComp[4][3] = {
            {200, 200, 200}, {50, 50, 50}, {0, 0, 0}, {0, 0, 0}};
        for (int i = 0; i < 4; i++) {
          for (int j = 0; j < 3; j++) {
            pNewModel->lightComp[i][j] = defLightComp[i][j];
          }
        }
        pNewModel->shininess = 0;
      }
      gModels.push_back(*pNewModel);
      delete pNewModel;
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

void genVBOs() {
  // Create vector of modelNames and remove duplicates to see how many vbos are
  // needed
  std::vector<std::string> modelNames;
  for (const auto &model : gModels) {
    modelNames.push_back(model.name);
  }
  std::sort(modelNames.begin(), modelNames.end());
  modelNames.erase(unique(modelNames.begin(), modelNames.end()),
                   modelNames.end());

  glGenBuffers(modelNames.size() * 3, gBuffers);

  int bufferIdx = 0;
  for (const auto &modelName : modelNames) {

    std::vector<float> vertex, normals, textures;
    float x, y, z, nx, ny, nz, s, t;

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
      lineStream >> nx;
      lineStream >> ny;
      lineStream >> nz;
      normals.push_back(nx);
      normals.push_back(ny);
      normals.push_back(nz);
      lineStream >> s;
      lineStream >> t;
      textures.push_back(s);
      textures.push_back(t);
    }
    File.close();

    struct VBOsInfo VBOsInfoNode;
    VBOsInfoNode.modelName = modelName;
    VBOsInfoNode.vertexCount = vertex.size();

    VBOsInfoNode.vertexBufferIndex = bufferIdx;
    glBindBuffer(GL_ARRAY_BUFFER, gBuffers[bufferIdx++]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertex.size(), vertex.data(),
                 GL_STATIC_DRAW);

    VBOsInfoNode.normalBufferIndex = bufferIdx;
    glBindBuffer(GL_ARRAY_BUFFER, gBuffers[bufferIdx++]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * normals.size(),
                 normals.data(), GL_STATIC_DRAW);

    VBOsInfoNode.texBufferIndex = bufferIdx;
    glBindBuffer(GL_ARRAY_BUFFER, gBuffers[bufferIdx++]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * textures.size(),
                 textures.data(), GL_STATIC_DRAW);

    gVBOsInfo.push_back(VBOsInfoNode);
  }
}

void drawAxes() {
  glDisable(GL_LIGHTING);
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
  glEnable(GL_LIGHTING);
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

  for (const auto &modelId : group->modelIds) {
    for (const auto &model : gModels) {
      if (model.id == modelId) {
        for (const auto &vboInfo : gVBOsInfo) {
          if (vboInfo.modelName == model.name) {
            float diffuse[4] = {model.lightComp[0][0], model.lightComp[0][1],
                                model.lightComp[0][2], 1.0f};
            float ambient[4] = {model.lightComp[1][0], model.lightComp[1][1],
                                model.lightComp[1][2], 1.0f};
            float specular[4] = {model.lightComp[2][0], model.lightComp[2][1],
                                 model.lightComp[2][2], 1.0f};
            float emission[4] = {model.lightComp[3][0], model.lightComp[3][1],
                                 model.lightComp[3][2], 1.0f};
            // glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
            // glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
            // glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
            // glMaterialfv(GL_FRONT, GL_EMISSION, emission);
            // glMaterialf(GL_FRONT, GL_SHININESS, model.shininess);

            glBindBuffer(GL_ARRAY_BUFFER, gBuffers[vboInfo.vertexBufferIndex]);
            glVertexPointer(3, GL_FLOAT, 0, 0);

            glBindBuffer(GL_ARRAY_BUFFER, gBuffers[vboInfo.normalBufferIndex]);
            glNormalPointer(GL_FLOAT, 0, 0);

            if (std::strcmp(model.texture.data(), "EMPTY") != 0) {
              glBindTexture(GL_TEXTURE_2D, model.textureId);
              glBindBuffer(GL_ARRAY_BUFFER, gBuffers[vboInfo.texBufferIndex]);
              glTexCoordPointer(2, GL_FLOAT, 0, 0);
            }

            glDrawArrays(GL_TRIANGLES, 0, vboInfo.vertexCount);

            glBindTexture(GL_TEXTURE_2D, 0);
          }
        }
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

// Lights
void initLights() {
  glEnable(GL_LIGHTING);

  float dark[4] = {0.2, 0.2, 0.2, 1.0};
  float white[4] = {1.0, 1.0, 1.0, 1.0};
  float black[4] = {0.0f, 0.0f, 0.0f, 0.0f};

  for (int i = 0; i < gLights.size(); i++) {
    glEnable(GL_LIGHT0 + i);
    glLightfv(GL_LIGHT0 + i, GL_AMBIENT, dark);
    glLightfv(GL_LIGHT0 + i, GL_DIFFUSE, white);
    glLightfv(GL_LIGHT0 + i, GL_SPECULAR, white);
  }

  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, black);
}

void setupLights() {
  for (int i = 0; i < gLights.size(); i++) {
    if (std::strcmp(gLights[i].type.data(), "point") == 0) {
      const GLfloat pos[4] = {gLights[i].pos.x, gLights[i].pos.y,
                              gLights[i].pos.z, 1};
      glLightfv(GL_LIGHT0 + i, GL_POSITION, pos);

    } else if (std::strcmp(gLights[i].type.data(), "directional") == 0) {
      const GLfloat dir[4] = {gLights[i].dir.x, gLights[i].dir.y,
                              gLights[i].dir.z, 0};
      glLightfv(GL_LIGHT0 + i, GL_POSITION, dir);

    } else if (std::strcmp(gLights[i].type.data(), "spot") == 0) {
      const GLfloat pos[4] = {gLights[i].pos.x, gLights[i].pos.y,
                              gLights[i].pos.z, 1};
      const GLfloat dir[3] = {gLights[i].dir.x, gLights[i].dir.y,
                              gLights[i].dir.z};
      glLightfv(GL_LIGHT0 + i, GL_POSITION, pos);
      glLightfv(GL_LIGHT0 + i, GL_SPOT_DIRECTION, dir);
      glLightf(GL_LIGHT0 + i, GL_SPOT_CUTOFF, gLights[i].cutoff);
    }
  }
}

void initAndLoadTextures() {
  ilInit();
  ilEnable(IL_ORIGIN_SET);
  ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
  glEnable(GL_TEXTURE_2D);

  for (auto &model : gModels) {
    if (std::strcmp(model.texture.data(), "EMPTY") != 0) {
      model.textureId = loadTexture(model.texture);
    }
  }
}

int loadTexture(std::string file) {
  unsigned int t, tw, th, texID;
  unsigned char *texData;

  ilGenImages(1, &t);
  ilBindImage(t);
  ilLoadImage((ILstring)file.c_str());
  tw = ilGetInteger(IL_IMAGE_WIDTH);
  th = ilGetInteger(IL_IMAGE_HEIGHT);
  ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
  texData = ilGetData();

  glGenTextures(1, &texID);

  glBindTexture(GL_TEXTURE_2D, texID);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tw, th, 0, GL_RGBA, GL_UNSIGNED_BYTE,
               texData);
  glGenerateMipmap(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, 0);

  return texID;
}
