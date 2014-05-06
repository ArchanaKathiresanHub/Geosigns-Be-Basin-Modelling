#include "GLInfoDialog.h"
#include <Inventor\sys\SoGL.h>
#include <Inventor\devices\SoGLContext.h>

void GLInfoDialog::setupInfo()
{
  SoGLContext* context = new SoGLContext(true);
  context->bind();

  const char* vendorStr = (const char*)glGetString(GL_VENDOR);
  const char* versionStr = (const char*)glGetString(GL_VERSION);
  const char* rendererStr = (const char*)glGetString(GL_RENDERER);

  QString extensionsStr = (const char*)glGetString(GL_EXTENSIONS);

  m_dlg.labelVendor->setText(vendorStr);
  m_dlg.labelVersion->setText(versionStr);
  m_dlg.labelRenderer->setText(rendererStr);
  m_dlg.listExtensions->addItems(extensionsStr.split(" "));

  context->unbind();
  context->unref();
}

GLInfoDialog::GLInfoDialog(QWidget* parent, Qt::WindowFlags f)
  : QDialog(parent, f)
{
  m_dlg.setupUi(this);

  setupInfo();
}