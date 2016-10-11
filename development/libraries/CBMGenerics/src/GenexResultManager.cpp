//
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "stdafx.h"

#include "GenexResultManager.h"

namespace CBMGenerics
{

GenexResultManager::GenexResultManager()
{
   int i;

   const std::string tempResultNames[] =
   {
      "InstantaneousExpulsionApi",         "CumulativeExpulsionApi",         "InstantaneousExpulsionCondensateGasRatio", "CumulativeExpulsionCondensateGasRatio",
      "InstantaneousExpulsionGasOilRatio", "CumulativeExpulsionGasOilRatio", "InstantaneousExpulsionGasWetness",         "CumulativeExpulsionGasWetness",
      "InstantaneousExpulsionAromaticity", "CumulativeExpulsionAromaticity", "KerogenConversionRatio", 
      "OilGeneratedCumulative",          "OilGeneratedRate",         "OilExpelledCumulative",         "OilExpelledRate",
      "HcGasGeneratedCumulative",        "HcGasGeneratedRate",       "HcGasExpelledCumulative",       "HcGasExpelledRate",
      "DryGasGeneratedCumulative",       "DryGasGeneratedRate",      "DryGasExpelledCumulative",      "DryGasExpelledRate",
      "WetGasGeneratedCumulative",       "WetGasGeneratedRate",      "WetGasExpelledCumulative",      "WetGasExpelledRate", 
      "SbearingHCsGeneratedCumulative",  "SbearingHCsGeneratedRate", "SbearingHCsExpelledCumulative", "SbearingHCsExpelledRate", 
       ""
   };
   for(i = 0; tempResultNames[i] != ""; ++i)  
   {
      m_ResultNames[i] =  tempResultNames[i];
   }
   //need to be updated

      // "°API", "°API", " ", " ",
      // " API", " API", " ", " ",

   std::string tempResultUnits[] =
   {
      "°API", "°API", " ", " ",
      " "   , " "   , " ", " ",
      " "   , " "   , " ",                 
      "kg/m2",          "kg/m2/Ma",       "kg/m2",                  "kg/m2/Ma",
      "kg/m2",          "kg/m2/Ma",       "kg/m2",                  "kg/m2/Ma",
      "kg/m2",          "kg/m2/Ma",       "kg/m2",                  "kg/m2/Ma",
      "kg/m2",          "kg/m2/Ma",       "kg/m2",                  "kg/m2/Ma",
      "kg/m2",          "kg/m2/Ma",       "kg/m2",                  "kg/m2/Ma",
       ""
   };

#if 0
   // Is this a fix for the compiler warning when using the Intel compiler?
   // Add degree sign
   tempResultUnits [0][0]=char ( 167 );
   tempResultUnits [1][0]=char ( 167 );
#endif 

   for(i = 0; tempResultUnits[i] != ""; ++i)  
   {
      m_ResultUnits[i] =  tempResultUnits[i];
   }
   for(i = 0; i < NumberOfResults; ++i)
   {
      m_ResultRequired[i] = false;
   } 
}
void GenexResultManager::SetResultToggleByResId(const int &theId, const bool &on_off)
{
   if(theId >= 0 && theId < NumberOfResults)
   {
      m_ResultRequired[theId] =  on_off;
   }
}
void GenexResultManager::SetResultToggleByName(const std::string &theResultName, const bool &on_off)
{  
   int i;
   for(i = 0; i < NumberOfResults; ++i)
   {
      if(theResultName == m_ResultNames[i] )
      {
         m_ResultRequired[i] =  on_off;
      }
   }
}

int GenexResultManager::getResultId ( const std::string& name ) const {  

   int i;

   for(i = 0; i < NumberOfResults; ++i)
   {
      if( name == m_ResultNames[i] )
      {
         return i;
      }
   }

   return -1;
}

bool GenexResultManager::IsResultRequired(const int &theId) const
{
   bool ret = false;
   if(theId >= 0 && theId < NumberOfResults)
   {
      ret = m_ResultRequired[theId];
   }
   return ret;
}

bool GenexResultManager::IsSulphurResult(const std::string& name) const
{
   return (getResultId(name) >= SbearingHCsGeneratedCum && getResultId(name) < NumberOfResults);
}

std::string GenexResultManager::GetResultUnit(const int &ResultId)
{
   std::string ret;
   if(ResultId >= 0 && ResultId < NumberOfResults)
   {
      ret = m_ResultUnits[ResultId];
   }
   return ret;
}
std::string GenexResultManager::GetResultName(const int &ResultId)
{
   std::string ret;
   if(ResultId >= 0 && ResultId < NumberOfResults)
   {
      ret = m_ResultNames[ResultId];
   }
   return ret;
}
GenexResultManager::~GenexResultManager()
{
    
}
GenexResultManager & GenexResultManager::getInstance()
{
   static GenexResultManager theManager;
   return theManager;
}

}

