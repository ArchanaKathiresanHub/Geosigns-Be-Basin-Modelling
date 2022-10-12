//
// Copyright (C) 2010-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _INTERFACE_RESERVOIR_H_
#define _INTERFACE_RESERVOIR_H_

#include "AbstractReservoir.h"

#include "DAObject.h"
#include "Interface.h"
#include "Formation.h"

#include <map>

namespace DataAccess
{
   namespace Interface
   {

      /// Reservoir Class
      /// A Reservoir object models a carrier bed in a Formation.
      class Reservoir : public DataModel::AbstractReservoir, public DAObject
      {
      public:
         Reservoir (ProjectHandle& projectHandle, database::Record * record);
         ~Reservoir (void);

         /// Return the name of this Reservoir
         const std::string & getName (void) const;

         /// Set the name of a new reservoir
         void setName (const std::string reservoirName);

         /// Return the minimum capacity for the traps of this Reservoir
         double getTrapCapacity (void) const;

         /// Return the mangled name of this Reservoir
         virtual const std::string & getMangledName (void) const;

         /// Return the name of the Formation containing this Reservoir
         const std::string & getFormationName (void) const;

         /// set the Formation of this Reservoir
         void setFormation (const Formation * formation);

         /// Return the Formation containing this Reservoir
         const Formation * getFormation (void) const;

         /// Return the ActivityMode, one of "AlwaysActive", "NeverActive", "ActiveFrom"
         virtual const std::string & getActivityMode (void) const;

         /// Determine whether the reservoir is active at the given snapshot
         virtual bool isActive (const Interface::Snapshot * snapshot) const;

         /// Return the time at which this Reservoir becomes active.
         /// Only valid if ActivityMode == "ActiveFrom".
         virtual double getActivityStart (void) const;

         /// @return whether diffusion is ON
         bool isDiffusionOn (void) const;
         /// @return whether biodegradation is ON
         bool isBiodegradationOn (void) const;
         /// @return whether OTGC is ON
         bool isOilToGasCrackingOn (void) const;

         /// Return whether vertical migration blocking into this reservoir is enabled
         bool isBlockingOn (void) const;
         /// return the vertical migration blocking permeability threshold.
         double getBlockingPermeability (void) const;
         /// return the vertical migration blocking porosity threshold.
         double getBlockingPorosity (void) const;

         /// Return the minimum height for gas and oil columns
         double getMinOilColumnHeight (void) const;
         double getMinGasColumnHeight (void) const;

         /// Return the (GridMap) value of one of this Reservoir's attributes
         virtual const GridMap * getMap (ReservoirMapAttributeId attributeId) const;

         /// load a map
         GridMap * loadMap (ReservoirMapAttributeId attributeId) const;

         virtual void printOn (std::ostream &) const;
         virtual void asString (std::string &) const;

         void setTrappersUpAndDownstreamConnected (double time);
         bool trappersAreUpAndDownstreamConnected (double time) const;

      private:
         const Formation * m_formation;

         std::string m_mangledName;

         static const std::string s_MapAttributeNames[];

         std::map<double, bool> m_trappersUpAndDownstreamConnected;
      };
   }
}

#endif // _INTERFACE_RESERVOIR_H_
