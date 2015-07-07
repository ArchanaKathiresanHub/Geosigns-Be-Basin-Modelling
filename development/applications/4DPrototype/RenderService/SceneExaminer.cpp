#include "SceneExaminer.h"
#include <Inventor/events/SoMouseButtonEvent.h>
#include <Inventor/events/SoLocation2Event.h>
#include <Inventor/events/SoMouseWheelEvent.h>
#include <Inventor/events/SoKeyboardEvent.h>
#include <Inventor/touch/events/SoTouchEvent.h>
#include <Inventor/gestures/events/SoScaleGestureEvent.h>
#include <Inventor/gestures/events/SoRotateGestureEvent.h>
#include <Inventor/gestures/events/SoDoubleTapGestureEvent.h>
#include <Inventor/gestures/events/SoLongTapGestureEvent.h>
#include <Inventor/nodes/SoRotation.h>
#include <Inventor/nodes/SoDirectionalLight.h>
#include <Inventor/nodes/SoTransformSeparator.h>

#define ZOOM 0.9f

void SceneExaminer::viewAll(const SbViewportRegion &viewport){
  m_perspCamera->viewAll(this, viewport);
  m_interactor->setRotationCenter(m_interactor->getFocalPoint());
}

/** Mouse */

void SceneExaminer::wheelCB(void * userdata, SoEventCallback * node){
  SceneExaminer * examiner = (SceneExaminer *) userdata;
  SoMouseWheelEvent *wheelEvent = (SoMouseWheelEvent*) node->getEvent();

  if (wheelEvent->getDelta() > 0){
    examiner->m_interactor->dollyWithZoomCenter(examiner->m_mousePositionNorm, .9f, node->getAction()->getViewportRegion());
  }else{
    examiner->m_interactor->dollyWithZoomCenter(examiner->m_mousePositionNorm, 1.1f, node->getAction()->getViewportRegion());
  }

  examiner->m_interactor->adjustClippingPlanes(examiner, node->getAction()->getViewportRegion());
}

void SceneExaminer::mouseMoveCB(void * userdata, SoEventCallback * node){
  SceneExaminer * examiner = (SceneExaminer *) userdata;
  SoLocation2Event *mouseEvent = (SoLocation2Event*) node->getEvent();

  if (mouseEvent != NULL && examiner->m_activeViewingMode != PICK){
    if (mouseEvent->getEventSource() == SoLocation2Event::MOUSE_LEAVE){
      examiner->m_activeViewingMode = NOTHING;
    }else{

      examiner->m_mousePositionNorm = mouseEvent->getNormalizedPosition(node->getAction()->getViewportRegion());  

      switch (examiner->m_activeViewingMode)
      {
      case ORBIT:
        examiner->m_interactor->orbit(examiner->m_mousePositionNorm);
        break;
      case PAN:
        examiner->m_interactor->pan(examiner->m_mousePositionNorm, node->getAction()->getViewportRegion());
        break;
      }

      examiner->m_interactor->adjustClippingPlanes(examiner, node->getAction()->getViewportRegion());

      examiner->m_interactor->setRotationCenter(examiner->m_interactor->getFocalPoint());

    }

    node->getAction()->setHandled();

  }

};

void SceneExaminer::mouseCB(void * userdata, SoEventCallback * node){
  SceneExaminer * examiner = (SceneExaminer *) userdata;
  SoMouseButtonEvent *mouseEvent = (SoMouseButtonEvent*) node->getEvent();

  if (mouseEvent != NULL && examiner->m_activeViewingMode != PICK){

    examiner->m_mousePositionNorm = mouseEvent->getNormalizedPosition(node->getAction()->getViewportRegion());  

    if (mouseEvent->getButton() == SoMouseButtonEvent::BUTTON1){ // LEFT BOUTON

      if (mouseEvent->getState() == SoMouseButtonEvent::DOWN){
        examiner->m_isButton1Down = true;

        switch (examiner->m_activeViewingMode)
        {
        case NOTHING:
          examiner->m_interactor->activateOrbiting(examiner->m_mousePositionNorm);
          examiner->m_activeViewingMode = ORBIT;
          break;
        case ACTIVATEPAN:
          examiner->m_interactor->activatePanning(examiner->m_mousePositionNorm,node->getAction()->getViewportRegion());
          examiner->m_activeViewingMode = PAN;
          break;
        }

      }else if (mouseEvent->getState() == SoMouseButtonEvent::UP){
        examiner->m_isButton1Down = false;
        examiner->m_activeViewingMode = NOTHING;
      }

    }else  if (mouseEvent->getButton() == SoMouseButtonEvent::BUTTON2){ // WHEEL BOUTON
      if (mouseEvent->getState() == SoMouseButtonEvent::DOWN){
        examiner->m_isButton2Down = true;
      }else if (mouseEvent->getState() == SoMouseButtonEvent::UP){
        examiner->m_isButton2Down = false;
      }
    }else  if (mouseEvent->getButton() == SoMouseButtonEvent::BUTTON3){ // RIGHT BOUTON
      if (mouseEvent->getState() == SoMouseButtonEvent::DOWN){
        examiner->m_isButton3Down = true;
      }else if (mouseEvent->getState() == SoMouseButtonEvent::UP){
        examiner->m_isButton3Down = false;
      }
    }

    node->getAction()->setHandled();

  }

};

/** Keyboard */

void SceneExaminer::keyboardCB(void * userdata, SoEventCallback * node){
  SceneExaminer * examiner = (SceneExaminer *) userdata;
  SoKeyboardEvent *keyboardEvent = (SoKeyboardEvent*) node->getEvent();

  if (keyboardEvent != NULL){
    if (examiner->m_activeViewingMode != PICK){

      if (keyboardEvent->getKey() == SoKeyboardEvent::LEFT_CONTROL){
        if (keyboardEvent->getState() == SoButtonEvent::DOWN){

          if (examiner->m_activeViewingMode != PAN){

            if (examiner->m_isButton1Down){
              examiner->m_interactor->activatePanning(examiner->m_mousePositionNorm,node->getAction()->getViewportRegion());
              examiner->m_activeViewingMode = PAN;
            } else {
              examiner->m_activeViewingMode = ACTIVATEPAN;
            }
          }

        }else if (keyboardEvent->getState() == SoButtonEvent::UP){
          examiner->m_activeViewingMode = NOTHING;
        }
      }else if (keyboardEvent->getKey() == SoKeyboardEvent::LEFT_SHIFT && keyboardEvent->getState() == SoButtonEvent::DOWN){
        examiner->m_activeViewingMode = PICK;    
      }
      node->getAction()->setHandled();

    }else if (keyboardEvent->getKey() == SoKeyboardEvent::LEFT_SHIFT && keyboardEvent->getState() == SoButtonEvent::UP){
      examiner->m_activeViewingMode = NOTHING;    
    }
  }

};

/** Touch */

void SceneExaminer::touchCB(void * userdata, SoEventCallback * node){
  SceneExaminer * examiner = (SceneExaminer *) userdata;
  SoTouchEvent * touchevent = (SoTouchEvent *) node->getEvent();

  if (touchevent != NULL && examiner->m_activeViewingMode != PICK){
    SbVec2f touchnormposition = touchevent->getNormalizedPosition(node->getAction()->getViewportRegion());
    if (touchevent->getTouchManager()->getFingerNumber() == 1){
      if (touchevent->getState() == SoTouchEvent::DOWN && examiner->m_activeViewingMode != ORBIT){
        examiner->m_interactor->activateOrbiting(touchevent->getNormalizedPosition(node->getAction()->getViewportRegion()));
        examiner->m_activeViewingMode = ORBIT;
      } else if (touchevent->getState() == SoTouchEvent::UP){
        examiner->m_activeViewingMode = NOTHING;
      } else if (touchevent->getState() == SoTouchEvent::MOVE && examiner->m_activeViewingMode == ORBIT){
        examiner->m_interactor->setRotationCenter(examiner->m_interactor->getFocalPoint());
        examiner->m_interactor->orbit(touchevent->getNormalizedPosition(node->getAction()->getViewportRegion()));
        examiner->m_interactor->adjustClippingPlanes(examiner, node->getAction()->getViewportRegion());
      }

    } else if (touchevent->getTouchManager()->getFingerNumber() == 2){
      if (touchevent->getState() == SoTouchEvent::DOWN){
        examiner->m_interactor->translate(node->getAction()->getViewportRegion().normalize(SbVec2s(touchevent->getDisplacement() * 0.5f)), node->getAction()->getViewportRegion());
        examiner->m_activeViewingMode = PAN;
      } else if (touchevent->getState() == SoTouchEvent::UP){
        examiner->m_activeViewingMode = NOTHING;
      } else if (touchevent->getState() == SoTouchEvent::MOVE && examiner->m_activeViewingMode == PAN){
        examiner->m_interactor->translate(node->getAction()->getViewportRegion().normalize(SbVec2s(touchevent->getDisplacement() * 0.5f)), node->getAction()->getViewportRegion());
      }

    } else if (touchevent->getTouchManager()->getFingerNumber() == 3){

    }

    node->getAction()->setHandled();

  }

};

void SceneExaminer::zoomCB(void * userdata, SoEventCallback * node){
  SceneExaminer * examiner = (SceneExaminer *) userdata;
  SoScaleGestureEvent * scaleEvent = (SoScaleGestureEvent*) node->getEvent();
  float delta = scaleEvent->getDeltaScaleFactor();

  if(scaleEvent != NULL)
  {

    const SbViewportRegion &region = node->getAction()->getViewportRegion();
    float aspect = region.getViewportAspectRatio();

    SbViewVolume myViewVolume = examiner->m_perspCamera->getViewVolume( aspect );
    if ( aspect < 1 )
      myViewVolume.scale( 1 / aspect );

    SbVec2f normPosition = node->getAction()->getViewportRegion().normalize(SbVec2s(scaleEvent->getPositionFloat()));

    SbVec3f zoomCenter = myViewVolume.getPlanePoint(examiner->m_perspCamera->nearDistance.getValue(), normPosition);
    SbVec3f eyePosition = myViewVolume.getPlanePoint(examiner->m_perspCamera->nearDistance.getValue(), SbVec2f(0.5f, 0.5f));
    SbVec3f zoomCenterToEyePosition = eyePosition - zoomCenter;

    // Camera is moved to the new position according to the scale factor
    examiner->m_perspCamera->position.setValue(examiner->m_perspCamera->position.getValue() - zoomCenterToEyePosition + (zoomCenterToEyePosition / delta));


    float focalDistance = examiner->m_perspCamera->focalDistance.getValue();
    float newFocalDist = focalDistance / delta;

    // finally reposition the camera
    SbRotation camRot = examiner->m_perspCamera->orientation.getValue();
    SbMatrix mx;
    camRot.getValue(mx);
    SbVec3f forward = SbVec3f(-mx[2][0], -mx[2][1], -mx[2][2]);

    examiner->m_perspCamera->position.setValue(examiner->m_perspCamera->position.getValue() + (focalDistance - newFocalDist) * forward);
    examiner->m_perspCamera->focalDistance.setValue(newFocalDist);

    examiner->m_interactor->adjustClippingPlanes(examiner, node->getAction()->getViewportRegion());
  }
  
};

void SceneExaminer::rotateCB(void * userdata, SoEventCallback * node){
  SceneExaminer * examiner = (SceneExaminer *) userdata;
  SoRotateGestureEvent * rotateEvent = (SoRotateGestureEvent*) node->getEvent();
  examiner->m_interactor->setRotationAxis(SbVec3f(0, 0, 1.0f));
  examiner->m_interactor->setRotationCenter(examiner->m_interactor->projectToPlane(rotateEvent->getNormalizedPosition(node->getAction()->getViewportRegion()), examiner->m_interactor->getCamera()->getViewVolume().getNearDist(), node->getAction()->getViewportRegion()));
  examiner->m_interactor->rotate(- rotateEvent->getDeltaRotation() * 0.5f);
  examiner->m_activeViewingMode = PAN;
};

void SceneExaminer::doubleTapCB(void * userdata, SoEventCallback * node){
  SceneExaminer * examiner = (SceneExaminer *) userdata;

  // examiner->viewAll(node->getAction()->getViewportRegion());

};

void SceneExaminer::longTapCB(void * userdata, SoEventCallback * node){
  SceneExaminer * examiner = (SceneExaminer *) userdata;

};

void SceneExaminer::setPickingMode(bool value){
  if (value){
    m_activeViewingMode = PICK;
  }else{
    m_activeViewingMode = NOTHING;
  }
}

bool SceneExaminer::isPickingMode(){
  return m_activeViewingMode == PICK ? true : false;
}

SoCamera* SceneExaminer::getCamera(){
  return m_perspCamera;
}

SceneExaminer::SceneExaminer(){

  m_perspCamera = new SoPerspectiveCamera();
  m_perspCamera->ref();

  this->addChild(m_perspCamera);

  m_interactor = SoCameraInteractor::getNewInstance(m_perspCamera);

  m_eventCallBack = new SoEventCallback();
  m_eventCallBack->ref();
  this->addChild(m_eventCallBack);

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
  m_eventCallBack->addEventCallback(SoLongTapGestureEvent::getClassTypeId(), &longTapCB, this);

  m_switch = new SoSwitch();
  m_switch->ref();

  SoTransformSeparator * transformSeparator = new SoTransformSeparator();
  SoRotation* headlightRot = new SoRotation();
  headlightRot->rotation.connectFrom(&m_perspCamera->orientation);
  transformSeparator->addChild(headlightRot);
  transformSeparator->addChild(new SoDirectionalLight());
  m_switch->addChild(transformSeparator);

  this->addChild(m_switch);

  m_activeViewingMode = NOTHING;

  setHeadLightMode(true);

  m_isButton1Down = false;
  m_isButton2Down = false;
  m_isButton3Down = false;
}

void SceneExaminer::setHeadLightMode(bool value){
  if (value){
    m_switch->whichChild = 0;
  }else{
    m_switch->whichChild = -1;
  }

  m_headLightMode = value;
}

bool SceneExaminer::isHeadLightMode(){
  return m_headLightMode;
}

SceneExaminer::~SceneExaminer(){
  m_switch->unref();
  m_eventCallBack->unref();
  m_perspCamera->unref();
}
