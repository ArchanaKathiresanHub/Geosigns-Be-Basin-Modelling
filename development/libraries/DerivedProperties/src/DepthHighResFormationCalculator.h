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
   class DepthHighResFormationCalculator : public FormationPropertyCalculator
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
      virtual void calculate (       AbstractPropertyManager &      propertyManager,
                               const DataModel::AbstractSnapshot *  snapshot,
                               const DataModel::AbstractFormation * formation,
                                     FormationPropertyList &        derivedProperties ) const;

   private :

      /// \brief Compute (indirectly, because we actually already have it) high resolution depth for coupled runs or non subsampled runs.
      void computeIndirectly(       AbstractPropertyManager &      propertyManager,
                              const DataModel::AbstractSnapshot *  snapshot,
                              const DataModel::AbstractFormation * formation,
                                    FormationPropertyList &        derivedProperties ) const;
      
      /// \brief Initialize the formation top surface depending on the formation above (if any)
      void initializeTopSurface(       AbstractPropertyManager &     propertyManager,
                                 const DataModel::AbstractProperty * depthHighResProperty,
                                 const DataModel::AbstractSnapshot * snapshot,
                                 const GeoPhysics::Formation *       currentFormation,
                                       DerivedFormationPropertyPtr & depthHighRes ) const;
      
      /// \brief Compute depth high res for mantle
      void computeForMantle( const DataModel::AbstractSnapshot * snapshot,
                                   DerivedFormationPropertyPtr & depthHighRes ) const;

      /// \brief Compute high resolution depth for coupled runs with non-geometric loops active.
      void computeForCoupledRunWithNonGeometricLoop( const GeoPhysics::Formation *       formation,
                                                     const DataModel::AbstractSnapshot * snapshot,
                                                           DerivedFormationPropertyPtr & depthHighRes,
                                                           FormationPropertyList &       derivedProperties ) const;

      /// \brief Compute high resolution depth for subsampled runs.
      void computeForSubsampledRun(       AbstractPropertyManager &     propertyManager,
                                    const GeoPhysics::Formation *       formation,
                                    const DataModel::AbstractSnapshot * snapshot,
                                          DerivedFormationPropertyPtr & depthHighRes,
                                          FormationPropertyList &       derivedProperties ) const;

      const GeoPhysics::ProjectHandle * const m_projectHandle; //!< Project handle

      bool m_isCoupledMode;                                    //!< Boolean flag for coupled mode

      const bool m_isSubsampled;                               //!< Boolean flag for subsampled 

      const bool m_isNonGeometricLoopActive;                   //!< Boolean flag for non geometric loops

   };

}

#endif // DERIVED_PROPERTIES__DEPTH_HIGHRES_FORMATION_CALCULATOR_H
