//
// Copyright (C) 2021-2021 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef DERIVED_PROPERTIES_DEPTH_BELOW_MUDLINE_FORMATION_CALCULATOR_H
#define DERIVED_PROPERTIES_DEPTH_BELOW_MUDLINE_FORMATION_CALCULATOR_H

#include "FormationPropertyCalculator.h"

namespace DerivedProperties 
{
   /// \brief Calculator for the Depth-Below-Mudline for a layer
   class DepthBelowMudlineFormationCalculator : public AbstractDerivedProperties::FormationPropertyCalculator 
   {
   public :

      explicit DepthBelowMudlineFormationCalculator( const GeoPhysics::ProjectHandle& projectHandle );

      /// \brief Calculate the Depth-below-mudline for the formation.
      ///
      /// \param [in]  propManager The property manager.
      /// \param [in]  snapshot    The snapshot time for which the depth-below-mudline is requested.
      /// \param [in]  formation   The formation for which the depth-below-mudline is requested.
      /// \param [out] derivedProperties On exit will contain a single formation property, the depth below mudline of the layer.
      /// \pre snapshot points to a valid snapshot age.
      /// \pre formation points to a valid formation.
      void calculate ( AbstractDerivedProperties::AbstractPropertyManager& propManager,
                               const DataModel::AbstractSnapshot*                        snapshot,
                               const DataModel::AbstractFormation*                       formation,
                                     AbstractDerivedProperties::FormationPropertyList&   derivedProperties ) const override;

      /// \brief Determine if the property is computable for the specific combination of formation and snapshot.
      bool isComputable ( const AbstractDerivedProperties::AbstractPropertyManager& propManager,
                                  const DataModel::AbstractSnapshot*                        snapshot,
                                  const DataModel::AbstractFormation*                       formation ) const override;

   private :

      const GeoPhysics::ProjectHandle& m_projectHandle;
   };


}

#endif
