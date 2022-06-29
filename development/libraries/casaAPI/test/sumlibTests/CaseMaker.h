//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <Case.h>
#include <vector>

//Virtual base
class CaseMaker
{
public:
   virtual ~CaseMaker() = default;
   virtual const std::vector<SUMlib::Case>& createCaseCollection(int nDataPointsPerDim, int nDims) = 0;
   virtual const std::vector<SUMlib::Case>& caseCollection() = 0;
   virtual const SUMlib::Case& minCase() = 0;
   virtual const SUMlib::Case& maxCase() = 0;
};

class GridCaseMaker : public CaseMaker
{
   //Create case grid in n dimensions (nDims), with nDataPointsPerDim.
   const std::vector<SUMlib::Case>& createCaseCollection(int nDataPointsPerDim, int nDims);

   const std::vector<SUMlib::Case>& caseCollection() { return m_caseCollection;}
   const SUMlib::Case& minCase(){return m_minCase;}
   const SUMlib::Case& maxCase(){return m_maxCase;}

private:

   //Recursive function to create case grid in n dimensions. A case represents a DoE point.
   static void makeCases(const std::vector<double>& pars, const std::vector<double>& parSingleValue, int nDims, std::vector<SUMlib::Case>& caseCollection);

   std::vector<SUMlib::Case> m_caseCollection;
   SUMlib::Case m_minCase;
   SUMlib::Case m_maxCase;
};

class RandomCaseMaker : public CaseMaker
{
   //Create nDataPointsPerDim*nDims random cases with value ranges between -1 and 1
   const std::vector<SUMlib::Case>& createCaseCollection(int nDataPointsPerDim, int nDims);

   const std::vector<SUMlib::Case>& caseCollection() { return m_caseCollection;}
   const SUMlib::Case& minCase(){return m_minCase;}
   const SUMlib::Case& maxCase(){return m_maxCase;}

private:
   std::vector<SUMlib::Case> m_caseCollection;
   SUMlib::Case m_minCase;
   SUMlib::Case m_maxCase;
};

