#include "GeoPhysicsSourceRock.h"


#include <vector>
#include <cmath>

#include "Interface/ProjectHandle.h"
#include "Interface/Surface.h"
#include "Interface/Formation.h"
#include "Interface/Snapshot.h"

#include "database.h"
#include "cauldronschemafuncs.h"

#include "SourceRockNode.h"
#include "Simulator.h"
#include "Input.h"
#include "SpeciesResult.h"
#include "SpeciesManager.h"
#include "SimulatorState.h"

#include <iostream>

using namespace std;
using namespace database;
using namespace CBMGenerics;

GeoPhysics::GeoPhysicsSourceRock::GeoPhysicsSourceRock (DataAccess::Interface::ProjectHandle * projectHandle, database::Record * record) :
   Genex6::SourceRock ( projectHandle, record ) {

   m_canIncludeAdsorption = true;

}

GeoPhysics::GeoPhysicsSourceRock::~GeoPhysicsSourceRock () {
}

void GeoPhysics::GeoPhysicsSourceRock::computeTimeInterval ( const double             previousTime,
                                                             const double             currentTime, 
                                                             const Genex6::LocalGridInterpolator*   ves,
                                                             const Genex6::LocalGridInterpolator*   temperature,
                                                             const Genex6::LocalGridInterpolator*   thickness,
                                                             const Genex6::LocalGridInterpolator* lithostaticPressure,
                                                             const Genex6::LocalGridInterpolator* hydrostaticPressure,
                                                             const Genex6::LocalGridInterpolator* porePressure,
                                                             const Genex6::LocalGridInterpolator* porosity,
                                                             const Genex6::LocalGridInterpolator* permeability,
                                                             const Genex6::LocalGridInterpolator* vre,
                                                             Interface::GridMap* genexData ) {

   double depositionTime = m_projectHandle->findFormation ( getLayerName ())->getTopSurface ()->getSnapshot ()->getTime ();
   unsigned int i;
   unsigned int j;
   unsigned int k;

   zeroTimeStepAccumulations ();
   genexData->retrieveData ();

   for ( i = genexData->firstI (); i <= genexData->lastI (); ++i ) {

      for ( j = genexData->firstJ (); j <= genexData->lastJ (); ++j ) {

         for ( k = genexData->firstK (); k <= genexData->lastK (); ++k ) {
            genexData->setValue ( i, j, k, 0.0 );
         }

      }

   } 

#if 0
   cout << " computeTimeInterval " << previousTime << "  " << currentTime << "   " << getMaximumTimeStepSize ( depositionTime ) << endl;
#endif

   if ( previousTime - currentTime <= getMaximumTimeStepSize ( depositionTime ) or currentTime >= depositionTime ) {
      computeTimeInstance ( previousTime, currentTime, ves, temperature, thickness, lithostaticPressure, hydrostaticPressure, porePressure, porosity, permeability, vre );
   } else {

      // Check here that the interval end points are not computed twice.
      int numberOfTimeSteps = int ( std::ceil (( previousTime - currentTime ) / getMaximumTimeStepSize ( depositionTime )));
      double deltaT = ( previousTime - currentTime ) / double ( numberOfTimeSteps );

      double subTimeStepBegin = previousTime;
      double subTimeStepEnd = previousTime - deltaT;

#if 0
      cout << " numberOfTimeSteps " << numberOfTimeSteps << endl;
#endif

      for ( i = 1; i <= numberOfTimeSteps; ++i ) {
         computeTimeInstance ( subTimeStepBegin, subTimeStepEnd, ves, temperature, thickness, lithostaticPressure, hydrostaticPressure, porePressure, porosity, permeability, vre );
         subTimeStepBegin = subTimeStepEnd;
         subTimeStepEnd -= deltaT;
      }

   }

   collectIntervalResults ( genexData );
   genexData->restoreData ();

}

void GeoPhysics::GeoPhysicsSourceRock::collectIntervalResults ( Interface::GridMap* genexData ) {

   std::vector<Genex6::SourceRockNode*>::const_iterator itNode;

   unsigned int k;

   for ( itNode = m_theNodes.begin(); itNode != m_theNodes.end(); ++itNode ) {
      const unsigned int i = (*itNode)->GetI ();
      const unsigned int j = (*itNode)->GetJ ();

#if 0
      bool print = ( i == 1 and j == 1 );
#endif

      for ( k = genexData->firstK (); k <= genexData->lastK (); ++k ) {
         genexData->setValue ( i, j, k,  (*itNode)->getPrincipleSimulatorState ().getIntervalSpeciesCumulative ( ComponentManager::SpeciesNamesId ( k )));
         // genexData->setValue ( i, j, k,  (*itNode)->GetMixedSimulatorState ().getIntervalSpeciesCumulative ( ComponentManager::SpeciesNamesId ( k )));
         // genexData->setValue ( i, j, k,  (*itNode)->GetSimulatorState ().getIntervalSpeciesCumulative ( ComponentManager::SpeciesNamesId ( k )));

#if 0
         if ( print ) {
            std::cout << " species: " 
                      << ComponentManager::getInstance ().GetSpeciesName ( k ) << "  " 
                      << k << "  " 
                      << genexData->getValue ( i, j, k ) << "  "
                      << (*itNode)->getPrincipleSimulatorState ().getIntervalSpeciesCumulative ( ComponentManager::SpeciesNamesId ( k )) << "  "
                      << (*itNode)->GetMixedSimulatorState ().getIntervalSpeciesCumulative ( ComponentManager::SpeciesNamesId ( k )) 
                      << std::endl;
         }
#endif

      }

   }

}


void GeoPhysics::GeoPhysicsSourceRock::fillResultMap ( const ComponentManager::SpeciesNamesId name,
                                                       Interface::GridMap*               map ) const {


   const std::string& speciesName = ComponentManager::getInstance ().GetSpeciesName ( name );

   std::vector<Genex6::SourceRockNode*>::const_iterator itNode;

   for ( itNode = m_theNodes.begin(); itNode != m_theNodes.end(); ++itNode ) {
      const unsigned int i = (*itNode)->GetI ();
      const unsigned int j = (*itNode)->GetJ ();

#if 0
      bool print = ( i == 1 and j == 1 );
#endif

      const Genex6::SpeciesManager& speciesManager = getSpeciesManager ();

      const int speciesId = speciesManager.mapComponentManagerSpeciesToId ( name );

      if ( speciesId > 0 ) {
         const Genex6::SimulatorState& state = (*itNode)->getPrincipleSimulatorState ();
         const Genex6::SpeciesResult& result = state.GetSpeciesResult ( speciesId );

         map->setValue ( i, j, result.GetExpelledMass ());

#if 0
         if ( print ) {
            cout << " filled result map:  " << ComponentManager::getInstance ().GetSpeciesName ( name ) << "  " << result.GetExpelledMass () << endl;
         }
#endif

      }

   }

}


void GeoPhysics::GeoPhysicsSourceRock::fillOptionalResultMap ( const GenexResultManager::ResultId name,
                                                               Interface::GridMap*           map ) const {


   std::vector<Genex6::SourceRockNode*>::const_iterator itNode;

   for ( itNode = m_theNodes.begin(); itNode != m_theNodes.end(); ++itNode ) {
      const unsigned int i = (*itNode)->GetI ();
      const unsigned int j = (*itNode)->GetJ ();

      const Genex6::SimulatorState& state = (*itNode)->getPrincipleSimulatorState ();

      map->setValue ( i, j, state.GetResult ( int ( name )));
   }

}

bool GeoPhysics::GeoPhysicsSourceRock::doApplyAdsorption () const {
   return m_canIncludeAdsorption and Genex6::SourceRock::doApplyAdsorption ();
}

void GeoPhysics::GeoPhysicsSourceRock::setCanIncludeAdsorption ( const bool canIncludeAdsorption ) {
   m_canIncludeAdsorption = canIncludeAdsorption;
}
