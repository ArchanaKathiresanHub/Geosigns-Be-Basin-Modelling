#ifndef _SceneExaminer_
#define _SceneExaminer_

#include "SceneInteractor.h" 

#include <Inventor/events/SoMouseButtonEvent.h>
#include <Inventor/events/SoLocation2Event.h>

#include <memory>
#include <functional>

class SoMouseWheelEvent;
class SoKeyboardEvent;
class SoLocation2Event;
class SoScaleGestureEvent;
class SoEvent;
class SoTouchEvent;
class SoRotateGestureEvent;
class SceneGraphManager;

/**
* Tool class for easily building a basic interactive OpenInventor application 
* without using existing viewer classes.
*
* The SceneExaminer is a simple extension of the SoSeparator node that allows
* handling of Open Inventor events and interaction with a camera similar to
* the behavior of the classic Open Inventor viewer class SoXtExaminerViewer.
*
* The SceneExaminer is not comparable with a classic OpenInventor viewer as 
* it does not provide any GUI (no buttons, no popup menu) and fewer interactive
* features (no animation, no seek-to-point). However it does provide a touch 
* event handler that allows manipulating a scene on a touch device.
*
* The SceneExaminer uses an instance of SoCameraInteractor 
* to manipulate the camera in response to OpenInventor events.
*
* @note
* The SceneExaminer needs a component that builds OpenInventor events (SoEvent) 
* from native system events. Such behavior is provided by the SoEventBuilder class.
*
* @note
* SceneExaminer is not part of the Open Inventor API, but is provided as source code 
* in the demo folders. Its current API may change in future versions.
*
* @USAGE
*
* - With a mouse:
*    - @B Left Mouse: @b Rotate the scene.
*    - @B Left Mouse + Ctrl or Middle Mouse: @b Pan the scene.
*    - @B Left Mouse + Ctrl + Shift: @b Zoom in/out.
*    - @B Mouse Wheel: @b Zoom in/out (zoom center is the mouse cursor location).
*    - @B Escape key: @b Enable/Disable picking mode.
*    .
* @BR
*
* - With a touchscreen:
*    - @B 1 finger: @b Rotate the scene.
*    - @B 2 fingers: @b Rotate the scene on the screen plane, zoom in/out and pan (rotation and zoom center are located between the two fingers).
*    - @B Double tap: @b View all the scene.
*
* \htmlonly </UL> \endhtmlonly
*/
class SceneExaminer : public SceneInteractor
{
public:
  
  /**
  * Interaction Mode (viewing or picking)
  */
  enum InteractionMode
  {
    VIEWING,
    PICKING,
    FENCE_EDITING
  };

  explicit SceneExaminer(std::shared_ptr<SceneGraphManager> mgr);

  virtual ~SceneExaminer();

  /**
  * Set interaction mode to viewing or picking. Default is VIEWING.
  */
  void setInteractionMode(SceneExaminer::InteractionMode mode);

  /**
  * Returns the current interaction mode.
  */
  SceneExaminer::InteractionMode getInteractionMode();

  void setModeChangedCallback(std::function<void(InteractionMode)> cb);
  void setFenceAddedCallback(std::function<void(int)> cb);

protected:

  virtual void mouseWheelMoved( SoMouseWheelEvent* wheelEvent, SoHandleEventAction* action );
  virtual void mouseMoved( SoLocation2Event* mouseEvent, SoHandleEventAction* action );
  virtual void mousePressed( SoMouseButtonEvent* mouseEvent, SoHandleEventAction* action );
  virtual void mouseReleased( SoMouseButtonEvent* mouseEvent, SoHandleEventAction* action );
  virtual void keyPressed( SoKeyboardEvent* keyEvent, SoHandleEventAction* action );
  virtual void keyReleased( SoKeyboardEvent* keyEvent, SoHandleEventAction* action );
  virtual void touch( SoTouchEvent* touchEvent, SoHandleEventAction* action );
  virtual void zoom( SoScaleGestureEvent* scaleEvent, SoHandleEventAction* action );
  virtual void rotate( SoRotateGestureEvent* rotateEvent, SoHandleEventAction* action );

private:

  bool m_isButton1Down;
  bool m_isButton2Down;
  bool m_isTouchOrbitActivated;
  InteractionMode m_activeMode;
  SbVec2f m_mousePositionNorm;
  int m_mouseWheelDelta;

  SoMouseButtonEvent m_touchMouseButtonEvent;
  SoLocation2Event m_touchLocation2Event;

  // given an SoTouchEvent convert it into MouseButton or Location2Event
  SoEvent* convertTouchEvent( SoTouchEvent* touchEvent );

  std::shared_ptr<SceneGraphManager> m_scenegraph;

  std::function<void(int)> m_fenceAddedCallback;
  std::function<void(InteractionMode)> m_modeChangedCallback;

  int m_currentFenceId;
  std::vector<SbVec3f> m_fencePoints;
};

#endif // _SceneExaminer_
