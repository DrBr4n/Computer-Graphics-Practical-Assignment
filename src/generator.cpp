#include "tinyxml2.h"
// guide: https://shilohjames.wordpress.com/2014/04/27/tinyxml2-tutorial/
using namespace tinyxml2;

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

int main(int argc, char *argv[]) {
    
    generateConfig();

    return 1;
}
