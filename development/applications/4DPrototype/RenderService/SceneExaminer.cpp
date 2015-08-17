//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "SceneExaminer.h"

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

SceneExaminer::SceneExaminer()
  : m_isPickingEnabled(true)
  , m_isZoomEnabled(true)
  , m_isPanEnabled(true)
  , m_isOrbitEnabled(true)
  , m_isRotateEnabled(true)
  , m_isButton1Down(false)
  , m_isButton2Down(false)
  , m_isTouchOrbitActivated(false)
  , m_activeMode(VIEWING)
  , m_mousePositionNorm( 0.f, 0.f )
{
  m_mouseWheelDelta = SoPreferences::getInt( "OIV_WHEEL_DELTA", 120 );
}

SceneExaminer::~SceneExaminer()
{
}

void
SceneExaminer::enablePicking(bool enabled)
{
  m_isPickingEnabled = enabled;
}

bool
SceneExaminer::isPickingEnabled()
{
  return m_isPickingEnabled;
}

void
SceneExaminer::enableZoom(bool enabled)
{
  m_isZoomEnabled = enabled;
}

bool
SceneExaminer::isZoomEnabled()
{
  return m_isZoomEnabled;
}

void
SceneExaminer::enablePan(bool enabled)
{
  m_isPanEnabled = enabled;
}

bool
SceneExaminer::isPanEnabled()
{
  return m_isPanEnabled;
}

void
SceneExaminer::enableOrbit(bool enabled)
{
  m_isOrbitEnabled = enabled;
}

bool
SceneExaminer::isOrbitEnabled()
{
  return m_isOrbitEnabled;
}

void
SceneExaminer::enableRotate(bool enabled)
{
  m_isRotateEnabled = enabled;
}

bool
SceneExaminer::isRotateEnabled()
{
  return m_isRotateEnabled;
}

SceneExaminer::InteractionMode
SceneExaminer::getInteractionMode()
{
  return m_activeMode;
}

void 
SceneExaminer::setInteractionMode(SceneExaminer::InteractionMode mode)
{
  m_activeMode = mode;
}


/** Keyboard */
void
SceneExaminer::keyPressed( SoKeyboardEvent* keyboardEvent, SoHandleEventAction* action )
{
  SoKeyboardEvent::Key key = keyboardEvent->getKey();

  switch ( key )
  {
  case SoKeyboardEvent::LEFT_CONTROL:
    if( m_isPanEnabled && m_isButton1Down )
    {
      // BUTTON 1 + CTRL = pan
      m_cameraInteractor->activatePanning(m_mousePositionNorm, action->getViewportRegion());
      action->setHandled();
    }
    break;
  case SoKeyboardEvent::ESCAPE:
    if( m_isPickingEnabled )
    {
      if (m_activeMode == VIEWING)
      {
        // Activate picking mode
        m_activeMode = PICKING;
      } 
      else if (m_activeMode == PICKING)
      {
        // Activate viewing mode
        m_activeMode = VIEWING;
      }
      action->setHandled();
    }
    break;
  default:
    break;
  }
}

void
SceneExaminer::keyReleased( SoKeyboardEvent* , SoHandleEventAction* )
{
}

/** Mouse */
void
SceneExaminer::mousePressed( SoMouseButtonEvent* mouseEvent, SoHandleEventAction* action )
{
  SoMouseButtonEvent::Button button = mouseEvent->getButton();
  SbViewportRegion vpRegion = action->getViewportRegion();
  m_mousePositionNorm = mouseEvent->getNormalizedPosition( vpRegion );

  if ( button == SoMouseButtonEvent::BUTTON1)
    m_isButton1Down = true;
  if ( button == SoMouseButtonEvent::BUTTON2)
    m_isButton2Down = true;

  if ( m_isButton1Down && m_activeMode == VIEWING )
  {
    // BUTTON 1
    bool ctrlDown = mouseEvent->wasCtrlDown();

    if ( m_isPanEnabled && ctrlDown )
    {
      // BUTTON 1 + CTRL = pan
      m_cameraInteractor->activatePanning(m_mousePositionNorm, vpRegion);
      action->setHandled();
    }
    else if ( m_isOrbitEnabled && !ctrlDown )
    {
      // BUTTON 1 without modifier = orbit
      m_cameraInteractor->activateOrbiting(m_mousePositionNorm);
      m_cameraInteractor->setRotationCenter(m_cameraInteractor->getFocalPoint());
      action->setHandled();
    }
  }
  else if (m_isPanEnabled && m_isButton2Down)
  {
    m_cameraInteractor->activatePanning(m_mousePositionNorm, action->getViewportRegion());
    action->setHandled();
  }
}

void
SceneExaminer::mouseReleased( SoMouseButtonEvent* mouseEvent, SoHandleEventAction* action )
{
  SoMouseButtonEvent::Button button = mouseEvent->getButton();
  m_mousePositionNorm = mouseEvent->getNormalizedPosition( action->getViewportRegion() );

  if( button == SoMouseButtonEvent::BUTTON1 )
    m_isButton1Down = false;
  if ( button == SoMouseButtonEvent::BUTTON2)
    m_isButton2Down = false;
}

void
SceneExaminer::mouseWheelMoved( SoMouseWheelEvent* wheelEvent, SoHandleEventAction* action )
{
  if ( m_isZoomEnabled && m_activeMode == VIEWING )
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

void
SceneExaminer::mouseMoved( SoLocation2Event* mouseEvent, SoHandleEventAction* action )
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
    else if ( m_isPanEnabled && ctrlDown )
    {
      // BUTTON 1 + CTRL = pan
      m_cameraInteractor->pan(m_mousePositionNorm, vpRegion);
      action->setHandled();
    }
    else if ( m_isOrbitEnabled && !ctrlDown )
    {
      // BUTTON 1 without modifier = orbit
      m_cameraInteractor->orbit(m_mousePositionNorm);
      m_cameraInteractor->adjustClippingPlanes(this, vpRegion);
      action->setHandled();
    }
  }
  else if ( m_isButton2Down )
  {
    if ( ctrlDown )
    {
      // BUTTON 2 + CTRL = dolly
      m_cameraInteractor->dolly((float)pow( 2.0f, d ));
      m_cameraInteractor->adjustClippingPlanes(this, vpRegion);
      action->setHandled();
    } 
    else if ( m_isPanEnabled  )
    {
      // BUTTON 2 only = pan
      m_cameraInteractor->pan(m_mousePositionNorm, vpRegion);
      action->setHandled();
    }
  }
}

/** Touch */
SoEvent*
SceneExaminer::convertTouchEvent(SoTouchEvent* touchEvent)
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

void
SceneExaminer::touch( SoTouchEvent* touchEvent, SoHandleEventAction* action )
{
  if( m_activeMode == VIEWING )
  {
    SbViewportRegion vpRegion = action->getViewportRegion();
    SbVec2f touchNormPosition = touchEvent->getNormalizedPosition(vpRegion);
    SoTouchEvent::State state = touchEvent->getState();
    int numFinger = touchEvent->getTouchManager()->getFingerNumber();

    if ( numFinger == 1)
    {
      if ( m_isOrbitEnabled )
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
    }
    else if (numFinger == 2)
    {
      if ( m_isPanEnabled && ( state == SoTouchEvent::DOWN || state == SoTouchEvent::MOVE ) )
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

void
SceneExaminer::zoom( SoScaleGestureEvent* scaleEvent, SoHandleEventAction* action )
{
  if( m_isZoomEnabled &&  m_activeMode == VIEWING )
  {
    float delta = scaleEvent->getDeltaScaleFactor();
    const SbViewportRegion &region = action->getViewportRegion();
    SbVec2f normPosition = region.normalize(scaleEvent->getPosition());

    m_cameraInteractor->dollyWithZoomCenter(normPosition, 1.f/delta, region);

    m_cameraInteractor->adjustClippingPlanes(this, region);

    action->setHandled();
  }
}

void
SceneExaminer::rotate( SoRotateGestureEvent* rotateEvent, SoHandleEventAction* action )
{
  if ( m_isRotateEnabled && m_activeMode == VIEWING )
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
