#pragma once

#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoSwitch.h>
#include <Inventor/nodes/SoPerspectiveCamera.h>
#include <Inventor/nodes/SoEventCallback.h>
#include <Inventor/ViewerComponents/SoCameraInteractor.h>
#include <Inventor/projectors/SbSphereSheetProjector.h>
#include <Inventor/touch/SoTouchManager.h>

class SceneExaminer : public SoSeparator
{
public:
  SceneExaminer();
  ~SceneExaminer();
  SoCamera* getCamera();
  void viewAll(const SbViewportRegion &viewport);
  void setPickingMode(bool value);
  bool isPickingMode();
  void setHeadLightMode(bool value);
  bool isHeadLightMode();
private:
  enum ViewingMode
  {
    NOTHING,
    ORBIT,
    ACTIVATEPAN,
    PAN,
    PICK
  };
  SbVec2f m_mousePositionNorm;
  SoPerspectiveCamera* m_perspCamera;
  SoEventCallback* m_eventCallBack;
  SoSwitch * m_switch;
  SoCameraInteractor* m_interactor;
  static void mouseMoveCB(void * userdata, SoEventCallback * node);
  static void mouseCB(void * userdata, SoEventCallback * node);
  static void keyboardCB(void * userdata, SoEventCallback * node);
  static void touchCB(void * userdata, SoEventCallback * node);
  static void zoomCB(void * userdata, SoEventCallback * node);
  static void rotateCB(void * userdata, SoEventCallback * node);
  static void doubleTapCB(void * userdata, SoEventCallback * node);
  static void longTapCB(void * userdata, SoEventCallback * node);
  static void wheelCB(void * userdata, SoEventCallback * node);
  ViewingMode m_activeViewingMode;
  bool m_isButton1Down;
  bool m_isButton2Down;
  bool m_isButton3Down;
  bool m_headLightMode;
};

