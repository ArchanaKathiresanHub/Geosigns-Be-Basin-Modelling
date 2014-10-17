#ifndef _MIGRATION_FORMATION_H_
#define _MIGRATION_FORMATION_H_

namespace database
{
   class Record;
   class Table;
}

#include "GeoPhysicsFormation.h"

namespace DataAccess
{
   namespace Interface
   {
      class Property;
      class Snapshot;
   }
}

using namespace DataAccess;

#include "SurfaceGridMap.h"
#include "FormationSurfaceGridMaps.h"
#include "RequestDefs.h"

/// Formation Class
namespace migration
{

   /// This class implements the Formation-specific migration functionality.
   /// It is constructed on top of the DataAccess::formation class.
   class Formation : public GeoPhysics::Formation
   {
      public:
	 /// This constructor is called by the object factory
	 Formation (Interface::ProjectHandle * projectHandle, database::Record * record);

	 /// Destructor
	 virtual ~Formation (void);

	 FormationSurfaceGridMaps getFormationSurfaceGridMaps(const DataAccess::Interface::Property* prop, 
	       const DataAccess::Interface::Snapshot* snapshot) const;

	 SurfaceGridMap getTopSurfaceGridMap(const DataAccess::Interface::Property* prop, const DataAccess::Interface::Snapshot* snapshot) const;

	 SurfaceGridMap getBaseSurfaceGridMap(const DataAccess::Interface::Property* prop, const DataAccess::Interface::Snapshot* snapshot) const;

	 const GridMap * getTopGridMap(const DataAccess::Interface::Property* prop, const DataAccess::Interface::Snapshot* snapshot) const;

	 const GridMap * getBaseGridMap(const DataAccess::Interface::Property* prop, const DataAccess::Interface::Snapshot* snapshot) const;

	 const Formation * getTopFormation () const;
	 const Formation * getBottomFormation () const;
   };
}

#endif // _MIGRATION_FORMATION_H_

