//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

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
