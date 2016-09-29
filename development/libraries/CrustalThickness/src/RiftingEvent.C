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

using namespace CrustalThickness;
using DataAccess::Interface::TectonicFlag;

RiftingEvent::RiftingEvent( const TectonicFlag tectonicFlag,
                            GridMap const * const seaLevelAdjustment,
                            GridMap const * const maximumOceanicCrustThickness ) :
   m_tectonicFlag( tectonicFlag ),
   m_seaLevelAdjustment(seaLevelAdjustment),
   m_maximumOceanicCrustThickness( maximumOceanicCrustThickness ) {
   if (seaLevelAdjustment == nullptr){
      throw std::invalid_argument( "No sea level adjustement is provided for the current rifting event" );
   }
   if (maximumOceanicCrustThickness == nullptr){
      throw std::invalid_argument( "No maximum oceanic crustal thickness is provided for the current rifting event" );
   }
};
