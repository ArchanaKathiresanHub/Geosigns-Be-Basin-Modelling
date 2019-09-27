//
// Copyright (C) 2012-2019 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <vector>

namespace WellReader
{

bool readWells( const char* wellFile, std::vector<double>& xWells, std::vector<double>& yWells );

}
