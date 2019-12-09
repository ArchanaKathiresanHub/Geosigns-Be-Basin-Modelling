//
// Copyright (C) 2016-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef DERIVED_PROPERTIES__MAX_VES_HIGHRES_FORMATION_CALCULATOR_H
#define DERIVED_PROPERTIES__MAX_VES_HIGHRES_FORMATION_CALCULATOR_H

#include "FormationPropertyCalculator.h"
#include "GeoPhysicsProjectHandle.h"
#include "AbstractPropertyManager.h"
#include "AbstractSnapshot.h"
#include "AbstractFormation.h"
#include "FormationProperty.h"
#include "DerivedFormationProperty.h"

namespace DerivedProperties
{

   /// \brief Calculates the high resolution max VES for a formation.
   class MaxVesHighResFormationCalculator : public AbstractDerivedProperties::FormationPropertyCalculator
   {

   public :

      /// \brief Input constructor.
      /// \param [in]  projectHandle   Manager for all derived properties.
      /// \pre SimulationDetails is not null, ie there is at least one last run.
      MaxVesHighResFormationCalculator( const GeoPhysics::ProjectHandle& projectHandle );

      /// \brief Calculate the high resolution max VES for the formation at the snapshot age.
      /// \param [in]  propertyManager   Manager for all derived properties.
      /// \param [in]  snapshot          The snapshot at which the property is to be calculated.
      /// \param [in]  formation         The formation for which the property is to be calculated.
      /// \param [out] derivedProperties On exit will contain the newly computed high resolution max VES.
      /// \pre snapshot is not null and is a valid snapshot age.
      /// \pre formation is not null and is a valid formation.
      virtual void calculate (       AbstractDerivedProperties::AbstractPropertyManager & propertyManager,
                               const DataModel::AbstractSnapshot *                        snapshot,
                               const DataModel::AbstractFormation *                       formation,
                                     AbstractDerivedProperties::FormationPropertyList &   derivedProperties ) const;

   private :

      /// \brief Compute (indirectly, because we actually already have it) high resolution max VES for non subsampled runs.
      void computeIndirectly(       AbstractDerivedProperties::AbstractPropertyManager & propertyManager,
                              const DataModel::AbstractSnapshot *                        snapshot,
                              const DataModel::AbstractFormation *                       formation,
                                    AbstractDerivedProperties::FormationPropertyList &   derivedProperties ) const;

      /// \brief Compute high resolution max VES for subsampled runs.
      void computeForSubsampledRun(       AbstractDerivedProperties::AbstractPropertyManager & propertyManager,
                                    const DataModel::AbstractSnapshot *                        snapshot,
                                    const DataModel::AbstractFormation *                       formation,
                                          AbstractDerivedProperties::FormationPropertyList &   derivedProperties ) const;

      const GeoPhysics::ProjectHandle& m_projectHandle; //!< Project handle

      const bool m_isSubsampled;                               //!< Boolean flag for subsampled grids

   };

}

#endif // DERIVED_PROPERTIES__MAX_VES_HIGHRES_FORMATION_CALCULATOR_H
