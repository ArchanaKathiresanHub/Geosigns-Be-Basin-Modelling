//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "VectorVectorMap.h"

#include "logger.h"

#include "ConstantsNumerical.h"

#include <cmath>

namespace casaWizard
{

VectorVectorMap::VectorVectorMap(const std::vector<std::vector<double>> mapData) :
  m_mapData{}
{
   if (isMapDataValid(mapData))
   {
      m_mapData = mapData;
   }
   else
   {
      Logger::log() << "Error in setting VectorVector map data" << Logger::endl();
   }
}

void VectorVectorMap::setUndefinedValuesBasedOnReferenceMap(const VectorVectorMap& referenceMap)
{
   if (isIncompatibleForOperations(referenceMap))
   {
      return;
   }

   for (int i = 0; i < m_mapData.size(); ++i)
   {
      for (int j = 0; j < m_mapData[0].size(); ++j)
      {
         if (isUndefined(referenceMap.getData()[i][j]))
         {
            m_mapData[i][j] = Utilities::Numerical::CauldronNoDataValue;
         }
      }
   }
}

void VectorVectorMap::setDefinedValuesToOne()
{
   for ( auto& row : m_mapData )
   {
      for ( double& value : row )
      {
         if (!isUndefined(value))
         {
            value = 1.0;
         }
      }
   }
}

const std::vector<std::vector<double>>& VectorVectorMap::getData() const
{
   return m_mapData;
}

VectorVectorMap VectorVectorMap::operator+(const VectorVectorMap& lithoMap) const
{
   const std::vector<std::vector<double>>& rhsData = lithoMap.getData();
   if (isIncompatibleForOperations(lithoMap))
   {
      return VectorVectorMap({});
   }

   std::vector<std::vector<double>> resultData;
   for (int i = 0; i < m_mapData.size(); i++)
   {
      std::vector<double> resultRow;
      for (int j = 0; j < m_mapData[0].size(); j++)
      {
         resultRow.push_back(sum(m_mapData[i][j], rhsData[i][j]));
      }
      resultData.push_back(resultRow);
   }

   return VectorVectorMap(resultData);
}

VectorVectorMap VectorVectorMap::operator+(const double rhs) const
{
   std::vector<std::vector<double>> resultData;
   for (int i = 0; i < m_mapData.size(); i++)
   {
      std::vector<double> resultRow;
      for (int j = 0; j < m_mapData[0].size(); j++)
      {
         resultRow.push_back(sum(m_mapData[i][j], rhs));
      }
      resultData.push_back(resultRow);
   }

   return VectorVectorMap(resultData);
}

VectorVectorMap VectorVectorMap::operator*(const double rhs) const
{
   std::vector<std::vector<double>> resultData;
   for (int i = 0; i < m_mapData.size(); i++)
   {
      std::vector<double> resultRow;
      for (int j = 0; j < m_mapData[0].size(); j++)
      {
         resultRow.push_back(multiplication(m_mapData[i][j], rhs));
      }
      resultData.push_back(resultRow);
   }

   return VectorVectorMap(resultData);
}

double VectorVectorMap::sum(const double input1, const double input2) const
{
   if (isUndefined(input1) || isUndefined(input2))
   {
      return Utilities::Numerical::CauldronNoDataValue;
   }
   return input1 + input2;
}

bool VectorVectorMap::isMapDataValid(const std::vector<std::vector<double>>& mapData) const
{
   for (const std::vector<double>& row : mapData)
   {
      if (row.size() != mapData[0].size())
      {
         return false;
      }
   }

   return true;
}

bool VectorVectorMap::isIncompatibleForOperations(const VectorVectorMap& rhs) const
{
   return m_mapData.empty() || m_mapData.size() != rhs.getData().size() || m_mapData[0].size() != rhs.getData()[0].size();
}

double VectorVectorMap::multiplication(const double input1, const double input2) const
{
   if (isUndefined(input1) || isUndefined(input2))
   {
      return Utilities::Numerical::CauldronNoDataValue;
   }
   return input1 * input2;
}

bool VectorVectorMap::isUndefined(const double input) const
{
   return (std::fabs(input - Utilities::Numerical::CauldronNoDataValue) < 1e-5);
}

} // namespace casaWizard
