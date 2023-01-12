//
// Copyright (C) 2012 - 2023 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "GuiTestUtilities.h"

#include <QFileDialog>
#include <QInputDialog>
#include <QLineEdit>

namespace guiUtilities
{

void openExistingFileInDialog(const QString& fileName, const bool accept)
{
   if (auto* dialog = topLevelWidget<QFileDialog*>())
   {
      QFile file(fileName);
      if(file.exists())
      {
         dialog->hide();
         dialog->selectFile(fileName);
         dialog->show();

         accept ? QTest::keyClick(dialog, Qt::Key_Enter, Qt::NoModifier) :
                  QTest::keyClick(dialog, Qt::Key_Escape, Qt::NoModifier);
      }
      else
      {
         QTest::keyClick(dialog, Qt::Key_Escape, Qt::NoModifier);
      }
   }
}

void openNewFileInDialog(const QString& fileName, const bool accept)
{
   if (auto* dialog = topLevelWidget<QFileDialog*>())
   {
      dialog->hide();
      dialog->setOption(QFileDialog::DontUseNativeDialog);
      QLineEdit* fileNameEdit = dialog->findChild<QLineEdit*>(QStringLiteral("fileNameEdit"));
      fileNameEdit->setText(fileName);
      dialog->show();

      accept ? QTest::keyClick(dialog, Qt::Key_Enter, Qt::NoModifier) :
               QTest::keyClick(dialog, Qt::Key_Escape, Qt::NoModifier);
   }
}

void setIntInInputDialog(const int value, const bool accept)
{
   if (auto* dialog = topLevelWidget<QInputDialog*>())
   {
      dialog->hide();
      dialog->setIntValue(value);
      dialog->show();
      accept ? QTest::keyClick(dialog, Qt::Key_Enter, Qt::NoModifier) :
               QTest::keyClick(dialog, Qt::Key_Escape, Qt::NoModifier);
   }
}

} //namespace guiUtilities
