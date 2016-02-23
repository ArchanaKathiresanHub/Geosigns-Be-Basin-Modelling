#include "SceneInteractor.h"

#include <Inventor/events/SoMouseButtonEvent.h>
#include <Inventor/events/SoLocation2Event.h>
#include <Inventor/events/SoMouseWheelEvent.h>
#include <Inventor/events/SoKeyboardEvent.h>
#include <Inventor/touch/events/SoTouchEvent.h>
#include <Inventor/gestures/events/SoScaleGestureEvent.h>
#include <Inventor/gestures/events/SoRotateGestureEvent.h>
#include <Inventor/gestures/events/SoDoubleTapGestureEvent.h>
#include <Inventor/nodes/SoDirectionalLight.h>
#include <Inventor/nodes/SoSwitch.h>
#include <Inventor/nodes/SoTransformSeparator.h>
#include <Inventor/nodes/SoRotation.h>
#include <Inventor/nodes/SoPerspectiveCamera.h>
#include <Inventor/nodes/SoOrthographicCamera.h>
#include <Inventor/nodes/SoEventCallback.h>
#include <Inventor/touch/SoTouchManager.h>
#include <Inventor/ViewerComponents/SoCameraInteractor.h>

SceneInteractor::SceneInteractor()
{
  // Perspective camera
  SoPerspectiveCamera* perspCamera = new SoPerspectiveCamera();
  m_perspInteractor = SoCameraInteractor::getNewInstance(perspCamera);

  // Orthographic camera
  SoOrthographicCamera* orthoCamera = new SoOrthographicCamera();
  m_orthoInteractor = SoCameraInteractor::getNewInstance(orthoCamera);

  // Camera switch
  m_cameraSwitch = new SoSwitch();
  
  m_cameraSwitch->addChild(perspCamera);
  m_cameraSwitch->addChild(orthoCamera);

  // Headlight
  m_headlightRot = new SoRotation();
  SoTransformSeparator* transformSeparator = new SoTransformSeparator();
  transformSeparator->addChild(m_headlightRot.ptr());
  transformSeparator->addChild(new SoDirectionalLight());

  m_headlightSwitch = new SoSwitch();
  m_headlightSwitch->addChild(transformSeparator);

    // enable headlight by default
  m_headlightSwitch->whichChild.setValue(SO_SWITCH_ALL);

  // perspective camera by default
  setCameraMode(SceneInteractor::PERSPECTIVE);

  // Events callback
  m_eventCallBack = new SoEventCallback();

  // MOUSE
  m_eventCallBack->addEventCallback(SoMouseButtonEvent::getClassTypeId(), &mouseCB, this);
  m_eventCallBack->addEventCallback(SoLocation2Event::getClassTypeId(), &mouseMoveCB, this);
  m_eventCallBack->addEventCallback(SoMouseWheelEvent::getClassTypeId(), &wheelCB, this);

  // KEYBOARD
  m_eventCallBack->addEventCallback(SoKeyboardEvent::getClassTypeId(), &keyboardCB, this);

  // TOUCH
  m_eventCallBack->addEventCallback(SoTouchEvent::getClassTypeId(), &touchCB, this);
  m_eventCallBack->addEventCallback(SoScaleGestureEvent::getClassTypeId(), &zoomCB, this);
  m_eventCallBack->addEventCallback(SoRotateGestureEvent::getClassTypeId(), &rotateCB, this);
  m_eventCallBack->addEventCallback(SoDoubleTapGestureEvent::getClassTypeId(), &doubleTapCB, this);

  addChild(m_cameraSwitch.ptr());
  addChild(m_headlightSwitch.ptr());
  addChild(m_eventCallBack.ptr());
}

SceneInteractor::~SceneInteractor()
{
}

void
SceneInteractor::viewAll(const SbViewportRegion &viewport)
{
  m_cameraInteractor->viewAll(this, viewport);
}

SoCameraInteractor*
SceneInteractor::getCameraInteractor()
{
  return m_cameraInteractor;
}

void
SceneInteractor::setCameraMode(SceneInteractor::CameraMode mode)
{
  if (mode == SceneInteractor::PERSPECTIVE)
  {
    m_cameraInteractor = m_perspInteractor.ptr();
    m_cameraSwitch->whichChild = 0;
  }
  else if (mode == SceneInteractor::ORTHOGRAPHIC)
  {
    m_cameraInteractor = m_orthoInteractor.ptr();
    m_cameraSwitch->whichChild = 1;
  }
  m_headlightRot->rotation.connectFrom(&m_cameraInteractor->getCamera()->orientation);
}

SceneInteractor::CameraMode
SceneInteractor::getCameraMode()
{
  int activatedCamera = m_cameraSwitch->whichChild.getValue();

  if (activatedCamera == 0)
    return SceneInteractor::PERSPECTIVE;
  else
    return SceneInteractor::ORTHOGRAPHIC;
}

void
SceneInteractor::enableHeadLight(bool enabled)
{
  if (enabled)
    m_headlightSwitch->whichChild = SO_SWITCH_ALL;
  else
    m_headlightSwitch->whichChild = SO_SWITCH_NONE;
}

bool
SceneInteractor::isHeadLightEnabled()
{
  return m_headlightSwitch->whichChild.getValue() == SO_SWITCH_ALL;
}


/** Keyboard */
void
SceneInteractor::keyboardCB(void* userdata, SoEventCallback* node)
{
  SceneInteractor* _this = (SceneInteractor *) userdata;
  SoKeyboardEvent* keyboardEvent = (SoKeyboardEvent*) node->getEvent();

  if( keyboardEvent != NULL )
  {
    if( SoKeyboardEvent::isKeyPressEvent( keyboardEvent, SoKeyboardEvent::ANY ) )
      _this->keyPressed( keyboardEvent, node->getAction() );
    else if( SoKeyboardEvent::isKeyReleaseEvent( keyboardEvent, SoKeyboardEvent::ANY ) )
      _this->keyReleased( keyboardEvent, node->getAction() );
  }
}
void SceneInteractor::keyPressed( SoKeyboardEvent* , SoHandleEventAction* ) {}
void SceneInteractor::keyReleased( SoKeyboardEvent* , SoHandleEventAction* ) {}

/** Mouse button */
void
SceneInteractor::mouseCB( void* userdata, SoEventCallback* node )
{
  SceneInteractor* _this = (SceneInteractor *) userdata;
  SoMouseButtonEvent* mouseEvent = (SoMouseButtonEvent*) node->getEvent();

  if( mouseEvent != NULL )
  {
    if( SoMouseButtonEvent::isButtonPressEvent( mouseEvent, SoMouseButtonEvent::ANY ) )
      _this->mousePressed( mouseEvent, node->getAction() );
    else if( SoMouseButtonEvent::isButtonReleaseEvent( mouseEvent, SoMouseButtonEvent::ANY ) )
      _this->mouseReleased( mouseEvent, node->getAction() );
  }
}
void SceneInteractor::mousePressed( SoMouseButtonEvent* , SoHandleEventAction* ) {}
void SceneInteractor::mouseReleased( SoMouseButtonEvent* , SoHandleEventAction* ) {}

/** Mouse wheel */
void
SceneInteractor::wheelCB( void* userdata, SoEventCallback* node )
{
  SceneInteractor* _this = (SceneInteractor *) userdata;
  SoMouseWheelEvent* wheelEvent = (SoMouseWheelEvent*) node->getEvent();

  if( wheelEvent != NULL )
    _this->mouseWheelMoved( wheelEvent, node->getAction() );
}
void SceneInteractor::mouseWheelMoved( SoMouseWheelEvent* , SoHandleEventAction* ) {}

/** Mouse move */
void
SceneInteractor::mouseMoveCB( void* userdata, SoEventCallback* node )
{
  SceneInteractor* _this = (SceneInteractor *) userdata;
  SoLocation2Event* mouseEvent = (SoLocation2Event*) node->getEvent();

  if( mouseEvent != NULL )
    _this->mouseMoved( mouseEvent, node->getAction() );
}
void SceneInteractor::mouseMoved( SoLocation2Event* , SoHandleEventAction* ) {}


/** Touch */
void
SceneInteractor::touchCB(void* userdata, SoEventCallback* node)
{
  SceneInteractor* _this = (SceneInteractor *) userdata;
  SoTouchEvent* touchEvent = (SoTouchEvent *) node->getEvent();

  if( touchEvent != NULL )
    _this->touch( touchEvent, node->getAction() );
}
void SceneInteractor::touch( SoTouchEvent* , SoHandleEventAction* ) {}

void
SceneInteractor::zoomCB(void* userdata, SoEventCallback* node)
{
  SceneInteractor* _this = (SceneInteractor *) userdata;
  SoScaleGestureEvent* scaleEvent = (SoScaleGestureEvent*) node->getEvent();

  if( scaleEvent != NULL )
    _this->zoom( scaleEvent, node->getAction() );
}
void SceneInteractor::zoom( SoScaleGestureEvent* , SoHandleEventAction* ) {}

void
SceneInteractor::rotateCB( void* userdata, SoEventCallback* node )
{
  SceneInteractor* _this = (SceneInteractor *) userdata;
  SoRotateGestureEvent* rotateEvent = (SoRotateGestureEvent*) node->getEvent();

  if( rotateEvent != NULL )
    _this->rotate( rotateEvent, node->getAction() );
}
void SceneInteractor::rotate( SoRotateGestureEvent* , SoHandleEventAction* ) {}

void
SceneInteractor::doubleTapCB( void* userdata, SoEventCallback* node )
{
  SceneInteractor* _this = (SceneInteractor *) userdata;

  _this->viewAll(node->getAction()->getViewportRegion());
  node->getAction()->setHandled();
}
