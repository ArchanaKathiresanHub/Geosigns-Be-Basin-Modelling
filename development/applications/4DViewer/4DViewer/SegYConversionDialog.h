//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef SEGYCONVERSIONDIALOG_H_INCLUDED
#define SEGYCONVERSIONDIALOG_H_INCLUDED

#include "ui_SegYConversionDialog.h" 

#include <Inventor/misc/SoRef.h>

class SoVRSegyFileReader;

class SegYConversionDialog : public QDialog
{
  Q_OBJECT;

  Ui::SegYConversionDialog m_dlg;

  QString m_filename;
  SoRef<SoVRSegyFileReader> m_reader;

private slots:

  void onCancel();

  void onConvert();

public:

  SegYConversionDialog(QWidget* parent=0, Qt::WindowFlags f=0);

  void setInputFileName(const QString& filename);
};

#endif
