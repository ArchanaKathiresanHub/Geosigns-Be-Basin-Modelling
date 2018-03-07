//
// Copyright (C) 2016-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef DERIVED_PROPERTIES__DEPTH_HIGHRES_FORMATION_CALCULATOR_H
#define DERIVED_PROPERTIES__DEPTH_HIGHRES_FORMATION_CALCULATOR_H

#include "FormationPropertyCalculator.h"
#include "GeoPhysicsProjectHandle.h"
#include "AbstractPropertyManager.h"
#include "AbstractSnapshot.h"
#include "AbstractFormation.h"
#include "FormationProperty.h"
#include "DerivedFormationProperty.h"

namespace GeoPhysics
{
   class Formation;
}

namespace DerivedProperties
{

   /// \brief Calculates the high resolution depth for a formation.
   class DepthHighResFormationCalculator : public AbstractDerivedProperties::FormationPropertyCalculator
   {

   public :

      /// \brief Input constructor.
      /// \param [in]  projectHandle   Manager for all derived properties.
      /// \pre SimulationDetails is not null, ie there is at least one last run.
      DepthHighResFormationCalculator( const GeoPhysics::ProjectHandle * projectHandle );

      /// \brief Calculate the high resolution depth for the formation at the snapshot age.
      /// \param [in]  propertyManager   Manager for all derived properties.
      /// \param [in]  snapshot          The snapshot at which the property is to be calculated.
      /// \param [in]  formation         The formation for which the property is to be calculated.
      /// \param [out] derivedProperties On exit will contain the newly computed high resolution depth.
      /// \pre snapshot is not null and is a valid snapshot age.
      /// \pre formation is not null and is a valid formation.
      virtual void calculate (       AbstractDerivedProperties::AbstractPropertyManager & propertyManager,
                               const DataModel::AbstractSnapshot *                        snapshot,
                               const DataModel::AbstractFormation *                       formation,
                                     AbstractDerivedProperties::FormationPropertyList &   derivedProperties ) const;

   private :

      /// \brief Compute (indirectly, because we actually already have it) high resolution depth for non subsampled runs.
      void computeIndirectly(       AbstractDerivedProperties::AbstractPropertyManager & propertyManager,
                              const DataModel::AbstractSnapshot *                        snapshot,
                              const DataModel::AbstractFormation *                       formation,
                                    AbstractDerivedProperties::FormationPropertyList &   derivedProperties ) const;

      /// \brief Initialize the formation top surface depending on the formation above (if any)
      void initializeTopSurface(       AbstractDerivedProperties::AbstractPropertyManager &     propertyManager,
                                 const DataModel::AbstractProperty *                            depthHighResProperty,
                                 const DataModel::AbstractSnapshot *                            snapshot,
                                 const GeoPhysics::Formation *                                  currentFormation,
                                       DerivedFormationPropertyPtr &                            depthHighRes ) const;

      /// \brief Compute depth high res for mantle
      void computeForMantle( const DataModel::AbstractSnapshot * snapshot,
                                   DerivedFormationPropertyPtr & depthHighRes ) const;

      /// \brief Compute high resolution depth for coupled runs with non-geometric loops active.
      void computeForCoupledRunWithNonGeometricLoop( const GeoPhysics::Formation *                            formation,
                                                     const DataModel::AbstractSnapshot *                      snapshot,
                                                           DerivedFormationPropertyPtr &                      depthHighRes,
                                                           AbstractDerivedProperties::FormationPropertyList & derivedProperties ) const;

      /// \brief Compute high resolution depth for subsampled runs.
      void computeForSubsampledRun(       AbstractDerivedProperties::AbstractPropertyManager & propertyManager,
                                    const GeoPhysics::Formation *                              formation,
                                    const DataModel::AbstractSnapshot *                        snapshot,
                                          DerivedFormationPropertyPtr &                        depthHighRes,
                                          AbstractDerivedProperties::FormationPropertyList &   derivedProperties ) const;

      const GeoPhysics::ProjectHandle * const m_projectHandle; //!< Project handle

      bool m_isCoupledMode;                                    //!< Boolean flag for coupled mode

      const bool m_isSubsampled;                               //!< Boolean flag for subsampled 

      const bool m_isNonGeometricLoopActive;                   //!< Boolean flag for non geometric loops

   };

}

#endif // DERIVED_PROPERTIES__DEPTH_HIGHRES_FORMATION_CALCULATOR_H
