#ifndef _SceneInteractor_
#define _SceneInteractor_


#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/SbViewportRegion.h>

class SoMouseButtonEvent;
class SoMouseWheelEvent;
class SoKeyboardEvent;
class SoLocation2Event;
class SoScaleGestureEvent;
class SoTouchEvent;
class SoRotateGestureEvent;
class SoCamera;
class SoEventCallback;
class SoCameraInteractor;
class SoRotation;
class SoSwitch;

/**
* Tool class for easily building a basic OpenInventor application 
* without using existing viewer classes.
*
* The SceneInteractor is a simple extension of the SoSeparator node that allows
* handling of Open Inventor events. This class should be overridden as it provides 
* only empty event handlers.
* 
* This node is intended to be used as the root of a scene graph. 
* The SceneInteractor is a custom SoSeparator whose children are:
*  - A camera (switch between perspective and orthographic camera).
*
*  - A headlight.
*
*  - An SoEventCallback node that handles keyboard, mouse and touch events and
*    provides the camera and headlight manipulations like panning, zooming 
*    and orbiting.
*
*  - The application's scene graph, which should be the last child.
*
* The SceneInteractor uses an instance of SoCameraInteractor in order 
* to manipulate the camera in response to OpenInventor events.
*
* Class diagram of the SceneInteractor showing the relationship between the 
* SoEventCallback, the SoCamera, the SoDirectionalLight (used as
* headlight) and the SoCameraInteractor.
* @IMAGE SceneInteractor.png
*
* Detail of the scene graph rooted by a SceneInteractor:
* @IMAGE SceneInteractorSceneGraph.png
*
* @note
* SceneInteractor is not part of the Open Inventor API, but is provided as source code 
* in the demo folders. Its current API may change in future versions.
*/
class SceneInteractor : public SoSeparator
{
public:

  /** Constructor */
  SceneInteractor();

  /** Destructor */
  virtual ~SceneInteractor();

  /**
   * Move the camera to view the scene defined by the given path. 
   * Equivalent to calling the SoCamera method viewAll(). Camera position is changed, but not orientation. 
   */
  void viewAll(const SbViewportRegion &viewport);

  /**
  * Returns the current camera interactor.
  */
  SoCameraInteractor* getCameraInteractor();

  /**
  * Type of camera (perspective or orthographic)
  */
  enum CameraMode
  {
    PERSPECTIVE,
    ORTHOGRAPHIC
  };

  /**
  * Set camera to perspective or orthographic.
  */
  void setCameraMode(SceneInteractor::CameraMode mode);

  /**
  * Returns the current camera mode.
  */
  SceneInteractor::CameraMode getCameraMode();

  /**
  * Enable or disable headlight.
  */
  void enableHeadLight(bool enabled);

  /**
  * Returns if headlight is enabled.
  */
  bool isHeadLightEnabled();

protected:
  SoRef<SoEventCallback> m_eventCallBack;
  SoCameraInteractor* m_cameraInteractor;

  virtual void mouseWheelMoved( SoMouseWheelEvent* wheelEvent, SoHandleEventAction* action );
  virtual void mouseMoved( SoLocation2Event* mouseEvent, SoHandleEventAction* action );
  virtual void mousePressed( SoMouseButtonEvent* mouseEvent, SoHandleEventAction* action );
  virtual void mouseReleased( SoMouseButtonEvent* mouseEvent, SoHandleEventAction* action );
  virtual void keyPressed( SoKeyboardEvent* keyEvent, SoHandleEventAction* action );
  virtual void keyReleased( SoKeyboardEvent* keyEvent, SoHandleEventAction* action );
  virtual void touch( SoTouchEvent* touchEvent, SoHandleEventAction* action );
  virtual void zoom( SoScaleGestureEvent* scaleEvent, SoHandleEventAction* action );
  virtual void rotate( SoRotateGestureEvent* rotateEvent, SoHandleEventAction* action );

  // Events callbacks
  static void mouseMoveCB(void * userdata, SoEventCallback * node);
  static void mouseCB(void * userdata, SoEventCallback * node);
  static void keyboardCB(void * userdata, SoEventCallback * node);
  static void touchCB(void * userdata, SoEventCallback * node);
  static void zoomCB(void * userdata, SoEventCallback * node);
  static void rotateCB(void * userdata, SoEventCallback * node);
  static void doubleTapCB(void * userdata, SoEventCallback * node);
  static void wheelCB(void * userdata, SoEventCallback * node);

private:
  SoRef<SoRotation> m_headlightRot;
  SoRef<SoSwitch> m_cameraSwitch;
  SoRef<SoSwitch> m_headlightSwitch;

  SoRef<SoCameraInteractor> m_perspInteractor;
  SoRef<SoCameraInteractor> m_orthoInteractor;

};

#endif // _SceneInteractor_
