//
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef GENEXRESULTMANAGER_H
#define GENEXRESULTMANAGER_H

#include <string>
#include <map>

namespace CBMGenerics
{

class GenexResultManager
{
public:
   enum ResultId
   {
      ExpulsionApiInst = 0,          ExpulsionApiCum = 1,          ExpulsionCondensateGasRatioInst = 2, ExpulsionCondensateGasRatioCum = 3,
      ExpulsionGasOilRatioInst = 4,  ExpulsionGasOilRatioCum = 5,  ExpulsionGasWetnessInst = 6,         ExpulsionGasWetnessCum = 7,
      ExpulsionAromaticityInst = 8,  ExpulsionAromaticityCum = 9,  KerogenConversionRatio = 10, 
      FluxOA1 = 11, FluxOA2 = 12,
      OilGeneratedCum = 13,          OilGeneratedRate = 14,        OilExpelledCum = 15,         OilExpelledRate = 16,
      HcGasGeneratedCum = 17,        HcGasGeneratedRate = 18,      HcGasExpelledCum = 19,       HcGasExpelledRate = 20,
      DryGasGeneratedCum = 21,       DryGasGeneratedRate = 22,     DryGasExpelledCum = 23,      DryGasExpelledRate = 24,
      WetGasGeneratedCum = 25,       WetGasGeneratedRate = 26,     WetGasExpelledCum = 27,      WetGasExpelledRate = 28, 
      SbearingHCsGeneratedCum = 29,  SbearingHCsGeneratedRate = 30,SbearingHCsExpelledCum = 31, SbearingHCsExpelledRate = 32, 
    

      NumberOfResults = 33
   };
   
   void SetResultToggleByResId(const int &theId, const bool &on_off);
   void SetResultToggleByName(const std::string &theResultName, const bool &on_off);
   bool IsResultRequired(const int &theId) const;
   bool IsSulphurResult(const std::string &theResultNameId) const;

   int getResultId ( const std::string& name ) const;
 
   std::string GetResultUnit(const int &ResultId);
   std::string GetResultName(const int &ResultId);

   static GenexResultManager &getInstance();
   ~GenexResultManager();

private:
   int   m_numberOfOutputSpecies;
   std::string m_ResultNames   [NumberOfResults];
   std::string m_ResultUnits   [NumberOfResults];
   bool        m_ResultRequired[NumberOfResults];
  
   GenexResultManager();
   GenexResultManager(const GenexResultManager &in_Manager);
   GenexResultManager & operator =(const GenexResultManager &in_Manager);
   
};

}
#endif
