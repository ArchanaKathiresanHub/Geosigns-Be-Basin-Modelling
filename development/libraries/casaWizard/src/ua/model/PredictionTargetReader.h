//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <QStringList>

namespace casaWizard
{

namespace ua
{
class PredictionTarget;
class PredictionTargetReader
{
public:
   static PredictionTarget* readTarget(const int version, const QStringList& parameters);
};

}

}
