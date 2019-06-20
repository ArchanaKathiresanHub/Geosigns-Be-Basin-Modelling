#include "CauldronWell.h"

#include "Interface.h"

const double DataAccess::Mining::CauldronWell::DefaultSamplingResolution = 50;

DataAccess::Mining::CauldronWell::CauldronWell ( const std::string& name ) : m_name ( name )
{ 
   m_nullValue = DataAccess::Interface::DefaultUndefinedScalarValue;
   m_waterDepth = m_nullValue;
   m_kellyBushingDepth = m_nullValue;   
   m_elevation = m_nullValue;
   m_samplingResolution = DefaultSamplingResolution;
}

void DataAccess::Mining::CauldronWell::setNullValue ( const double nullValue ) {

   double oldNullValue = m_nullValue;

   m_nullValue = nullValue;

   if ( m_elevation == oldNullValue ) {
      m_elevation = m_nullValue;
   }

   if ( m_waterDepth == oldNullValue ) {
      m_waterDepth = m_nullValue;
   }

   if ( m_kellyBushingDepth == oldNullValue ) {
      m_kellyBushingDepth = m_nullValue;
   }

}

void DataAccess::Mining::CauldronWell::setElevation ( const double elevation ) {
   m_elevation = elevation;
}

void DataAccess::Mining::CauldronWell::setWaterDepth ( const double waterDepth ) {
   m_waterDepth = waterDepth;
}

void DataAccess::Mining::CauldronWell::setKellyBushingDepth ( const double kellyBushingDepth ) {
   m_kellyBushingDepth = kellyBushingDepth;
}

void DataAccess::Mining::CauldronWell::setSamplingResolution ( const double resolution ) {
   m_samplingResolution = resolution;
}

