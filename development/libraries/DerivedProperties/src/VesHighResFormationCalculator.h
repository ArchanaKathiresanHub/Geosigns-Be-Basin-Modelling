//
// Copyright (C) 2016-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef DERIVED_PROPERTIES__VES_HIGHRES_FORMATION_CALCULATOR_H
#define DERIVED_PROPERTIES__VES_HIGHRES_FORMATION_CALCULATOR_H

#include "FormationPropertyCalculator.h"
#include "GeoPhysicsProjectHandle.h"
#include "AbstractPropertyManager.h"
#include "AbstractSnapshot.h"
#include "AbstractFormation.h"
#include "FormationProperty.h"
#include "DerivedFormationProperty.h"

namespace GeoPhysics
{
   class GeoPhysicsFormation;
   class FluidType;
}

namespace DerivedProperties
{

   /// \brief Calculates the high resolution VES for a formation.
   class VesHighResFormationCalculator : public AbstractDerivedProperties::FormationPropertyCalculator
   {

   public :

      /// \brief Input constructor.
      /// \param [in]  projectHandle   Manager for all derived properties.
      /// \pre SimulationDetails is not null, ie there is at least one last run.
      VesHighResFormationCalculator( const GeoPhysics::ProjectHandle& projectHandle );

      /// \brief Calculate the high resolution VES for the formation at the snapshot age.
      /// \param [in]  propertyManager   Manager for all derived properties.
      /// \param [in]  snapshot          The snapshot at which the property is to be calculated.
      /// \param [in]  formation         The formation for which the property is to be calculated.
      /// \param [out] derivedProperties On exit will contain the newly computed high resolution VES.
      /// \pre snapshot is not null and is a valid snapshot age.
      /// \pre formation is not null and is a valid formation.
      virtual void calculate (       AbstractDerivedProperties::AbstractPropertyManager & propertyManager,
                               const DataModel::AbstractSnapshot *                        snapshot,
                               const DataModel::AbstractFormation *                       formation,
                                     AbstractDerivedProperties::FormationPropertyList &   derivedProperties ) const;

   private :

      /// \brief Compute (indirectly, because we actually already have it or interpolate it) high res VES for coupled or non subsampled runs.
      void computeIndirectly(       AbstractDerivedProperties::AbstractPropertyManager & propertyManager,
                              const DataModel::AbstractSnapshot *                        snapshot,
                              const DataModel::AbstractFormation *                       formation,
                                    AbstractDerivedProperties::FormationPropertyList &   derivedProperties ) const;

      /// \brief Compute high resolution VES for subsampled hydrostatic runs using a constant fluid density
      void computeForSubsampledHydroRun(       AbstractDerivedProperties::AbstractPropertyManager & propertyManager,
                                         const DataModel::AbstractSnapshot *                        snapshot,
                                         const DataModel::AbstractFormation *                       formation,
                                               AbstractDerivedProperties::FormationPropertyList &   derivedProperties ) const;

      /// \brief Initialize the formation top surface depending on the formation above (if any)
      void initializeTopSurface(       AbstractDerivedProperties::AbstractPropertyManager & propertyManager,
                                 const DataModel::AbstractProperty *                        vesHighResProperty,
                                 const DataModel::AbstractSnapshot *                        snapshot,
                                 const DataModel::AbstractFormation *                       formationAbove,
                                       DerivedFormationPropertyPtr &                        vesHighRes ) const;

      const GeoPhysics::ProjectHandle& m_projectHandle; //!< Project handle

      bool m_isCoupledMode;                                    //!< Boolean flag for coupled mode

      const bool m_isSubsampled;                               //!< Boolean flag for subsampled grids

   };

}

#endif // DERIVED_PROPERTIES__VES_HIGHRES_FORMATION_CALCULATOR_H
