#include "tinyxml2.h"
#include <GL/gl.h>
#include <GL/glut.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
// guide: https://shilohjames.wordpress.com/2014/04/27/tinyxml2-tutorial/
using namespace tinyxml2;
using namespace std;

void genPlane(float length, int divisions) {
  ostringstream fileName;
  fileName << "../3d/plane_" << length << "_" << divisions << ".3d";
  ofstream File(fileName.str(), ios::trunc);

  int fixDiv = 0;
  float ofset = 0;
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

void generateConfig() {
  XMLDocument xmlDoc;

  XMLNode *pRoot = xmlDoc.NewElement("world");
  xmlDoc.InsertFirstChild(pRoot);

  XMLElement *pElement = xmlDoc.NewElement("window");
  pElement->SetAttribute("width", "512");
  pElement->SetAttribute("height", "512");
  pRoot->InsertEndChild(pElement);

  XMLElement *pListElement = xmlDoc.NewElement("camera");

  pElement = xmlDoc.NewElement("position");
  pElement->SetAttribute("x", "3");
  pElement->SetAttribute("y", "2");
  pElement->SetAttribute("z", "1");
  pListElement->InsertEndChild(pElement);

  pElement = xmlDoc.NewElement("lookAt");
  pElement->SetAttribute("x", "0");
  pElement->SetAttribute("y", "0");
  pElement->SetAttribute("z", "0");
  pListElement->InsertEndChild(pElement);

  pElement = xmlDoc.NewElement("up");
  pElement->SetAttribute("x", "0");
  pElement->SetAttribute("y", "1");
  pElement->SetAttribute("z", "0");
  pListElement->InsertEndChild(pElement);

  pElement = xmlDoc.NewElement("projection");
  pElement->SetAttribute("fov", "60");
  pElement->SetAttribute("near", "1");
  pElement->SetAttribute("far", "1000");
  pListElement->InsertEndChild(pElement);

  pRoot->InsertEndChild(pListElement);

  XMLElement *pListListElement = xmlDoc.NewElement("group");
  pListElement = xmlDoc.NewElement("models");
  pListListElement->InsertEndChild(pListElement);

  pElement = xmlDoc.NewElement("model");
  pElement->SetAttribute("file", "plane.3d");
  pListElement->InsertEndChild(pElement);

  pElement = xmlDoc.NewElement("model");
  pElement->SetAttribute("file", "box.3d");
  pListElement->InsertEndChild(pElement);

  pElement = xmlDoc.NewElement("model");
  pElement->SetAttribute("file", "sphere.3d");
  pListElement->InsertEndChild(pElement);

  pElement = xmlDoc.NewElement("model");
  pElement->SetAttribute("file", "cone.3d");
  pListElement->InsertEndChild(pElement);

  pListListElement->InsertEndChild(pListElement);
  pRoot->InsertEndChild(pListListElement);

  xmlDoc.SaveFile("config.xml");
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

int main(int argc, char *argv[]) {

  // generateConfig();

  if (strcmp(argv[1], "plane") == 0) {
    cout << argv[1];
    if (stof(argv[2]) > 0 && stoi(argv[3]) > 0) {
      genPlane(stof(argv[2]), stoi(argv[3]));
    } else
      cout << "Invalid length(float) or divisions(int)." << endl;

  } else if (strcmp(argv[1], "box") == 0) {
    cout << argv[1];
    if (stof(argv[2]) > 0 && stoi(argv[3]) > 0) {
      genBox(stof(argv[2]), stoi(argv[3]));
    } else
      cout << "Invalid length(float) or divisions(int)." << endl;
  } else
    cout << "Invalid model (available: plane, box)" << endl;

  return 1;
}
