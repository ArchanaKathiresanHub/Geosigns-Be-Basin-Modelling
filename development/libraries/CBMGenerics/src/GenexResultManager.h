#ifndef GENEXRESULTMANAGER_H
#define GENEXRESULTMANAGER_H

#include "DllExport.h"

#include <string>
#include <map>

namespace CBMGenerics
{

class CBMGENERICS_DLL_EXPORT GenexResultManager
{
public:
   enum CBMGENERICS_DLL_EXPORT ResultId
   {
      ExpulsionApiInst = 0,          ExpulsionApiCum = 1,          ExpulsionCondensateGasRatioInst = 2, ExpulsionCondensateGasRatioCum = 3,
      ExpulsionGasOilRatioInst = 4,  ExpulsionGasOilRatioCum = 5,  ExpulsionGasWetnessInst = 6,         ExpulsionGasWetnessCum = 7,
      ExpulsionAromaticityInst = 8,  ExpulsionAromaticityCum = 9,  KerogenConversionRatio = 10, 
      OilGeneratedCum = 11,          OilGeneratedRate = 12,        OilExpelledCum = 13,         OilExpelledRate = 14,
      HcGasGeneratedCum = 15,        HcGasGeneratedRate = 16,      HcGasExpelledCum = 17,       HcGasExpelledRate = 18,
      DryGasGeneratedCum = 19,       DryGasGeneratedRate = 20,     DryGasExpelledCum = 21,      DryGasExpelledRate = 22,
      WetGasGeneratedCum = 23,       WetGasGeneratedRate = 24,     WetGasExpelledCum = 25,      WetGasExpelledRate = 26, 
      SbearingHCsGeneratedCum = 27,  SbearingHCsGeneratedRate = 28,SbearingHCsExpelledCum = 29, SbearingHCsExpelledRate = 30, 

      NumberOfResults = 31
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
