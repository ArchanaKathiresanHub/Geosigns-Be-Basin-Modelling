//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#ifndef INTERFACE_TRAP_H
#define INTERFACE_TRAP_H

// DataAccess library
#include "Interface/DAObject.h"
#include "Interface/Interface.h"

// CBMGenerics library
#include "ComponentManager.h"
typedef CBMGenerics::ComponentManager::PhaseId PhaseId;
typedef CBMGenerics::ComponentManager::SpeciesNamesId ComponentId;

namespace DataAccess
{
   namespace Interface
   {
      /// Trap Class
      /// A Trap object models a hydrocarbon storage area in a Reservoir
      class Trap : public DAObject
      {
         public:
            Trap (ProjectHandle * projectHandle, database::Record * record);
            ~Trap (void);

            /// See if this Trap matches the given conditions
            bool matchesConditions (const Reservoir * reservoir, const Snapshot * snapshot, unsigned int id);

            /// Return the id of this Trap
            virtual unsigned int getId (void) const;

            /// Return the Snapshot at which this Trap was computed
            virtual const Snapshot * getSnapshot (void) const;

            /// Return the Reservoir containing this Trap
            const Reservoir * getReservoir (void) const;

            /// Return the real world position of this Trap
            virtual void getPosition (double & x, double & y) const;

            /// Return the grid position of this Trap
            virtual void getGridPosition (unsigned int & i, unsigned int & j) const;

            /// return the mass of the given component in this Trap
            virtual double getMass (ComponentId componentId) const;

            /// return the mass of the given component in this Trap
            virtual double getMass (const string & componentName) const;

            /// return the volume of the given phase in this Trap
            virtual double getVolume (PhaseId phaseId) const;

            /// return the volume of the given phase in this Trap
            virtual double getVolume (const string & phaseName) const;

            /// return the Trap's depth
            virtual double getDepth (void) const;
            /// return the Trap's spill depth
            virtual double getSpillDepth (void) const;
            /// Return the real world position of this Trap's spill point
            virtual void getSpillPointPosition (double & x, double & y) const;
            /// Return the grid position of this Trap's spill point
            virtual void getSpillPointGridPosition (unsigned int & i, unsigned int & j) const;
            /// return the Trap's temperature
            virtual double getTemperature (void) const;
            /// return the Trap's pressure
            virtual double getPressure (void) const;
            /// return the Trap's gas-oil contact depth
            virtual double getGOC (void) const;
            /// return the Trap's oil-water contact depth
            virtual double getOWC (void) const;
            /// return the Trap's surface at the oil-water contact depth
            virtual double getWCSurface (void) const;
                 
            
            virtual void printOn (ostream &) const;
            virtual void asString (string &) const;

            void setReservoir (const Reservoir * reservoir);
            void setSnapshot (const Snapshot * snapshot);

         private:
            const Reservoir * m_reservoir;
            const Snapshot * m_snapshot;
      };
   }
}

#endif // INTERFACE_TRAP_H
