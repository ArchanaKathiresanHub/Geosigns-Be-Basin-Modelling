//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "model/TargetInputInfo.h"

namespace casaWizard{
namespace ua {

class TargetInputInfoFromASCII : public TargetInputInfo
{
public:
   TargetInputInfoFromASCII(QString filePath,
                            QVector<bool> surfaceSelectionStates,
                            bool temperatureTargetsSelected = false,
                            bool vreTargetsSelected = false,
                            QString depthInput = "",
                            QString defaultName = "PWS");

   QVector<XYName> getTargetLocations() const override;
   QString getFilePath();
   QString getDefaultName();
   void setFilePath(QString path);
   void setDefaultName(QString name);

private:
   QString m_filePath;
   QString m_defaultName;
};

} //namespace ua
} //namespace casaWizard
