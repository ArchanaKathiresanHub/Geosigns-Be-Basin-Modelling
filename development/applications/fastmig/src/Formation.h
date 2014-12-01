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
namespace Genex6
{
   class LinearGridInterpolator;
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

      SurfaceGridMap getBaseSurfaceGridMap(const DataAccess::Interface::Property* prop, const DataAccess::Interface::Snapshot* snapshot ) const;

      FormationSurfaceGridMaps getFormationSurfaceGridMaps(const DataAccess::Interface::Property* prop, const DataAccess::Interface::Snapshot* snapshot) const;

      SurfaceGridMap getTopSurfaceGridMap(const DataAccess::Interface::Property* prop, const DataAccess::Interface::Snapshot* snapshot ) const;

      const GridMap * getTopGridMap(const DataAccess::Interface::Property* prop, const DataAccess::Interface::Snapshot* snapshot) const;

      const GridMap * getBaseGridMap(const DataAccess::Interface::Property* prop, const DataAccess::Interface::Snapshot* snapshot) const;

      const Formation * getTopFormation () const;
      const Formation * getBottomFormation () const;
 
      const GridMap* getFormationPrimaryPropertyGridMap ( const DataAccess::Interface::Property* prop, const Interface::Snapshot * snapshot) const;

      const GridMap* getSurfacePropertyGridMap ( const DataAccess::Interface::Property* prop, const Interface::Snapshot * snapshot, 
                                                 const Interface::Surface* surface ) const;

      bool isPreprocessed () const;

      bool calculateGenexTimeInterval ( const Interface::Snapshot * start, const Interface::Snapshot * end, const bool printDebug = false );
      bool preprocessSourceRock ( const double startTime, const bool printDebug = false );
      const Interface::GridMap * getOneComponentMap ( int componentId );

      Interface::GridMap* getGenexData() const { return m_genexData; }

      
  private:

      // indicates whether the source rock is initialized and preprocessed
      bool m_isInitialised;

      // Last calculated genex time interval
      double m_startGenexTime;
      double m_endGenexTime;

      // Map of all genex data
      Interface::GridMap* m_genexData;    

      bool computeInterpolator( const string & propertyName, const Interface::Snapshot *intervalStart, const Interface::Snapshot *intervalEnd, 
                                Genex6::LinearGridInterpolator& interpolator );
      bool extractGenexDataInterval ( const Interface::Snapshot *intervalStart, const Interface::Snapshot *intervalEnd,
                                           Genex6::LinearGridInterpolator& thickness,
                                           Genex6::LinearGridInterpolator& ves,
                                           Genex6::LinearGridInterpolator& temperature,
                                           Genex6::LinearGridInterpolator& hydrostaticPressure,
                                           Genex6::LinearGridInterpolator& lithostaticPressure,
                                           Genex6::LinearGridInterpolator& porePressure,
                                           Genex6::LinearGridInterpolator& porosity,
                                           Genex6::LinearGridInterpolator& permeability,
                                           Genex6::LinearGridInterpolator& vre );
      

   };

inline bool Formation::isPreprocessed () const { 
   return m_isInitialised; 
}

}

#endif // _MIGRATION_FORMATION_H_

