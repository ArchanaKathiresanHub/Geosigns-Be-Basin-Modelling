#include "Interface/BiodegradationParameters.h"

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
  bioConsts.push_back( database::getasphaltenes_BioFactor(m_record) );
  bioConsts.push_back( database::getresins_BioFactor(m_record) );
  bioConsts.push_back( database::getC15Aro_BioFactor(m_record) );
  bioConsts.push_back( database::getC15Sat_BioFactor(m_record) );
  bioConsts.push_back( database::getC6_14Aro_BioFactor(m_record) );
  bioConsts.push_back( database::getC6_14Sat_BioFactor(m_record) );
  bioConsts.push_back( database::getC5_BioFactor(m_record) );
  bioConsts.push_back( database::getC4_BioFactor(m_record) );
  bioConsts.push_back( database::getC3_BioFactor(m_record) );
  bioConsts.push_back( database::getC2_BioFactor(m_record) );
  bioConsts.push_back( database::getC1_BioFactor(m_record) );
  bioConsts.push_back( database::getCOx_BioFactor(m_record) );
  bioConsts.push_back( database::getN2_BioFactor(m_record) );

  return BioConsts( database::getTempConstant(m_record), bioConsts ); 
}

double BiodegradationParameters::timeFactor() const
{
  return database::getTimeConstant(m_record);
}

double BiodegradationParameters::bioRate() const
{
   double bioRate = database::getBioRate(m_record);

   // The biodegradation rate cannot be outside of the allowed range of values (from 0.1 to 1.5 m/Ma)
   if (bioRate < 0.1)
   {
      cerr << "Warning: The bioRate coefficient used for biodegradation is too low: " << bioRate << " is outside of the allowed range (0.1 - 1.5 m/Ma)" << endl;
      bioRate = 0.1;
   }
   if (bioRate > 1.5)
   {
      cerr << "Warning: The bioRate coefficient used for biodegradation is too high: " << bioRate << " is outside of the allowed range (0.1 - 1.5 m/Ma)" << endl;
      bioRate = 1.5;
   }
   assert(bioRate >= 0.1 || bioRate <= 1.5);
   return bioRate;
}

} } // namespace DataAccess::Implementation
