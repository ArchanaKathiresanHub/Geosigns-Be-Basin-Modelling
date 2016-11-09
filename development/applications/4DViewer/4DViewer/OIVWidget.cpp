//
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "OIVWidget.h"

#include <QtGui/QMouseEvent>

#include <Inventor/SoSceneManager.h>
#include <Inventor/SoOffscreenRenderer.h>
#include <Inventor/ViewerComponents/SoRenderAreaCore.h>
#include <Inventor/devices/SoGLContext.h>
#include <Inventor/events/SoMouseButtonEvent.h>
#include <Inventor/events/SoLocation2Event.h>
#include <Inventor/events/SoKeyboardEvent.h>

namespace
{
  void fillEvent(const QMouseEvent& event, SoEvent& oivEvent, int viewportHeight)
  {
    oivEvent.setPosition(SbVec2s((short)event.x(), (short)(viewportHeight - 1 - event.y())));

    auto mod = event.modifiers();
    oivEvent.setShiftDown((mod & Qt::ShiftModifier) != 0);
    oivEvent.setCtrlDown((mod & Qt::ControlModifier) != 0);
    oivEvent.setAltDown((mod & Qt::AltModifier) != 0);

    oivEvent.setButton1Down((event.buttons() & Qt::LeftButton) != 0);
  }

  SoMouseButtonEvent makeButtonEvent(QMouseEvent* event, int viewportHeight)
  {
    SoMouseButtonEvent oivEvent;
    fillEvent(*event, oivEvent, viewportHeight);

    auto button = event->button();
    SoMouseButtonEvent::Button oivButton = SoMouseButtonEvent::ANY;

    switch (button)
    {
    case Qt::LeftButton: oivButton = SoMouseButtonEvent::BUTTON1; break;
    case Qt::MiddleButton: oivButton = SoMouseButtonEvent::BUTTON2; break;
    case Qt::RightButton: oivButton = SoMouseButtonEvent::BUTTON3; break;
    }

    oivEvent.setButton(oivButton);

    return oivEvent;
  }

  SoLocation2Event makeLocationEvent(QMouseEvent* event, int viewportHeight)
  {
    SoLocation2Event oivEvent;
    fillEvent(*event, oivEvent, viewportHeight);

    oivEvent.setEventSource(SoLocation2Event::MOUSE_MOVE);

    return oivEvent;
  }

  SoKeyboardEvent makeKeyboardEvent(QKeyEvent* event)
  {
    SoKeyboardEvent oivEvent;

    SoKeyboardEvent::Key key = SoKeyboardEvent::ANY;

    switch (event->key())
    {
    case Qt::Key_Escape: key = SoKeyboardEvent::ESCAPE; break;
    case Qt::Key_Control: key = SoKeyboardEvent::LEFT_CONTROL; break;
    default:
      if (event->key() >= Qt::Key_A && event->key() <= Qt::Key_Z)
        key = (SoKeyboardEvent::Key)(SoKeyboardEvent::A + event->key() - Qt::Key_A);
      break;
    }

    oivEvent.setKey(key);

    return oivEvent;
  }

  void requestRedraw(void* userData)
  {
    OIVWidget* widget = reinterpret_cast<OIVWidget*>(userData);
    widget->updateGL();
  }

  void renderCallback(void* userData, SoSceneManager* /*mgr*/)
  {
    SoRenderAreaCore* area = reinterpret_cast<SoRenderAreaCore*>(userData);
    area->render();
  }
}

void OIVWidget::initializeGL()
{
  m_glcontext = SoGLContext::getCurrent(true);
  if(!m_glcontext.ptr())
    return;

  m_glcontext->setNoGLContextDelete();
  m_glcontext->bind();
  
  m_renderArea = new SoRenderAreaCore(m_glcontext.ptr());
  m_renderArea->setRedrawRequestCallback(requestRedraw, this);
  m_renderArea->getSceneManager()->setRenderCallback(renderCallback, m_renderArea.ptr());
}

void OIVWidget::resizeGL(int width, int height)
{
  SbViewportRegion vpregion((short)width, (short)height);
  m_renderArea->setViewportRegion(vpregion);
}

void OIVWidget::paintGL()
{
  m_glcontext->bind();

  const bool clearWindow = true;
  const bool clearDepthBuffer = true;
  auto status = m_renderArea->render(clearWindow, clearDepthBuffer);

  // swapBuffers is called automatically after this function
}

void OIVWidget::mouseMoveEvent(QMouseEvent* event)
{
  auto oivEvent = makeLocationEvent(event, height());
  
  if (m_renderArea->processEvent(&oivEvent))
    updateGL();
}

void OIVWidget::mousePressEvent(QMouseEvent* event)
{
  auto oivEvent = makeButtonEvent(event, height());
  oivEvent.setState(SoButtonEvent::DOWN);

  if (m_renderArea->processEvent(&oivEvent))
    updateGL();
}

void OIVWidget::mouseReleaseEvent(QMouseEvent* event)
{
  auto oivEvent = makeButtonEvent(event, height());
  oivEvent.setState(SoButtonEvent::UP);

  if (m_renderArea->processEvent(&oivEvent))
    updateGL();
}

void OIVWidget::keyPressEvent(QKeyEvent* event)
{
  auto oivEvent = makeKeyboardEvent(event);

  if (oivEvent.getKey() != SoKeyboardEvent::ANY)
  {
    oivEvent.setState(SoButtonEvent::DOWN);
    if (m_renderArea->processEvent(&oivEvent))
      updateGL();
  }
}

void OIVWidget::keyReleaseEvent(QKeyEvent* event)
{
  auto oivEvent = makeKeyboardEvent(event);

  if (oivEvent.getKey() != SoKeyboardEvent::ANY)
  {
    oivEvent.setState(SoButtonEvent::UP);
    if (m_renderArea->processEvent(&oivEvent))
      updateGL();
  }
}
  
OIVWidget::OIVWidget(QWidget* parent, Qt::WindowFlags flags)
  : QGLWidget(parent, nullptr, flags)
{
  setFocusPolicy(Qt::StrongFocus);
}

void OIVWidget::saveSnapshot(const QString& filename)
{
  SbViewportRegion vpregion(width(), height());
  vpregion.setPixelsPerInch(72.f);

  SoOffscreenRenderer renderer(vpregion);
  renderer.setBackgroundColor(SbColor(0.f, 0.f, 0.f));

  SoNode* root = m_renderArea->getSceneGraph();
  if (renderer.render(root))
  {
	SbString file(filename.toStdString());
	const float quality = 0.75f;
	renderer.writeToJPEG(file, quality);
  }
}

void OIVWidget::setSceneGraph(SoNode* root)
{
  m_renderArea->setSceneGraph(root);
}
