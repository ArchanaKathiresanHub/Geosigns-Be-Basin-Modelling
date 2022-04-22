//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <QString>

namespace casaWizard
{

class ToDepthConverter
{
public:
   virtual double getDepth(double x, double y, QString name) const = 0;
   virtual ~ToDepthConverter() = default;
};

} // namespace casaWizard
