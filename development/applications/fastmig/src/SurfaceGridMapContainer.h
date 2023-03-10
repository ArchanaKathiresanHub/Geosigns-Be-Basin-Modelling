//
// Copyright (C) 2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _MIGRATION_SURFACEGRIDMAPCONTAINER_H_
#define _MIGRATION_SURFACEGRIDMAPCONTAINER_H_

#include "SurfaceGridMap.h"
#include "overburden_MPI.h"

#include <map>

using std::map;

namespace migration
{

   class SurfaceGridMapContainer
   {
   public:

      enum ContinuousPropertyEnum
      {
         CONTINUOUS_DEPTH,
         CONTINUOUS_TEMPERATURE,
         CONTINUOUS_LITHOSTATICPRESSURE,
         CONTINUOUS_HYDROSTATICPRESSURE
      };

      enum DiscontinuousPropertyEnum
      {
         DISCONTINUOUS_DEPTH,
         DISCONTINUOUS_POROSITY,
         DISCONTINUOUS_PERMEABILITY,
         DISCONTINUOUS_BRINEVISCOSITY
      };

      enum ConstantPropertyEnum
      {
         CONSTANT_LITHOTYPE1PERCENT,
         CONSTANT_LITHOTYPE2PERCENT,
         CONSTANT_LITHOTYPE3PERCENT
      };

      typedef std::vector<std::pair<const MigrationFormation*, SurfaceGridMap> > constant_properties;
      typedef std::vector<SurfaceGridMapFormations> continuous_properties;
      typedef std::vector<FormationSurfaceGridMaps> discontinuous_properties;

   private:

      enum SurfaceGridMapStatus
      {
         CREATED = 0, RETRIEVED, RESTORED
      };

      SurfaceGridMapStatus m_status;

      std::map<ConstantPropertyEnum, constant_properties> m_constants;
      std::map<ContinuousPropertyEnum, continuous_properties> m_continuous;
      std::map<DiscontinuousPropertyEnum, discontinuous_properties> m_discontinuous;

   public:

      SurfaceGridMapContainer ();
      ~SurfaceGridMapContainer ();

      void clear ();
      bool allValid ();
      void retrieveData ();
      void restoreData ();

      void setConstants (ConstantPropertyEnum prop, const constant_properties& gridMaps);
      void setContinuous (ContinuousPropertyEnum prop, const continuous_properties& gridMaps);
      void setDiscontinuous (DiscontinuousPropertyEnum prop, const discontinuous_properties& gridMaps);

      const constant_properties& constants (ConstantPropertyEnum prop) const
      {
         assert (m_status == RETRIEVED);
         std::map<ConstantPropertyEnum, constant_properties>::const_iterator it = m_constants.find (prop);
         assert (it != m_constants.end ());
         return (*it).second;
      }

      const continuous_properties& continuous (ContinuousPropertyEnum prop) const
      {
         assert (m_status == RETRIEVED);
         std::map<ContinuousPropertyEnum, continuous_properties>::const_iterator it = m_continuous.find (prop);
         assert (it != m_continuous.end ());
         return (*it).second;
      }

      const discontinuous_properties& discontinuous (DiscontinuousPropertyEnum prop) const
      {
         assert (m_status == RETRIEVED);
         std::map<DiscontinuousPropertyEnum, discontinuous_properties>::const_iterator it = m_discontinuous.find (prop);
         assert (it != m_discontinuous.end ());
         return (*it).second;
      }
   };

} // namespace migration

#endif
