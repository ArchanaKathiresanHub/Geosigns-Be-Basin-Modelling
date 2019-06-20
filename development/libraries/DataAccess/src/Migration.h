//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef INTERFACE_MIGRATION_H
#define INTERFACE_MIGRATION_H

// DataAccess library
#include "DAObject.h"
#include "Interface.h"

// CBMGenerics library
#include "ComponentManager.h"
typedef CBMGenerics::ComponentManager::SpeciesNamesId ComponentId;

namespace DataAccess
{
   namespace Interface
   {
      /// Migration Class
      /// A Migration object models the flow of an amount of HC through the petroleum system
      class Migration : public DAObject
      {
    public:
       Migration (ProjectHandle * projectHandle, database::Record * record);
       virtual ~Migration (void);

       bool matchesConditions (const string & process, const Formation * sourceFormation,
        const Snapshot * sourceSnapshot, const Reservoir * sourceReservoir, const Trapper * sourceTrapper,
        const Snapshot * destinationSnapshot, const Reservoir * destinationReservoir, const Trapper * destinationTrapper);

       /// return the process name
       virtual const string & getProcess (void) const;

       virtual unsigned int getSourceTrapId (void) const;
       virtual unsigned int getDestinationTrapId (void) const;

       /// Return the Source Snapshot of this Migration
       virtual const Snapshot * getSourceSnapshot (void) const;

       /// Return the Destination Snapshot of this Migration
       virtual const Snapshot * getDestinationSnapshot (void) const;

       /// Return the Source Formation of this Migration
       virtual const Formation * getSourceFormation (void) const;

       /// Return the Source Reservoir of this Migration
       virtual const Reservoir * getSourceReservoir (void) const;

       /// Return the Destination Reservoir of this Migration
       virtual const Reservoir * getDestinationReservoir (void) const;

       /// Return the Source Trapper of this Migration
       virtual const Trapper * getSourceTrapper (void) const;

       /// Return the Destination Trapper of this Migration
       virtual const Trapper * getDestinationTrapper (void) const;

       /// return the mass of the given component in this Migration
       virtual double getMass (ComponentId componentId) const;

       /// return the mass of the given component in this Migration
       virtual double getMass (const string & componentName) const;

       virtual void printOn (ostream &) const;
       virtual void asString (string &) const;

       void setSourceSnapshot       (const Snapshot * snapshot  );
       void setDestinationSnapshot  (const Snapshot * snapshot  );
       void setSourceFormation      (const Formation * formation);
       void setSourceReservoir      (const Reservoir * reservoir);
       void setDestinationReservoir (const Reservoir * reservoir);
       void setSourceTrapper        (const Trapper * trapper    );
       void setDestinationTrapper   (const Trapper * trapper    );

    private:
       const Snapshot * m_sourceSnapshot;
       const Snapshot * m_destinationSnapshot;
       const Formation * m_sourceFormation;
       const Reservoir * m_sourceReservoir;
       const Reservoir * m_destinationReservoir;
       const Trapper * m_sourceTrapper;
       const Trapper * m_destinationTrapper;
      };
   }
}

#endif // INTERFACE_MIGRATION_H
