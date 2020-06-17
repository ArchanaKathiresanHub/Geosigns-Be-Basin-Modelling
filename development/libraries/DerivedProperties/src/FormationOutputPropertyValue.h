//
// Copyright (C) 2013-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef DERIVED_PROPERTIES__FORMATION_OUTPUT_PROPERTY_VALUE__H
#define DERIVED_PROPERTIES__FORMATION_OUTPUT_PROPERTY_VALUE__H

#include "OutputPropertyValue.h"

#include "AbstractProperty.h"
#include "AbstractSnapshot.h"
#include "AbstractFormation.h"

#include "AbstractPropertyManager.h"
#include "FormationProperty.h"

namespace DerivedProperties {

   /// \brief Stores the Formation property to be calculated and saved to disk
   class FormationOutputPropertyValue : public OutputPropertyValue {

   public :

      /// \brief Allocate the Formation property 
      FormationOutputPropertyValue ( AbstractDerivedProperties::AbstractPropertyManager& propertyManager,
                                     const DataModel::AbstractProperty*                  property,
                                     const DataModel::AbstractSnapshot*                  snapshot,
                                     const DataModel::AbstractFormation*                 formation );

      FormationOutputPropertyValue ( AbstractDerivedProperties::FormationPropertyPtr&    formationProperty );

      /// \brief Get the value of the property at the position i,j,k
      virtual double getValue ( const double i, const double j, const double k ) const;

      /// \brief Get the number of nodes in the z direction
      unsigned int getDepth () const;

      /// \brief Get the name of the property
      const std::string & getName() const;

      /// \brief Get the grid on which the property values are defined.
      const DataModel::AbstractGrid* getGrid () const;

      /// \brief Determine whether or not the property is a primary property
      bool isPrimary() const;

      /// \brief Retreive the grid map
      void retrieveData() const;

      /// \brief Restore the grid map
      void restoreData() const;

      /// \brief Determine whether or not the property value has a property
      bool hasProperty() const;

   private :

      /// \brief The Formation property
      AbstractDerivedProperties::FormationPropertyPtr m_formationProperty;

   };

}
#endif // DERIVED_PROPERTIES__FORMATION_OUTPUT_PROPERTY_VALUE__H
