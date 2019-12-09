//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef DERIVED_PROPERTIES__HEATFLOW_CALCULATOR_H
#define DERIVED_PROPERTIES__HEATFLOW_CALCULATOR_H

#include "FormationPropertyCalculator.h"
#include "DerivedFormationProperty.h"

#include "GeoPhysicsProjectHandle.h"
#include "CompoundLithologyArray.h"
#include "GeoPhysicsFormation.h"

#include "FiniteElementTypes.h"
#include "FiniteElement.h"

using namespace FiniteElementMethod;

namespace DerivedProperties {

   class HeatFlowFormationCalculator : public AbstractDerivedProperties::FormationPropertyCalculator {

   public :

      HeatFlowFormationCalculator ( const GeoPhysics::ProjectHandle& projectHandle );

      /// \brief Calculate the permeability for the formation.
      ///
      /// \param [in]  propManager The property manager.
      /// \param [in]  snapshot    The snapshot time for which the permeability is requested.
      /// \param [in]  formation   The formation for which the permeability is requested.
      /// \param [out] derivedProperties On exit will contain two formation properties, the horizontal and vertical permeability of the layer.
      /// \pre snapshot points to a valid snapshot age.
      /// \pre formation points to a valid formation.
      virtual void calculate (       AbstractDerivedProperties::AbstractPropertyManager& propManager,
                               const DataModel::AbstractSnapshot*                        snapshot,
                               const DataModel::AbstractFormation*                       formation,
                                     AbstractDerivedProperties::FormationPropertyList&   derivedProperties ) const;

      virtual bool isComputable ( const AbstractDerivedProperties::AbstractPropertyManager& propManager,
                                  const DataModel::AbstractSnapshot*                        snapshot,
                                  const DataModel::AbstractFormation*                       formation ) const;

      virtual bool isComputableForBasement ( const AbstractDerivedProperties::AbstractPropertyManager& propManager,
                                             const DataModel::AbstractSnapshot*                        snapshot,
                                             const DataModel::AbstractFormation*                       formation ) const;


   private :

      const GeoPhysics::ProjectHandle& m_projectHandle;
      bool   m_chemicalCompactionRequired;
      double m_globalXNode;
      double m_globalYNode;

      void computeHeatFlow ( const bool                     isBasementFormation,
                             const bool                     isBasalt,
                             const double                   z,
                             const GeoPhysics::CompoundLithology* lithology,
                             const GeoPhysics::FluidType*   fluid,
                             const bool                     includeChemicalCompaction,
                             const bool                     includeAdvectionTerm,
                             const ElementGeometryMatrix&  geometryMatrix,
                             const ElementVector&          currentElementVES,
                             const ElementVector&          currentElementMaxVES,
                             const ElementVector&          currentElementPo,
                             const ElementVector&          currentElementPp,
                             const ElementVector&          currentElementLp,
                             const ElementVector&          currentElementTemperature,
                             const ElementVector&          currentElementChemicalCompaction,
                             ThreeVector&                  heatFlow ) const;

      void fillBorders( unsigned int i, unsigned int j, unsigned int k,
                        unsigned int globalXNodes,
                        unsigned int globalYNodes,
                        DerivedFormationPropertyPtr heatFlowX,
                        DerivedFormationPropertyPtr heatFlowY,
                        DerivedFormationPropertyPtr heatFlowZ,
                        ThreeVector heatFlow ) const;

      bool isDegenerate ( const ElementGeometryMatrix&  geometryMatrix ) const;
   };


}

#endif
