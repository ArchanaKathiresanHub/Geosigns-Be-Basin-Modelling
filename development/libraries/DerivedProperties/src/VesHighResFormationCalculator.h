#ifndef DERIVED_PROPERTIES__VES_HIGHRES_FORMATION_CALCULATOR_H
#define DERIVED_PROPERTIES__VES_HIGHRES_FORMATION_CALCULATOR_H

#include "FormationPropertyCalculator.h"
#include "GeoPhysicsProjectHandle.h"
#include "AbstractPropertyManager.h"
#include "AbstractSnapshot.h"
#include "AbstractFormation.h"
#include "FormationProperty.h"
#include "DerivedFormationProperty.h"

namespace DerivedProperties {

   /// \brief Calculates the high resolution VES for a formation.
   class VesHighResFormationCalculator : public FormationPropertyCalculator
   {

   public :
      
      /// \brief Input constructor.
      /// \param [in]  projectHandle   Manager for all derived properties.
      /// \pre SimulationDetails is not null, ie there is at least one last run.
      VesHighResFormationCalculator( const GeoPhysics::ProjectHandle * projectHandle );

      /// \brief Calculate the high resolution VES for the formation at the snapshot age.
      /// \param [in]  propertyManager   Manager for all derived properties.
      /// \param [in]  snapshot          The snapshot at which the property is to be calculated.
      /// \param [in]  formation         The formation for which the property is to be calculated.
      /// \param [out] derivedProperties On exit will contain the newly computed high resolution VES.
      /// \pre snapshot is not null and is a valid snapshot age.
      /// \pre formation is not null and is a valid formation.
      virtual void calculate (       AbstractPropertyManager &      propertyManager,
                               const DataModel::AbstractSnapshot *  snapshot,
                               const DataModel::AbstractFormation * formation,
                                     FormationPropertyList &        derivedProperties ) const;

   private :

      /// \brief Compute (indirectly, because we actually already have it) high resolution VES for coupled runs or non subsampled runs.
      void computeIndirectly(       AbstractPropertyManager &      propertyManager,
                              const DataModel::AbstractSnapshot *  snapshot,
                              const DataModel::AbstractFormation * formation,
                                    FormationPropertyList &        derivedProperties ) const;

      /// \brief Compute high resolution VES for subsampled runs (except for coupled runs).
      void computeForSubsampledRun(       AbstractPropertyManager &      propertyManager,
                                    const DataModel::AbstractSnapshot *  snapshot,
                                    const DataModel::AbstractFormation * formation,
                                          FormationPropertyList &        derivedProperties ) const;
      
      /// \brief Initialize the formation top surface depending on the formation above (if any)
      void initializeTopSurface(       AbstractPropertyManager &      propertyManager,
                                 const DataModel::AbstractProperty *  vesHighResProperty,
                                 const DataModel::AbstractSnapshot *  snapshot,
                                 const DataModel::AbstractFormation * formationAbove,
                                       DerivedFormationPropertyPtr &  vesHighRes ) const;

      const GeoPhysics::ProjectHandle * const m_projectHandle; //!< Project handle

      bool m_isCoupledMode;                                    //!< Boolean flag for coupled mode

      const bool m_isSubsampled;                               //!< Boolean flag for subsampled grids

   };

}

#endif // DERIVED_PROPERTIES__VES_HIGHRES_FORMATION_CALCULATOR_H
