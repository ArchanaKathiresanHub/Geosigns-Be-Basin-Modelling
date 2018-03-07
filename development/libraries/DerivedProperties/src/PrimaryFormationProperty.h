//
// Copyright (C) 2016-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef DERIVED_PROPERTIES__PRIMARY_FORMATION_PROPERTY_H
#define DERIVED_PROPERTIES__PRIMARY_FORMATION_PROPERTY_H

#include <boost/shared_ptr.hpp>
#include <boost/multi_array.hpp>

#include "AbstractProperty.h"
#include "AbstractSnapshot.h"
#include "AbstractFormation.h"
#include "AbstractGrid.h"

#include "Interface/GridMap.h"
#include "Interface/PropertyValue.h"

#include "FormationProperty.h"

namespace DerivedProperties {

   /// \brief Contains the values of the property at the snapshot time for the formation.
   ///
   /// The values are stored in a two dimensional array.
   class PrimaryFormationProperty : public AbstractDerivedProperties::FormationProperty  {

   public :

      PrimaryFormationProperty ( const DataAccess::Interface::PropertyValue* propertyValue );

      ~PrimaryFormationProperty();

      /// \brief Get the value at the position (ascending order).
      virtual double get ( unsigned int i,
                           unsigned int j,
                           unsigned int k ) const;

      /// \ brief Get the gridMap
      const DataAccess::Interface::GridMap* getGridMap() const;

      /// \brief Get the undefined value.
      virtual double getUndefinedValue () const;

      /// \brief Determine wether or not the data has been retrieved.
      virtual bool isRetrieved () const;

      /// \brief Retreive the grid map
      virtual void retrieveData () const;

      /// \brief Restore the grid map.
      virtual void restoreData () const;

      /// \brief Return true.
      bool isPrimary () const;

   private :

      const DataAccess::Interface::PropertyValue* m_propertyValue;
      const DataAccess::Interface::GridMap* m_gridMap;
      unsigned int m_depth;
      bool m_storedInAscending;

   };

   typedef boost::shared_ptr<PrimaryFormationProperty> PrimaryFormationPropertyPtr;

}

inline bool DerivedProperties::PrimaryFormationProperty::isPrimary () const {

   return true;
}

#endif // DERIVED_PROPERTIES__PRIMARY_FORMATION_PROPERTY_H
