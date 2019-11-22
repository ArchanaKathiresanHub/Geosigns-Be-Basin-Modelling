//
// Copyright (C) 2010-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include <assert.h>

#include <iostream>
#include <sstream>

#include "ReservoirOptions.h"
#include "database.h"
#include "cauldronschemafuncs.h"
#include "ObjectFactory.h"
#include "ProjectHandle.h"

using namespace std;
using namespace DataAccess;
using namespace Interface;

ReservoirOptions::ReservoirOptions (ProjectHandle& projectHandle, database::Record * record) : DAObject (projectHandle, record)
{
}

ReservoirOptions::~ReservoirOptions (void)
{
}

/// If legacy use reservoir-specific options (ReservoirIoTbl), otherwise global reservoir options (ReservoirOptionsIoTbl)
bool ReservoirOptions::isBiodegradationOn (void) const
{
   return database::getBioDegradInd (m_record) == 1;
}

bool ReservoirOptions::isOilToGasCrackingOn (void) const
{
   return database::getOilToGasCrackingInd (m_record) == 1;
}

bool ReservoirOptions::isDiffusionOn (void) const
{
   return database::getDiffusionInd (m_record) == 1;
}

double ReservoirOptions::getTrapCapacity (void) const
{
   return database::getTrapCapacity (m_record);
}

bool ReservoirOptions::isBlockingOn (void) const
{
   return database::getBlockingInd (m_record) == 1;
}

double ReservoirOptions::getBlockingPermeability (void) const
{
   return database::getBlockingPermeability (m_record);
}

double ReservoirOptions::getBlockingPorosity (void) const
{
   return database::getBlockingPorosity (m_record);
}

double ReservoirOptions::getMinOilColumnHeight (void) const
{
   return database::getMinOilColumnHeight (m_record);
}

double ReservoirOptions::getMinGasColumnHeight (void) const
{
   return database::getMinGasColumnHeight (m_record);
}
