//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "ErrorMessageBox.h"
#include <QMessageBox>
#include "model/logger.h"

namespace casaWizard
{
namespace messageBox
{
void showErrorMessageBox(QString message)
{
   if (message == "") message = "Unknown error occurred";
   QMessageBox box(QMessageBox::Critical, "Import Error",
                   message,
                   QMessageBox::Ok);
   box.exec();
   Logger::log() << message << Logger::endl();
}
}
}
