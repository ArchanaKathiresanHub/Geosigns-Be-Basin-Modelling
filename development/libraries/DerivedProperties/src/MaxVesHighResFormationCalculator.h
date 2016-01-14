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
   class MaxVesHighResFormationCalculator : public FormationPropertyCalculator
   {

   public :
      
      /// \brief Input constructor.
      /// \param [in]  projectHandle   Manager for all derived properties.
      /// \pre SimulationDetails is not null, ie there is at least one last run.
      MaxVesHighResFormationCalculator( const GeoPhysics::ProjectHandle * projectHandle );

      /// \brief Calculate the high resolution max VES for the formation at the snapshot age.
      /// \param [in]  propertyManager   Manager for all derived properties.
      /// \param [in]  snapshot          The snapshot at which the property is to be calculated.
      /// \param [in]  formation         The formation for which the property is to be calculated.
      /// \param [out] derivedProperties On exit will contain the newly computed high resolution max VES.
      /// \pre snapshot is not null and is a valid snapshot age.
      /// \pre formation is not null and is a valid formation.
      virtual void calculate (       AbstractPropertyManager &      propertyManager,
                               const DataModel::AbstractSnapshot *  snapshot,
                               const DataModel::AbstractFormation * formation,
                                     FormationPropertyList &        derivedProperties ) const;

   private :

      /// \brief Compute (indirectly, because we actually already have it) high resolution max VES for coupled runs or non subsampled runs.
      void computeIndirectly(       AbstractPropertyManager &      propertyManager,
                              const DataModel::AbstractSnapshot *  snapshot,
                              const DataModel::AbstractFormation * formation,
                                    FormationPropertyList &        derivedProperties ) const;

      /// \brief Compute high resolution max VES for subsampled runs (except for coupled runs).
      void computeForSubsampledRun(       AbstractPropertyManager &      propertyManager,
                                    const DataModel::AbstractSnapshot *  snapshot,
                                    const DataModel::AbstractFormation * formation,
                                          FormationPropertyList &        derivedProperties ) const;

      const GeoPhysics::ProjectHandle * const m_projectHandle; //!< Project handle

      bool m_isCoupledMode;                                    //!< Boolean flag for coupled mode

      const bool m_isSubsampled;                               //!< Boolean flag for subsampled grids

   };

}

#endif // DERIVED_PROPERTIES__MAX_VES_HIGHRES_FORMATION_CALCULATOR_H