//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "BiodegradationParameters.h"

#include "cauldronschemafuncs.h"

#include <assert.h>
#include <iostream>

using namespace database;

namespace DataAccess 
{ 
   namespace Interface
   {
      BiodegradationParameters::BiodegradationParameters(ProjectHandle* projecthandle, database::Record* record):
      DAObject(projecthandle, record){}

      BiodegradationParameters::~BiodegradationParameters(){}

double BiodegradationParameters::maxBioTemp() const
{
  return database::getMaxBioTemp(m_record);
}

BioConsts BiodegradationParameters::bioConsts() const
{
  vector<double> bioConsts;
  bioConsts.push_back( database::getasphaltenes_BioFactor (m_record) );
  bioConsts.push_back( database::getresins_BioFactor      (m_record) );
  bioConsts.push_back( database::getC15Aro_BioFactor      (m_record) );
  bioConsts.push_back( database::getC15Sat_BioFactor      (m_record) );
  bioConsts.push_back( database::getC6_14Aro_BioFactor    (m_record) );
  bioConsts.push_back( database::getC6_14Sat_BioFactor    (m_record) );
  bioConsts.push_back( database::getC5_BioFactor          (m_record) );
  bioConsts.push_back( database::getC4_BioFactor          (m_record) );
  bioConsts.push_back( database::getC3_BioFactor          (m_record) );
  bioConsts.push_back( database::getC2_BioFactor          (m_record) );
  bioConsts.push_back( database::getC1_BioFactor          (m_record) );
  bioConsts.push_back( database::getCOx_BioFactor         (m_record) );
  bioConsts.push_back( database::getN2_BioFactor          (m_record) );

  return BioConsts( database::getTempConstant(m_record), bioConsts ); 
}

double BiodegradationParameters::timeFactor() const
{
  return database::getTimeConstant(m_record);
}

double BiodegradationParameters::bioRate() const
{
   double bioRate = database::getBioRate(m_record);

   // The biodegradation rate cannot be less than 0.0 m/Ma)
   if (bioRate < 0.0)
   {
      getProjectHandle()->getMessageHandler().print("Basin_Warning: The biodegradation rate coefficient must be positive: ");
      getProjectHandle()->getMessageHandler().print(bioRate);
      getProjectHandle()->getMessageHandler().printLine(" < 0.0 (in m/Ma)");

      bioRate = 0.0;
   }
   return bioRate;
}

bool BiodegradationParameters::pasteurizationInd() const
{
   return database::getPasteurizationInd(m_record) == 0 ? false : true;
}

} } // namespace DataAccess::Interface
