//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef DERIVED_PROPERTIES__PRIMARY_RESERVOIR_PROPERTY_H
#define DERIVED_PROPERTIES__PRIMARY_RESERVOIR_PROPERTY_H

#include <boost/shared_ptr.hpp>
#include <boost/multi_array.hpp>

#include "AbstractProperty.h"
#include "AbstractSnapshot.h"
#include "AbstractFormation.h"
#include "AbstractGrid.h"


#include "Interface/GridMap.h"
#include "Interface/PropertyValue.h"

#include "ReservoirProperty.h"

namespace DerivedProperties {

   /// \brief Contains the values of the property at the snapshot time for the surface.
   ///
   /// The values are stored in a two dimensional array.
   class PrimaryReservoirProperty : public AbstractDerivedProperties::ReservoirProperty {


   public :

      PrimaryReservoirProperty ( const DataAccess::Interface::PropertyValue* propertyValue );

      ~PrimaryReservoirProperty();

      /// \ brief Get the gridMap
      const DataAccess::Interface::GridMap* getGridMap() const;

      /// \brief Get the value at the position.
      virtual double get ( unsigned int i,
                           unsigned int j ) const;

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

   };

   typedef boost::shared_ptr<PrimaryReservoirProperty> PrimaryReservoirPropertyPtr;

}

inline bool DerivedProperties::PrimaryReservoirProperty::isPrimary () const {

   return true;

}

inline const DataAccess::Interface::GridMap* DerivedProperties::PrimaryReservoirProperty::getGridMap() const {

   return m_gridMap;
}

#endif // DERIVED_PROPERTIES__PRIMARY_RESERVOIR_PROPERTY_H
