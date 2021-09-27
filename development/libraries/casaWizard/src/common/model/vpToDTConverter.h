//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <QVector>

namespace casaWizard
{

class VPToDTConverter
{
public:
  VPToDTConverter();
  std::vector<double> convertToDT(const std::vector<double>& velocityVector) const;
};

} // namespace casaWizard

