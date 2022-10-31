//
// Copyright (C) 2012-2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "view/assets/SacMapPlotOptions.h"

namespace casaWizard
{

namespace sac
{

namespace thermal
{

class ThermalMapPlotOptions: public SacMapPlotOptions
{
   Q_OBJECT
public:
   explicit ThermalMapPlotOptions(QWidget *parent = nullptr);
};

} // thermal

} // sac

} // casaWizard
