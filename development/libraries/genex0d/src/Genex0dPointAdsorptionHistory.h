//
// Copyright (C) 2013-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Genex0dPointAdsorptionHistory class gets source rock (SR) input data, and computes SR properties.
#pragma once

#include "PointAdsorptionHistory.h"

namespace genex0d
{

class Genex0dInputData;

class Genex0dPointAdsorptionHistory : public DataAccess::Interface::PointAdsorptionHistory
{

public :
  Genex0dPointAdsorptionHistory(DataAccess::Interface::ProjectHandle & projectHandle,
                                const Genex0dInputData & inData);
  virtual ~Genex0dPointAdsorptionHistory();
};

} // namespace genex0d
