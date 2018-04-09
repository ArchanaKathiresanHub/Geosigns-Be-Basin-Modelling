//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "MudstoneToMultipointPermeability.h"

//std
#include <cmath>

//utilities
#include "MapExtensions.h"

using namespace Prograde;

const std::vector<double> MudstoneToMultipointPermeability::s_VesSampling = {0, 1, 2, 3, 4, 10, 40, 60, 80, 90, 100, 1000};

//---------------------------------------------------------------------------------------------------

MudstoneToMultipointPermeability::MudstoneToMultipointPermeability(const Data& data):
   m_permAnisotropy(data.m_permAnisotropy),
   m_depositionalPerm(data.m_depositionalPerm),
   m_mudPermSensitivityCoeff(data.m_mudPermSensitivityCoeff),
   m_compactionCoeff(data.m_compactionCoeff),
   m_depositionalPoro(data.m_depositionalPoro),
   m_minPoro(data.m_minPoro),
   m_chemicalCompaction(data.m_chemicalCompaction) {}

//---------------------------------------------------------------------------------------------------

void MudstoneToMultipointPermeability::compute(std::vector<double>& modelPrms,
                                               std::vector<double>& mpPor,
                                               std::vector<double>& mpPerm) const
{
   modelPrms.clear();
   mpPor.clear();
   mpPerm.clear();

   // Add anisotropy
   modelPrms.push_back(m_permAnisotropy);

   // Add porosity-permeability points
   // use a map so points are not duplicated
   Utilities::doubleKeyMap points;
   for( auto& ves : s_VesSampling)
   {
      double porosity;
      if( m_chemicalCompaction ) {
         porosity = (m_depositionalPoro-m_minPoro)*std::exp(-m_compactionCoeff*ves)+m_minPoro;
      }
      else {
         porosity = m_depositionalPoro*std::exp(-m_compactionCoeff*ves);
      }
      const double permeability = m_depositionalPerm*std::pow((ves+s_initialVes)/s_initialVes, -m_mudPermSensitivityCoeff);
      const double permLog = std::log10(permeability);
      points[porosity]=permLog;
   }
   //populate vectors
   for (auto const& pair : points)
   {
      mpPor.push_back(pair.first);
      mpPerm.push_back(pair.second);
   }
}