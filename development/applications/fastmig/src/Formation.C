#include "RequestHandling.h"

#include "Formation.h"
#include "Surface.h"
#include "rankings.h"
#include "migration.h"
#include "ObjectFactory.h"

#include "Interface/ProjectHandle.h"
#include "Interface/Property.h"
#include "Interface/Snapshot.h"
#include "Interface/PropertyValue.h"
#include "Interface/GridMap.h"

#include "Migrator.h"
#include "MigrationPropertyManager.h"
#include "PrimaryFormationProperty.h"
#include "PrimarySurfaceProperty.h"

#include "LinearGridInterpolator.h"
#include "GeoPhysicsSourceRock.h"

#include <assert.h>
#include <math.h>

#include <iostream>
#include <vector>
using namespace std;

using namespace CBMGenerics;

using namespace DataAccess;
using Interface::Property;
using Interface::Snapshot;
using Interface::ProjectHandle;
using Interface::PropertyValueList;
using Interface::FormationList;

namespace migration
{

Formation::Formation (ProjectHandle * projectHandle, database::Record * record) :
   Interface::Formation (projectHandle, record),
   GeoPhysics::Formation (projectHandle, record)
{
   m_isInitialised = false;

   m_genexData = 0;

   m_startGenexTime = -1;
   m_endGenexTime = -1;
}

Formation::~Formation (void)
{
   if( m_genexData != 0 ) {
      delete m_genexData;
   }
}

FormationSurfaceGridMaps Formation::getFormationSurfaceGridMaps(const Property* prop, const Snapshot* snapshot ) const
{
   unsigned int index = 0;

   const GridMap* top  = getFormationPrimaryPropertyGridMap ( prop, snapshot );
   const GridMap* base = top;

   if ( top ) {

      index = top->getDepth()-1;

   } else {
      top =  getSurfacePropertyGridMap ( prop, snapshot, getTopSurface() );
      base = getSurfacePropertyGridMap ( prop, snapshot, getBottomSurface() );
      if( !top ) {
         //     cout << "No top property " << prop->getName() << " " << this->getName() << " " << snapshot->getTime() << endl;
      }
      if( !base ) {
         //      cout << "No base property " << prop->getName() << " " << this->getName() << " " << snapshot->getTime() << endl;
      }
   }

#ifdef DEBUG_FORMATION
   string formName = this->getName();
   double snapTime = snapshot->getTime();
   string propName = prop->getName();

   if (!top && !base) {
      cerr << "Formation.C:139: No formation and surface property grid maps found for formation: '" << 
	 formName << "' and property: '" << propName << "' at time: " << snapTime << "." << endl;
      cerr.flush();
   }
   else if (!top) {
      cerr << "Formation.C:143: No formation and top surface property grid map found for formation: '" << 
	 formName << "' and property: '" << propName << "' at time: " << snapTime << "." << endl;
      cerr.flush();
   }
   else if (!base) {
      cerr << "Formation.C:147: No formation and base surface property grid map found for formation: '" << 
	 formName << "' and property: '" << propName << "' at time: " << snapTime << "." << endl;
      cerr.flush();
   }
#endif
   return FormationSurfaceGridMaps( SurfaceGridMap( top, index ), SurfaceGridMap( base, (unsigned int) 0 ), this );
}


SurfaceGridMap Formation::getTopSurfaceGridMap(const Property* prop, const Snapshot* snapshot) const
{
   unsigned int index = 0;
   const GridMap* top = getFormationPrimaryPropertyGridMap ( prop, snapshot );
  
   if (top) {
      index = top->getDepth()-1;
   } else {
      top = getSurfacePropertyGridMap ( prop, snapshot, getTopSurface() );
   }

   return SurfaceGridMap( top, index );
}

SurfaceGridMap Formation::getBaseSurfaceGridMap(const Property* prop, const Snapshot* snapshot) const
{
   const GridMap* base = getFormationPrimaryPropertyGridMap ( prop, snapshot );

   if (!base)
      base = getSurfacePropertyGridMap ( prop, snapshot, getBottomSurface() ); 

   return SurfaceGridMap( base, (unsigned int)0 );
}


const Formation * Formation::getTopFormation () const
{
   const Interface::Surface * surface = getTopSurface ();

   return dynamic_cast <const Formation *> (surface ? surface->getTopFormation () : 0);
}

const Formation * Formation::getBottomFormation () const
{
   const Interface::Surface * surface = getBottomSurface ();
      
   return dynamic_cast <const Formation *> (surface ? surface->getBottomFormation () : 0);
}
 
// Use this method for getting a formation map of "primary" properties. If the formation map is not available calculate a surface map on-the-fly.
const GridMap* Formation::getFormationPrimaryPropertyGridMap ( const Property* prop, const Interface::Snapshot * snapshot ) const {

   const GridMap* theMap = 0;
   Migrator* mig = dynamic_cast<migration::Migrator*>( getProjectHandle() );

   if ( mig != 0 ) {
 
      DerivedProperties::FormationPropertyPtr theProperty = mig->getPropertyManager ().getFormationProperty ( prop, snapshot, this ); 
      if( theProperty != 0 ) {
      
         const DerivedProperties::PrimaryFormationProperty * thePrimaryProperty =  dynamic_cast< const DerivedProperties::PrimaryFormationProperty *>( theProperty.get() );

         if( thePrimaryProperty != 0 ) {
            theMap = thePrimaryProperty->getGridMap();
         } else {
            theMap = mig->getPropertyManager ().produceDerivedGridMap ( theProperty );
         }

      }

   }

   return theMap;
}

const GridMap* Formation::getSurfacePropertyGridMap( const Property* prop, const Snapshot* snapshot, const Interface::Surface* surface ) const
{

   const GridMap* theMap = 0;
   Migrator* mig = dynamic_cast<migration::Migrator*>( getProjectHandle() );

   if ( mig != 0 ) {

      if ( prop->getName ().find ( "Permeability" ) == std::string::npos ) {
 
         DerivedProperties::SurfacePropertyPtr theProperty = mig->getPropertyManager ().getSurfaceProperty ( prop, snapshot, surface ); 

         if( theProperty != 0 ) {
      
            const DerivedProperties::PrimarySurfaceProperty * thePrimaryProperty =  dynamic_cast< const DerivedProperties::PrimarySurfaceProperty *>( theProperty.get() );

            if( thePrimaryProperty != 0 ) {
               theMap = thePrimaryProperty->getGridMap();
            } else {
               theMap = mig->getPropertyManager ().produceDerivedGridMap ( theProperty );
            }

         } else {
            DerivedProperties::FormationSurfacePropertyPtr theFormationProperty = mig->getPropertyManager ().getFormationSurfaceProperty ( prop, snapshot, this, surface ); 

            if( theFormationProperty != 0 ) {
               theMap = mig->getPropertyManager ().produceDerivedGridMap ( theFormationProperty );
            }

         }

      } else {
         DerivedProperties::FormationSurfacePropertyPtr theFormationProperty = mig->getPropertyManager ().getFormationSurfaceProperty ( prop, snapshot, this, surface ); 

         if( theFormationProperty != 0 ) {
            theMap = mig->getPropertyManager ().produceDerivedGridMap ( theFormationProperty );
         }

      }

   }
 
   return theMap;  
}

bool Formation::preprocessSourceRock ( const double startTime, const bool printDebug ) {

   const double depoTime = ( getTopSurface () ? getTopSurface()->getSnapshot ()->getTime() : 0 );
   bool sourceRockIsActive = ( depoTime  > startTime )  || fabs ( depoTime - startTime ) < Genex6::Constants::ZERO;

   if( sourceRockIsActive  ) {
      if( GetRank() == 0 ) {
         cout << "Preprocessing formation " << getName() << " at the time interval: " << depoTime << " Ma to " <<  startTime << " Ma ..." <<endl;
      }
 
      if( m_genexData == 0 ) {
         m_genexData = m_projectHandle->getFactory()->produceGridMap ( 0, 0, m_projectHandle->getActivityOutputGrid (), 99999.0, NUM_COMPONENTS );
      }
      const GeoPhysics::GeoPhysicsSourceRock * sourceRock = dynamic_cast<const GeoPhysics::GeoPhysicsSourceRock *>( getSourceRock1 ());
      GeoPhysics::GeoPhysicsSourceRock * sourceRock1 = const_cast<GeoPhysics::GeoPhysicsSourceRock *>( sourceRock );
      
      Migrator* mig = dynamic_cast<migration::Migrator*>( getProjectHandle() );
      Interface::SnapshotList * snapshots = m_projectHandle->getSnapshots (Interface::MINOR | Interface::MAJOR);
      
      // present day map
      DerivedProperties::FormationPropertyPtr vrProperty = mig->getPropertyManager ().getFormationProperty ( mig->getPropertyManager ().getProperty ( "Vr" ), *(snapshots->begin ()), this ); 
      const GridMap * gridMapEnd = mig->getPropertyManager ().produceDerivedGridMap ( vrProperty );
      
      if( !gridMapEnd ) {
         
         if (GetRank () == 0) {
            cout << getName() <<  ": Cannot find Vr present day  map" << endl;
         }
         return false;
      }
      m_isInitialised = true;
      
      sourceRock1->setFormationData ( this );
      sourceRock1->initialize ( false );
      sourceRock1->preprocess ( gridMapEnd, gridMapEnd, false );
      
      Interface::SnapshotList::reverse_iterator snapshotIter;
      
      const Interface::Snapshot * start;
      const Interface::Snapshot * end = 0;
      bool status = true;
      
      for(snapshotIter = snapshots->rbegin (); snapshotIter != snapshots->rend () - 1; ++ snapshotIter) {
         
         start = (*snapshotIter);
         end = 0;
         
         if( start->getTime () > startTime ) {
            if (( depoTime > start->getTime ()) || (fabs (depoTime - start->getTime ()) < Genex6::Constants::ZERO)) {
               
               start = (*snapshotIter);
               end = *(snapshotIter + 1);
               
               status = calculateGenexTimeInterval( start, end, printDebug);
            }
         }
      }
      
      delete snapshots; 

      return status;
   }
   return true;
}


bool Formation::calculateGenexTimeInterval ( const Interface::Snapshot * start, const Interface::Snapshot * end, const bool printDebug ) {

   const double depoTime = ( getTopSurface () ? getTopSurface()->getSnapshot ()->getTime() : 0 );
   bool sourceRockIsActive = ( depoTime  > start->getTime() )  || fabs ( depoTime -  start->getTime ()) < Genex6::Constants::ZERO;

   if( sourceRockIsActive && m_isInitialised ) {

      bool status = m_isInitialised;
 
      if( start->getTime() == m_startGenexTime && end->getTime() == m_endGenexTime ) {
         if (GetRank () == 0 and printDebug) {
            cout << getName() <<  ": Genex interval start = " << start->getTime() << ", end = " << end->getTime() <<  " is already calculated" << endl;
         }
         return true;
      }       

      Genex6::LinearGridInterpolator vesInterp;
      Genex6::LinearGridInterpolator temperatureInterp;
      Genex6::LinearGridInterpolator thicknessInterp;
      
      Genex6::LinearGridInterpolator hydrostaticPressureInterp;
      Genex6::LinearGridInterpolator lithostaticPressureInterp;
      Genex6::LinearGridInterpolator porePressureInterp;
      Genex6::LinearGridInterpolator porosityInterp;
      Genex6::LinearGridInterpolator permeabilityInterp;
      Genex6::LinearGridInterpolator vreInterp;
      
      status = extractGenexDataInterval ( start, end,
                                          thicknessInterp,
                                          vesInterp,
                                          temperatureInterp,
                                          hydrostaticPressureInterp,
                                          lithostaticPressureInterp,
                                          porePressureInterp,
                                          porosityInterp,
                                          permeabilityInterp,
                                          vreInterp );
      
      if( ! status ) {
         if (GetRank () == 0) {
            cout << getName() <<  ": Cannot extract  genex interval start = " << start->getTime() << ", end = " << end->getTime() <<  endl;
         }
        
         return false;
      }
      
      if (GetRank () == 0 and printDebug ) {
         cout << getName() <<  ": Calculate genex interval start = " << start->getTime() << ", end = " << end->getTime() <<  endl;
      }

      const GeoPhysics::GeoPhysicsSourceRock * sourceRock = dynamic_cast<const GeoPhysics::GeoPhysicsSourceRock *>( getSourceRock1 ());
      GeoPhysics::GeoPhysicsSourceRock * sourceRock1 = const_cast<GeoPhysics::GeoPhysicsSourceRock *>( sourceRock );
      
      sourceRock1->computeTimeInterval ( start->getTime(), end->getTime(),
                                         &vesInterp,
                                         &temperatureInterp,
                                         &thicknessInterp,
                                         &lithostaticPressureInterp,
                                         &hydrostaticPressureInterp,
                                         &porePressureInterp,
                                         &porosityInterp,
                                         &permeabilityInterp,
                                         &vreInterp,
                                         m_genexData );
      
      
      
      m_startGenexTime = start->getTime();
      m_endGenexTime   = end->getTime();
      
      return status;
   } 
   
   return true;
}

bool Formation::extractGenexDataInterval ( const Snapshot *intervalStart,
                                           const Snapshot *intervalEnd,
                                           Genex6::LinearGridInterpolator& thickness,
                                           Genex6::LinearGridInterpolator& ves,
                                           Genex6::LinearGridInterpolator& temperature,
                                           Genex6::LinearGridInterpolator& hydrostaticPressure,
                                           Genex6::LinearGridInterpolator& lithostaticPressure,
                                           Genex6::LinearGridInterpolator& porePressure,
                                           Genex6::LinearGridInterpolator& porosity,
                                           Genex6::LinearGridInterpolator& permeability,
                                           Genex6::LinearGridInterpolator& vre ) {
   

   Migrator* mig = dynamic_cast<migration::Migrator*>( m_projectHandle );
 
   const DataModel::AbstractProperty* property = mig->getPropertyManager ().getProperty ( "ErosionFactor" );
   DerivedProperties::FormationMapPropertyPtr startProperty = mig->getPropertyManager ().getFormationMapProperty ( property, intervalStart, this );
   DerivedProperties::FormationMapPropertyPtr endProperty   = mig->getPropertyManager ().getFormationMapProperty ( property, intervalEnd, this );;

   bool status = true;
   if( startProperty && endProperty ) {
      startProperty->retrieveData();
      endProperty->retrieveData();

      thickness.compute(intervalStart, startProperty, intervalEnd, endProperty );  

      startProperty->restoreData();
      endProperty->restoreData();
   } else {
      if (GetRank () == 0) {
         if( !startProperty ) {
            cout << "Property map ErosionFactor " << getTopSurface()->getName() << " at " << intervalStart->getTime() << " not found" << endl;
         }
         if( !endProperty ) {
            cout << "Property map ErosionFactor " << getTopSurface()->getName() << " at " << intervalEnd->getTime() << " not found" << endl;
         }
      }
      status = false;
   }
   
   status = computeInterpolator ( "Ves", intervalStart, intervalEnd, ves ) and status;
   status = computeInterpolator ( "Temperature", intervalStart, intervalEnd, temperature ) and status;
   status = computeInterpolator ( "HydroStaticPressure", intervalStart, intervalEnd, hydrostaticPressure ) and status;
   status = computeInterpolator ( "LithoStaticPressure", intervalStart, intervalEnd, lithostaticPressure ) and status;
   status = computeInterpolator ( "Pressure", intervalStart, intervalEnd, porePressure ) and status;

   status = computeInterpolator ( "Porosity", intervalStart, intervalEnd, porosity ) and status;
   status = computeInterpolator ( "Permeability", intervalStart, intervalEnd, permeability ) and status;
   status = computeInterpolator ( "VrVec2", intervalStart, intervalEnd, vre ) and status;

   return status;
}

bool Formation::computeInterpolator( const string & propertyName, const Snapshot *intervalStart, const Snapshot *intervalEnd, Genex6::LinearGridInterpolator& interpolator ) {

   const DataModel::AbstractProperty* property;
   Migrator* mig = dynamic_cast<migration::Migrator*>( m_projectHandle );
   
   property = mig->getPropertyManager ().getProperty ( propertyName );


   if( property->getPropertyAttribute () == DataModel::CONTINUOUS_3D_PROPERTY ) {
      DerivedProperties::SurfacePropertyPtr startSurfaceProperty = mig->getPropertyManager ().getSurfaceProperty ( property, intervalStart, getTopSurface() );
      DerivedProperties::SurfacePropertyPtr endSurfaceProperty   = mig->getPropertyManager ().getSurfaceProperty ( property, intervalEnd, getTopSurface() );

      startSurfaceProperty->retrieveData();
      endSurfaceProperty->retrieveData();

      interpolator.compute ( intervalStart, startSurfaceProperty, intervalEnd, endSurfaceProperty );

      startSurfaceProperty->restoreData();
      endSurfaceProperty->restoreData();
   } else if( property->getPropertyAttribute () == DataModel::DISCONTINUOUS_3D_PROPERTY ) {
      DerivedProperties::FormationSurfacePropertyPtr startProperty = mig->getPropertyManager ().getFormationSurfaceProperty ( property, intervalStart, 
                                                                                                                              this, getTopSurface() );
      DerivedProperties::FormationSurfacePropertyPtr endProperty = mig->getPropertyManager ().getFormationSurfaceProperty ( property, intervalEnd, 
                                                                                                                            this, getTopSurface() );
      
      if( startProperty && endProperty ) {
         startProperty->retrieveData();
         endProperty->retrieveData();
            
         interpolator.compute ( intervalStart, startProperty, intervalEnd, endProperty ); 
            
         startProperty->restoreData();
         endProperty->restoreData();
      } else {
         if (GetRank () == 0) {
            if( !startProperty ) {
               cout << "Property map " << propertyName << " " << getTopSurface()->getName() << " at " << intervalStart->getTime() << " not found" << endl;
            }
            if( !endProperty ) {
               cout << "Property map " << propertyName << " " << getTopSurface()->getName() << " at " << intervalEnd->getTime() << " not found" << endl;
            }
         }
         return false;
      }

   } else {

      if (GetRank () == 0) {
         cout << "Property map " << propertyName << " " << getTopSurface()->getName() << " at " << intervalStart->getTime() << " not found" << endl;
      }
      return false;
   }

   return true;
}

}
