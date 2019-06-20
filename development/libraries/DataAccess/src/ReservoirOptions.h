//
// Copyright (C) 2010-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _INTERFACE_RESERVOIROPTIONS_H_
#define _INTERFACE_RESERVOIROPTIONS_H_

#include "AbstractReservoir.h"

#include "DAObject.h"
#include "Interface.h"
#include "Formation.h"

namespace DataAccess
{
   namespace Interface
   {
      /// ReservoirOptions class containing global reservoir user options
      class ReservoirOptions : public DAObject
      {
      public:
         ReservoirOptions (ProjectHandle * projectHandle, database::Record * record);
         ~ReservoirOptions (void);

         /// @return Boolean whether diffusion takes place inside traps
         bool isDiffusionOn (void) const;
         /// @return Boolean whether biodegradation takes place inside traps
         bool isBiodegradationOn (void) const;
         /// @return Boolean whether OTGC takes place inside traps
         bool isOilToGasCrackingOn (void) const;
         /// @return Boolean whether vertical migration blocking into this reservoir is enabled
         bool isBlockingOn (void) const;
         /// @return The vertical migration blocking permeability threshold.
         double getBlockingPermeability (void) const;
         /// @return The vertical migration blocking porosity threshold.
         double getBlockingPorosity (void) const;
         /// @return The minimum height for oil columns
         double getMinOilColumnHeight (void) const;
         /// @return The minimum height for gas columns
         double getMinGasColumnHeight (void) const;
         /// @return The minimum capacity for the traps of this Reservoir
         double getTrapCapacity (void) const;         
      };
   }
}

#endif // _INTERFACE_RESERVOIROPTIONS_H_
