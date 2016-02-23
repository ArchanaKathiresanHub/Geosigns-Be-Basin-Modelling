#include "SceneExaminer.h"
#include "SceneGraphManager.h"

#include <cmath>
#include <Inventor/events/SoMouseWheelEvent.h>
#include <Inventor/events/SoKeyboardEvent.h>
#include <Inventor/touch/events/SoTouchEvent.h>
#include <Inventor/gestures/events/SoScaleGestureEvent.h>
#include <Inventor/gestures/events/SoRotateGestureEvent.h>
#include <Inventor/gestures/events/SoDoubleTapGestureEvent.h>
#include <Inventor/touch/SoTouchManager.h>
#include <Inventor/nodes/SoEventCallback.h>
#include <Inventor/ViewerComponents/SoCameraInteractor.h>
#include <Inventor/SoPickedPoint.h>

SceneExaminer::SceneExaminer(std::shared_ptr<SceneGraphManager> mgr)
  : m_isButton1Down(false)
  , m_isButton2Down(false)
  , m_isTouchOrbitActivated(false)
  , m_activeMode(VIEWING)
  , m_mousePositionNorm( 0.f, 0.f )
  , m_currentFenceId(-1)
  , m_scenegraph(mgr)
{
  m_mouseWheelDelta = SoPreferences::getInt( "OIV_WHEEL_DELTA", 120 );

  addChild(mgr->getRoot());
}

SceneExaminer::~SceneExaminer()
{
}

SceneExaminer::InteractionMode SceneExaminer::getInteractionMode()
{
  return m_activeMode;
}

void SceneExaminer::setModeChangedCallback(std::function<void(InteractionMode)> cb)
{
  m_modeChangedCallback = cb;
}

void SceneExaminer::setFenceAddedCallback(std::function<void(int)> cb)
{
  m_fenceAddedCallback = cb;
}

void SceneExaminer::setInteractionMode(SceneExaminer::InteractionMode mode)
{
  if (mode != m_activeMode)
  {
    m_activeMode = mode;
    if (m_modeChangedCallback)
      m_modeChangedCallback(mode);
  }
}

/** Keyboard */
void SceneExaminer::keyPressed( SoKeyboardEvent* keyboardEvent, SoHandleEventAction* action )
{
  SoKeyboardEvent::Key key = keyboardEvent->getKey();

  switch ( key )
  {
  case SoKeyboardEvent::LEFT_CONTROL:
    if(m_isButton1Down)
    {
      // BUTTON 1 + CTRL = pan
      m_cameraInteractor->activatePanning(m_mousePositionNorm, action->getViewportRegion());
      action->setHandled();
    }
    break;

  case SoKeyboardEvent::ESCAPE:
    if (m_activeMode == VIEWING)
      setInteractionMode(PICKING);
    else
      setInteractionMode(VIEWING);

    action->setHandled();
    break;

  case SoKeyboardEvent::F:
    if (m_activeMode != FENCE_EDITING)
    {
      m_currentFenceId = -1;
      m_fencePoints.clear();
      setInteractionMode(FENCE_EDITING);
      action->setHandled();
    }
    break;

  default:
    break;
  }
}

void SceneExaminer::keyReleased( SoKeyboardEvent* , SoHandleEventAction* )
{
}

/** Mouse */
void SceneExaminer::mousePressed( SoMouseButtonEvent* mouseEvent, SoHandleEventAction* action )
{
  SoMouseButtonEvent::Button button = mouseEvent->getButton();
  SbViewportRegion vpRegion = action->getViewportRegion();
  m_mousePositionNorm = mouseEvent->getNormalizedPosition( vpRegion );

  if ( button == SoMouseButtonEvent::BUTTON1)
    m_isButton1Down = true;
  if ( button == SoMouseButtonEvent::BUTTON2)
    m_isButton2Down = true;

  if (m_activeMode == VIEWING)
  {
    if (m_isButton1Down)
    {
      // BUTTON 1
      bool ctrlDown = mouseEvent->wasCtrlDown();

      if (ctrlDown)
      {
        // BUTTON 1 + CTRL = pan
        m_cameraInteractor->activatePanning(m_mousePositionNorm, vpRegion);
        action->setHandled();
      }
      else
      {
        // BUTTON 1 without modifier = orbit
        m_cameraInteractor->activateOrbiting(m_mousePositionNorm);
        m_cameraInteractor->setRotationCenter(m_cameraInteractor->getFocalPoint());
        action->setHandled();
      }
    }
    else if (m_isButton2Down)
    {
      m_cameraInteractor->activatePanning(m_mousePositionNorm, action->getViewportRegion());
      action->setHandled();
    }
  }
  else if (m_activeMode == PICKING)
  {
    // let scenegraph handle this one
  }
  else if (m_activeMode == FENCE_EDITING)
  {
    auto pickedPoint = action->getPickedPoint();
    if (pickedPoint)
    {
      auto p = pickedPoint->getPoint();
      m_fencePoints.push_back(p);
      if (m_fencePoints.size() == 1)
      {
        m_currentFenceId = m_scenegraph->addFence(m_fencePoints);
        if (m_fenceAddedCallback)
          m_fenceAddedCallback(m_currentFenceId);
      }
      else
      {
        m_scenegraph->updateFence(m_currentFenceId, m_fencePoints);
      }

      action->setHandled();
    }
  }
}

void SceneExaminer::mouseReleased( SoMouseButtonEvent* mouseEvent, SoHandleEventAction* action )
{
  SoMouseButtonEvent::Button button = mouseEvent->getButton();
  m_mousePositionNorm = mouseEvent->getNormalizedPosition( action->getViewportRegion() );

  if( button == SoMouseButtonEvent::BUTTON1 )
    m_isButton1Down = false;
  if ( button == SoMouseButtonEvent::BUTTON2)
    m_isButton2Down = false;
}

void SceneExaminer::mouseWheelMoved( SoMouseWheelEvent* wheelEvent, SoHandleEventAction* action )
{
  if ( m_activeMode == VIEWING )
  {
    // ZOOM
    SbViewportRegion vpRegion = action->getViewportRegion();
    int wheelDelta = wheelEvent->getDelta() / m_mouseWheelDelta;
    float scaleFactor= std::pow( 2.f, (float) (wheelDelta * M_PI / 180.0f) );

    m_cameraInteractor->dollyWithZoomCenter(m_mousePositionNorm, scaleFactor, vpRegion);
    m_cameraInteractor->adjustClippingPlanes(this, vpRegion);

    action->setHandled();
  }
}

void SceneExaminer::mouseMoved( SoLocation2Event* mouseEvent, SoHandleEventAction* action )
{
  if (m_activeMode != VIEWING) 
    return;

  bool ctrlDown = mouseEvent->wasCtrlDown();
  bool shiftDown = mouseEvent->wasShiftDown();

  SbViewportRegion vpRegion = action->getViewportRegion();

  SbVec2f newLocator = mouseEvent->getNormalizedPosition(vpRegion);
  float d = 10.0f * ( newLocator[1] - m_mousePositionNorm[1] );
  m_mousePositionNorm = newLocator; 

  if ( m_isButton1Down && m_isButton2Down )
  {
    // BUTTON 1 + BUTTON 2 = dolly
    m_cameraInteractor->dolly((float)pow( 2.0f, d ));
    m_cameraInteractor->adjustClippingPlanes(this, vpRegion);
    action->setHandled();   
  }
  else if ( m_isButton1Down )
  {
    // BUTTON 1
    if ( ctrlDown && shiftDown )
    {
      // BUTTON 1 + CTRL + SHIFT = dolly
      m_cameraInteractor->dolly((float)pow( 2.0f, d ));
      m_cameraInteractor->adjustClippingPlanes(this, vpRegion);
      action->setHandled();
    }
    else if (ctrlDown)
    {
      // BUTTON 1 + CTRL = pan
      m_cameraInteractor->pan(m_mousePositionNorm, vpRegion);
      action->setHandled();
    }
    else if (!ctrlDown)
    {
      // BUTTON 1 without modifier = orbit
      m_cameraInteractor->orbit(m_mousePositionNorm);
      m_cameraInteractor->adjustClippingPlanes(this, vpRegion);
      action->setHandled();
    }
  }
  else if (m_isButton2Down)
  {
    if ( ctrlDown )
    {
      // BUTTON 2 + CTRL = dolly
      m_cameraInteractor->dolly((float)pow( 2.0f, d ));
      m_cameraInteractor->adjustClippingPlanes(this, vpRegion);
      action->setHandled();
    } 
    else
    {
      // BUTTON 2 only = pan
      m_cameraInteractor->pan(m_mousePositionNorm, vpRegion);
      action->setHandled();
    }
  }
}

/** Touch */
SoEvent* SceneExaminer::convertTouchEvent(SoTouchEvent* touchEvent)
{
  SoMouseButtonEvent* mbe = &m_touchMouseButtonEvent;
  SoLocation2Event* lct = &m_touchLocation2Event;
  SoTouchEvent::State state = touchEvent->getState();

  if ( state == SoTouchEvent::DOWN)
  {
    mbe->setTime(touchEvent->getTime());
    mbe->setPosition(touchEvent->getPosition());
    mbe->setButton(SoMouseButtonEvent::BUTTON1);
    mbe->setState(SoMouseButtonEvent::DOWN);
    return mbe;
  }

  if ( state == SoTouchEvent::MOVE)
  {
    lct->setTime(touchEvent->getTime());
    lct->setPosition(touchEvent->getPosition());
    lct->setEventSource(SoLocation2Event::MOUSE_MOVE);
    return lct;
  }

  if ( state == SoTouchEvent::UP)
  {
    mbe->setTime(touchEvent->getTime());
    mbe->setPosition(touchEvent->getPosition());
    mbe->setButton(SoMouseButtonEvent::BUTTON1);
    mbe->setState(SoMouseButtonEvent::UP);
    return mbe;
  }

  SoDebugError::post( __FUNCTION__, "Unknown Touch Event" );
  return NULL;
}

void SceneExaminer::touch( SoTouchEvent* touchEvent, SoHandleEventAction* action )
{
  if( m_activeMode == VIEWING )
  {
    SbViewportRegion vpRegion = action->getViewportRegion();
    SbVec2f touchNormPosition = touchEvent->getNormalizedPosition(vpRegion);
    SoTouchEvent::State state = touchEvent->getState();
    int numFinger = touchEvent->getTouchManager()->getFingerNumber();

    if ( numFinger == 1)
    {
      if( state == SoTouchEvent::DOWN )
      {
        // one finger down
        m_cameraInteractor->activateOrbiting(touchNormPosition);
        m_cameraInteractor->setRotationCenter(m_cameraInteractor->getFocalPoint());
        m_isTouchOrbitActivated = true;
        action->setHandled();
      }
      else if ( state == SoTouchEvent::MOVE && m_isTouchOrbitActivated )
      {
        // one finger moved
        m_cameraInteractor->orbit(touchNormPosition);
        m_cameraInteractor->adjustClippingPlanes(this, vpRegion);
        action->setHandled();
      }
    }
    else if (numFinger == 2)
    {
      if (state == SoTouchEvent::DOWN || state == SoTouchEvent::MOVE)
      {
        // 2 fingers down or moved
        m_cameraInteractor->translate(vpRegion.normalize(SbVec2s(touchEvent->getDisplacement() * 0.5f)), vpRegion);
        action->setHandled();
      }
      else if ( state == SoTouchEvent::UP )
      {
        // one finger is on the screen but one has been lifted,
        // orbiting is temporarily disabled until the next touch down event.
        m_isTouchOrbitActivated = false;
      }
    }
  }
  else
  {
    // picking mode
    SoEvent* event_out = convertTouchEvent(touchEvent);
    action->setEvent(event_out);
  }
}

void SceneExaminer::zoom( SoScaleGestureEvent* scaleEvent, SoHandleEventAction* action )
{
  if(m_activeMode == VIEWING)
  {
    float delta = scaleEvent->getDeltaScaleFactor();
    const SbViewportRegion &region = action->getViewportRegion();
    SbVec2f normPosition = region.normalize(scaleEvent->getPosition());

    m_cameraInteractor->dollyWithZoomCenter(normPosition, 1.f/delta, region);

    m_cameraInteractor->adjustClippingPlanes(this, region);

    action->setHandled();
  }
}

void SceneExaminer::rotate( SoRotateGestureEvent* rotateEvent, SoHandleEventAction* action )
{
  if (m_activeMode == VIEWING)
  {
    SbViewportRegion vpRegion = action->getViewportRegion();
    SbVec2f eventNormPosition = rotateEvent->getNormalizedPosition(vpRegion);
    float distFromEye = m_cameraInteractor->getCamera()->getViewVolume().getNearDist();
    SbVec3f rotCenter = m_cameraInteractor->projectToPlane(eventNormPosition, distFromEye, vpRegion);

    m_cameraInteractor->setRotationAxis(SbVec3f(0, 0, 1.0f));
    m_cameraInteractor->setRotationCenter(rotCenter);
    m_cameraInteractor->rotate(- rotateEvent->getDeltaRotation() * 0.5f);
    action->setHandled();
  }
}

