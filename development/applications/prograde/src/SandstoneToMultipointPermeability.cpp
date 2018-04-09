//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "SandstoneToMultipointPermeability.h"

//std
#include <cmath>
#include <map>

//utilities
#include "MapExtensions.h"

using namespace Prograde;

SandstoneToMultipointPermeability::SandstoneToMultipointPermeability(const Data& data):
   m_permAnisotropy(data.m_permAnisotropy),
   m_depositionalPerm(data.m_depositionalPerm),
   m_permSandClayPercentage(data.m_permSandClayPercentage),
   m_depositionalPoro(data.m_depositionalPoro),
   m_minPoro(data.m_minPoro) {}

//---------------------------------------------------------------------------------------------------

void SandstoneToMultipointPermeability::compute(std::vector<double>& modelPrms,
                                                std::vector<double>& mpPor,
                                                std::vector<double>& mpPerm) const
{
   modelPrms.clear();
   mpPor.clear();
   mpPerm.clear();

   // Add anisotropy
   modelPrms.push_back(m_permAnisotropy);

   // Add porosity-permeability points
   // only 4 points are needed
   //
   // ^
   // |               c----------d
   // |              /
   // perm.         /
   // |            /
   // |a----------b
   // |
   // 0----------poro.---------->100
   //
   // use a map so points are not duplicated
   Utilities::doubleKeyMap points;
   // point a: 0% porosity
   const double exponent = (0.12+0.02*m_permSandClayPercentage)*(m_minPoro - m_depositionalPoro);
   const double minPerm = m_depositionalPerm*std::pow(10, exponent);
   points[0]=std::log10(minPerm);
   // point b: minimum porosity
   points[m_minPoro]=std::log10(minPerm);
   // point c: depositional porosity
   const double permLog = std::log10(m_depositionalPerm);
   points[m_depositionalPoro]=permLog;
   // point d: 100% porosity
   points[100]=permLog;
   //populate vectors
   for (auto const& pair : points)
   {
      mpPor.push_back(pair.first);
      mpPerm.push_back(pair.second);
   }
}