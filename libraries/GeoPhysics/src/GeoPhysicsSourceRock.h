#ifndef _GEOPHYSICS__SOURCE_ROCK_H_
#define _GEOPHYSICS__SOURCE_ROCK_H_

#include "Interface/GridMap.h"

#include "SourceRock.h"
#include "LocalGridInterpolator.h"

#include "ComponentManager.h"
#include "GenexResultManager.h"

namespace GeoPhysics {

   class GeoPhysicsSourceRock : public Genex6::SourceRock {

   public :

      GeoPhysicsSourceRock (DataAccess::Interface::ProjectHandle * projectHandle, database::Record * record);

      ~GeoPhysicsSourceRock ();

      /// integrate GenEx over the time-interval and rturn the components expelled over this interval.
      void computeTimeInterval ( const double previousTime,
                                 const double currentTime, 
                                 const Genex6::LocalGridInterpolator* ves,
                                 const Genex6::LocalGridInterpolator* temperature,
                                 const Genex6::LocalGridInterpolator* thickness,
                                 const Genex6::LocalGridInterpolator* lithostaticPressure,
                                 const Genex6::LocalGridInterpolator* hydrostaticPressure,
                                 const Genex6::LocalGridInterpolator* porePressure,
                                 const Genex6::LocalGridInterpolator* porosity,
                                 const Genex6::LocalGridInterpolator* permeability,
                                 const Genex6::LocalGridInterpolator* vre,
                                 Interface::GridMap*            genexData );

      void fillResultMap ( const CBMGenerics::ComponentManager::SpeciesNamesId name,
                           Interface::GridMap*                                 map ) const;

      void fillOptionalResultMap ( const CBMGenerics::GenexResultManager::ResultId name,
                                   Interface::GridMap*                             map ) const;

      /// \brief Determine if adsorption needs to be applied.
      bool doApplyAdsorption () const;

      /// \brief Indicate if adsorption can be run for this source-rock.
      void setCanIncludeAdsorption ( const bool canIncludeAdsorption );

   protected :

      void collectIntervalResults ( Interface::GridMap* genexData );

   private :

      bool m_canIncludeAdsorption;

   };

}

#endif // _GEOPHYSICS__SOURCE_ROCK_H_
