#include "BpaRenderAreaListener.h"

#include <RemoteViz/Rendering/RenderArea.h>
#include <RemoteViz/Rendering/Connection.h>
#include <RemoteViz/Rendering/RenderAreaSettings.h>

#include <Inventor/SoSceneManager.h>
#include <Inventor/Nodes/SoSeparator.h>

#include <string>
#include <list>
#include <sstream>

using namespace std;

std::list<std::string> &split(const std::string &s, char delim, std::list<std::string> &elems) {
    stringstream ss(s);
    string item;
    while (getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

void BpaRenderAreaListener::onOpenedConnection(RenderArea* renderArea, Connection* connection)
{
  std::cout << "[BpaRenderAreaListener] onOpenedConnection(renderArea = " << renderArea->getId() << ", connection = " << connection->getId() << ")" << std::endl;
}

void BpaRenderAreaListener::onClosedConnection(RenderArea* renderArea, const std::string& connectionId)
{
  std::cout << "[BpaRenderAreaListener] onClosedConnection(renderArea = " << renderArea->getId() << ", connection = " << connectionId << ")" << std::endl;
}

void BpaRenderAreaListener::onReceivedMessage(RenderArea* renderArea, Connection* sender, const string& message)
{
  std::cout << "[BpaRenderAreaListener] onReceivedMessage(renderArea = " << renderArea->getId() << ", message = " << message << ")" << std::endl;
  SoSeparator* root = (SoSeparator*) renderArea->getSceneManager()->getSceneGraph();
  
  std::list<std::string> elems;
  split(message, ' ', elems); // split the received message, pattern : "COMMAND ARGUMENT"

  string command = elems.front();
  elems.pop_front();
  string argument = elems.front();

  // parse the commands
  if (command == "FPS"){
    renderArea->getSettings()->setMaxSendingFPS(atoi(argument.c_str()));
  }else if (command == "STILLQUALITY"){
    renderArea->getSettings()->setStillCompressionQuality((float)atof(argument.c_str())); 
  }else if (command == "SCALEFACTOR"){
    renderArea->getSettings()->setInteractiveScaleFactor((float)atof(argument.c_str()));
  }else if (command == "INTERACTIVEQUALITY"){
    renderArea->getSettings()->setInteractiveCompressionQuality((float)atof(argument.c_str())); 
  }else if (command == "WIDTH"){
    renderArea->resize(atoi(argument.c_str()),renderArea->getSceneManager()->getSize()[1]);
  }else if (command == "HEIGHT"){
    renderArea->resize(renderArea->getSceneManager()->getSize()[0], atoi(argument.c_str()));
  }else if(command == "SNAPSHOT")
  {
    int index = atoi(argument.c_str());
  }

}

void BpaRenderAreaListener::onRender(RenderArea* renderArea)
{
  //std::cout << "[BpaRenderAreaListener] onRender(renderArea = " << renderArea->getId() << ")" << std::endl;
}

void BpaRenderAreaListener::onResize(RenderArea* renderArea, unsigned int width, unsigned int height)
{
  std::cout << "[BpaRenderAreaListener] onResize(renderArea = " << renderArea->getId() << ", width = " << width << ", height = " << height << ")" << std::endl;
}

void BpaRenderAreaListener::onRequestedSize(RenderArea* renderArea, Connection* sender, unsigned int width, unsigned int height)
{
  std::cout << "[BpaRenderAreaListener] onRequestedResize(renderArea = " 
    << renderArea->getId() 
    << ", connection = " << sender->getId() 
    << ", width = " << width 
    << ", height = " << height << ")" << std::endl;
}

bool BpaRenderAreaListener::onMouseUp(int x, int y, int button)
{
  return true;
}

bool BpaRenderAreaListener::onMouseDown(int x, int y, int button)
{
  return true;
}

bool BpaRenderAreaListener::onMouseMove(int x, int y)
{
  return true;
}

bool BpaRenderAreaListener::onMouseEnter(int x, int y)
{
  return true;
}

bool BpaRenderAreaListener::onMouseLeave(int x, int y)
{
  return true;
}

bool BpaRenderAreaListener::onMouseWheel(int delta)
{
  return true;
}

bool BpaRenderAreaListener::onKeyUp(const std::string& key)
{
  return true;
}

bool BpaRenderAreaListener::onKeyDown(const std::string& key)
{
  return true;
}

bool BpaRenderAreaListener::onTouchStart(unsigned int id, int x, int y)
{
  return true;
}

bool BpaRenderAreaListener::onTouchEnd(unsigned int id, int x, int y)
{
  return true;
}

bool BpaRenderAreaListener::onTouchMove(unsigned int id, int x, int y)
{
  return true;
}

