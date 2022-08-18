//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "model/UAResultsTargetsData.h"

#include <functional>
#include <algorithm>

namespace casaWizard
{

namespace ua
{

namespace
{
bool sortingFunction(const UAResultsTargetData& d0, const UAResultsTargetData& d1, int colIdx)
{
   switch (colIdx)
   {
   case 0:
      return d0.locationName < d1.locationName;
   case 1:
      return d0.x < d1.x;
   case 2:
      return d0.y < d1.y;
   case 3:
      return d0.z < d1.z;
   case 4:
      return d0.surfaceName < d1.surfaceName;
   default:
      int propIdx = colIdx-5;
      if (d0.propertyStates.size() > propIdx && d1.propertyStates.size() > propIdx)
      {
         return d0.propertyStates[propIdx] > d1.propertyStates[propIdx];
      }
      return false;
   }
}
}

UAResultsTargetsData::UAResultsTargetsData(const QVector<const PredictionTarget*> predictionTargets,
                                           const QVector<QString>& targetProperties):
   m_sortedByCol(-1)
{
   setData(predictionTargets,targetProperties);
}

UAResultsTargetsData::UAResultsTargetsData():
   m_sortedByCol(-1)
{}

void UAResultsTargetsData::setData(const QVector<const PredictionTarget*> predictionTargets,
                                   const QVector<QString>& targetProperties)
{
   m_sortedByCol = -1;
   m_targetData.resize(0);
   m_targetProperties = targetProperties;

   QVector<bool> propertyStates(m_targetProperties.size(),false);

   int targetIdx = 0;
   for (const auto target : predictionTargets)
   {
      m_targetData.push_back(UAResultsTargetData(*target));

      const QVector<QString>& properties = target->properties();

      for (int i = 0; i < m_targetProperties.size(); i++)
      {
         const auto& p = m_targetProperties[i];
         if (properties.contains(p))
         {
            propertyStates[i] = true;
         }
         else
         {
            propertyStates[i] = false;
         }
      }

      m_targetData.back().propertyStates = propertyStates;
      m_targetData.back().targetIndex = targetIdx;
      targetIdx++;
   }
}

int UAResultsTargetsData::targetIndex(int rowIdx) const
{
   return m_targetData.at(rowIdx).targetIndex;
}

int UAResultsTargetsData::rowIndex(int targetIdx) const
{
   for (int i = 0; i < m_targetData.size(); i++)
   {
      if (m_targetData[i].targetIndex == targetIdx)
      {
         return i;
      }
   }
   return 0;
}

void UAResultsTargetsData::sortData(int column)
{
   using namespace std::placeholders;

   if (m_sortedByCol != column)
   {
      std::sort(m_targetData.begin(), m_targetData.end(), std::bind(sortingFunction,_1,_2,column));
   }
   else
   {
      std::reverse(m_targetData.begin(), m_targetData.end()); //If already sorted by this column, reverse order
   }
   m_sortedByCol = column;
}

const QVector<UAResultsTargetData>& UAResultsTargetsData::targetData() const
{
   return m_targetData;
}

} //ua
} //casaWizard
