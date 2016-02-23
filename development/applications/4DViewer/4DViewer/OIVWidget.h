#ifndef OIVWIDGET_H_INCLUDED
#define OIVWIDGET_H_INCLUDED

#include <QtOpenGL/QGLWidget>
#include <Inventor/misc/SoRef.h>

class SceneExaminer;
class SoGLContext;
class SoRenderAreaCore;
class SoNode;

class OIVWidget : public QGLWidget
{
  SoRef<SoGLContext> m_glcontext;
  SoRef<SoRenderAreaCore> m_renderArea;

 protected:

  virtual void initializeGL();

  virtual void resizeGL(int width, int height);

  virtual void paintGL();

  virtual void mouseMoveEvent(QMouseEvent* event);

  virtual void mousePressEvent(QMouseEvent* event);

  virtual void mouseReleaseEvent(QMouseEvent* event);

  virtual void keyPressEvent(QKeyEvent* event);

  virtual void keyReleaseEvent(QKeyEvent* event);
  
 public:

  OIVWidget(QWidget* parent=0, Qt::WindowFlags f=0);

  void setSceneGraph(SoNode* root);
};

#endif // OIVWIDGET_H_INCLUDED
