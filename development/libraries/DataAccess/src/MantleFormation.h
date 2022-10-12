//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#ifndef INTERFACE_MANTLE_FORMATION_H
#define INTERFACE_MANTLE_FORMATION_H

// DataAccess library
#include "DAObject.h"
#include "Interface.h"
#include "BasementFormation.h"

namespace DataAccess
{
   namespace Interface
   {

      class MantleFormation : virtual public BasementFormation
      {
      public:

         MantleFormation (ProjectHandle& projectHandle, database::Record* record);

         ~MantleFormation (void);

         /// Return the present day, user-supplied Mantle thickness GridMap.
         const GridMap * getInputThicknessMap (void) const;
         const GridMap * getInitialThicknessMap (void) const;

         /// The present day thickness is always a constant, it can never be a map.
         ///
         /// The values depends on the bottom boundary condition.
         virtual double getPresentDayThickness () const;
         virtual double getInitialLithosphericMantleThickness () const;

         /// Return a sequence of paleo-thickness maps.
         ///
         /// The thickness of the mantle is controlled by both the present
         /// day mantle thickness and the crust thinning history.
         PaleoFormationPropertyList * getPaleoThicknessHistory () const;

         virtual int getDepositionSequence () const;

         void asString (std::string & str) const;

      };
   }
}
#endif // INTERFACE_MANTLE_FORMATION_H
