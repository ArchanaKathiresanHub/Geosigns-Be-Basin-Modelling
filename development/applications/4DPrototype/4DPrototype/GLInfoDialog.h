#ifndef GLINFODIALOG_H_INCLUDED
#define GLINFODIALOG_H_INCLUDED

#include "GLInfoDialog.ui.h" 

class GLInfoDialog : public QDialog
{
  Ui::Dialog m_dlg;

  void setupInfo();

public:

  GLInfoDialog(QWidget* parent=0, Qt::WindowFlags f=0);

};

#endif
