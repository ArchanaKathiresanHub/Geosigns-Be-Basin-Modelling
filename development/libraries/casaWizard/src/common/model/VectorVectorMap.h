//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <vector>

namespace casaWizard
{

class VectorVectorMap
{
public:
   explicit VectorVectorMap(const std::vector<std::vector<double>> mapData);

   void setUndefinedValuesBasedOnReferenceMap(const VectorVectorMap& referenceMap);
   void setDefinedValuesToOne();
   const std::vector<std::vector<double>>& getData() const;
   VectorVectorMap operator+(const VectorVectorMap& lithoMap) const;
   VectorVectorMap operator+(const double rhs) const;
   VectorVectorMap operator*(const double rhs) const;

private:
   bool isUndefined(const double input) const;
   double multiplication(const double input1, const double input2) const;
   double sum(const double input1, const double input2) const;
   bool isMapDataValid(const std::vector<std::vector<double>>& mapData) const;
   bool isIncompatibleForOperations(const VectorVectorMap& rhs) const;

   std::vector<std::vector<double>> m_mapData;
};

} // namespace casaWizard
