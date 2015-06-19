//                                                                      
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file VarPrmCrustThinning.h
/// @brief This file keeps API implementation for handling variation of Single thinning crust event parameter. 


#include "PrmCrustThinning.h"
#include "VarPrmCrustThinning.h"

#include "Path.h" // for to_string

#include <cassert>

namespace casa
{

VarPrmCrustThinning::VarPrmCrustThinning( const std::vector<double>      & baseValues
                                        , const std::vector<double>      & minValues
                                        , const std::vector<double>      & maxValues
                                        , const std::vector<std::string> & mapsName
                                        , PDF                              prmPDF
                                        ) 
{
   m_pdf = prmPDF;
   m_mapsName.insert( m_mapsName.begin(), mapsName.begin(), mapsName.end() );
   m_eventsNumber = (baseValues.size() - 1) / 3;

   // check input values
   if ( baseValues.size() != minValues.size() ||
        baseValues.size() != maxValues.size() || // base/min/max arrays must have same dimensions
        m_eventsNumber == 0                   ||
        ((baseValues.size() - 1) % 3) != 0    || // initial crust thickness, (t0,deltaT,fact),(...),(...)
        mapsName.size() != m_eventsNumber        // maps name number must be the same as number of events
      )
   {
      throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << 
            "Wrong parameters number for CrustTinnning influential parameter";
   }

   for ( int i = -1, pos = 0; i < m_eventsNumber; ++i )
   {
      for ( size_t j = 0; j < 3; ++j, ++pos )
      {
         if ( minValues[pos] > baseValues[pos] || baseValues[pos] > maxValues[pos] )
         {
            ErrorHandler::Exception ex( ErrorHandler::OutOfRangeValue );
            ex << "Crust thinning event: " << i + (i < 0 ? 1 : i + 1) << ", has ";

            if (      i <  0 ) { ex << "initial crust thickness"; }
            else if ( i == 0 ) { ex << "start time"; }
            else if ( i == 1 ) { ex << "event duration"; }
            else               { ex << "thinning factor"; }

            throw ex << " base case value: " << baseValues[pos] <<  " outside of the given [" << 
                        minValues[pos] << "," << maxValues[pos] << "] range";
         }
         if ( i < 0 ) { ++pos; break; }
      }
   }
   
   // create base/min/max parameters
   m_minValue.reset(  new PrmCrustThinning( this, minValues,  mapsName ) );
   m_maxValue.reset(  new PrmCrustThinning( this, maxValues,  mapsName ) );
   m_baseValue.reset( new PrmCrustThinning( this, baseValues, mapsName ) );
}

VarPrmCrustThinning::~VarPrmCrustThinning()
{
}

std::vector<std::string> VarPrmCrustThinning::name() const
{
   std::vector<std::string> ret;
   ret.push_back("InitialCrustThickness [m]");

   for ( size_t i = 0; i < m_eventsNumber; ++i )
   {  
      const std::string & evNum = ibs::to_string( m_eventsNumber );

      ret.push_back( std::string( "Event_" ) + evNum + "_StartTime [Ma]" );
      ret.push_back( std::string( "Event_" ) + evNum + "_Duration [Ma]" );
      ret.push_back( std::string( "Event_" ) + evNum + "_CrustThinningFactor [m/m] ");
   }
   return ret;
}


SharedParameterPtr VarPrmCrustThinning::newParameterFromDoubles( std::vector<double>::const_iterator & vals ) const
{
   const std::vector<double> & minV = dynamic_cast<PrmCrustThinning*>( m_minValue.get() )->asDoubleArray();
   const std::vector<double> & maxV = dynamic_cast<PrmCrustThinning*>( m_maxValue.get() )->asDoubleArray();

   std::vector<double> prmV( minV.size() );

   for ( size_t i = 0; i < minV.size(); ++i )
   {
      prmV[i] = *vals++;

      if ( prmV[i] < minV[i] || prmV[i] > maxV[i] )
      {
         ErrorHandler::Exception  ex( ErrorHandler::OutOfRangeValue );
         ex << "Variation of CrustThinning sub-parameter ";

         if ( i == 0 )              ex << "initial crust thickness";
         else if ( (i - 1)%3 == 0 ) ex << "event start time";
         else if ( (i - 1)%3 == 1 ) ex << "event duration";
         else                       ex << "thinning factor";
         
         throw ex << " parameter " << prmV[i] << " falls out of range: [" << minV[i] << ":" << maxV[i] << "]";
      }
   }

   SharedParameterPtr prm( new PrmCrustThinning( this, prmV, m_mapsName ) );

   return prm;
}

}

