//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "RiftingEvent.h"

// std library
#include <exception>

// DataAccess library
#include "Interface/GridMap.h"

using namespace CrustalThickness;
using DataAccess::Interface::TectonicFlag;

RiftingEvent::RiftingEvent( const TectonicFlag tectonicFlag,
                            GridMap const * const seaLevelAdjustment,
                            GridMap const * const maximumOceanicCrustThickness ) :
   m_tectonicFlag( tectonicFlag ),
   m_seaLevelAdjustment(seaLevelAdjustment),
   m_maximumOceanicCrustThickness( maximumOceanicCrustThickness ),
   m_startRiftAge( DataAccess::Interface::DefaultUndefinedScalarValue ),
   m_endRiftAge  ( DataAccess::Interface::DefaultUndefinedScalarValue ),
   m_calculationMask ( false ),
   m_riftId      ( 0 )
{
   if (seaLevelAdjustment == nullptr){
      throw std::invalid_argument( "No sea level adjustement is provided for the current rifting event" );
   }
   if (maximumOceanicCrustThickness == nullptr){
      throw std::invalid_argument( "No maximum oceanic crustal thickness is provided for the current rifting event" );
   }
};


void RiftingEvent::setStartRiftAge( const double age ){
   if (age != DataAccess::Interface::DefaultUndefinedScalarValue){
      if (age < 0){
         throw std::invalid_argument( "Cannot set a start rift age for the rifting event to a negative value" );
      }
      else if (age == 0){
         throw std::invalid_argument( "Cannot set a start rift age for the rifting event to present day 0Ma" );
      }
      else if (m_endRiftAge != DataAccess::Interface::DefaultUndefinedScalarValue
         and m_endRiftAge >= age){
         throw std::invalid_argument( "Cannot set a start rift age for the rifting event to " + std::to_string(age) +
            "Ma because it is posterior to its end rift age " + std::to_string(m_endRiftAge)  + "Ma" );
      }
      else{
         m_startRiftAge = age;
      }
   }
   else{
      m_startRiftAge = age;
   }
}


void RiftingEvent::setEndRiftAge( const double age ){
   if (age != DataAccess::Interface::DefaultUndefinedScalarValue){
      if (age < 0){
         throw std::invalid_argument( "Cannot set an end rift age for the rifting event to a negative value" );
      }
      else if (m_startRiftAge != DataAccess::Interface::DefaultUndefinedScalarValue
         and age >= m_startRiftAge){
         throw std::invalid_argument( "Cannot set an end rift age for the rifting event to " + std::to_string( age ) +
            "Ma because it is anterior to its start rift age " + std::to_string( m_startRiftAge ) + "Ma" );
      }
      else{
         m_endRiftAge = age;
      }
   }
   else{
      m_endRiftAge = age;
   }
}

