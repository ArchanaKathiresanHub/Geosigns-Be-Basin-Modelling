//
// Copyright (C) 2016-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef DERIVED_PROPERTIES__PRIMARY_SURFACE_PROPERTY_H
#define DERIVED_PROPERTIES__PRIMARY_SURFACE_PROPERTY_H

#include <boost/multi_array.hpp>

#include "AbstractProperty.h"
#include "AbstractSnapshot.h"
#include "AbstractSurface.h"
#include "AbstractGrid.h"


#include "GridMap.h"
#include "PropertyValue.h"

#include "SurfaceProperty.h"

#include <memory>

namespace DerivedProperties {

   /// \brief Contains the values of the property at the snapshot time for the surface.
   ///
   /// The values are stored in a two dimensional array.
   class PrimarySurfaceProperty : public AbstractDerivedProperties::SurfaceProperty {


   public :

      PrimarySurfaceProperty ( const DataAccess::Interface::PropertyValue* propertyValue );

      ~PrimarySurfaceProperty();

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

       /// \brief Get the average value of the map.
      double getAverageValue () const;


      /// \ brief Get the gridMap
      const DataAccess::Interface::GridMap* getGridMap() const;

      /// \brief Return true.
      bool isPrimary () const;
  private :

      const DataAccess::Interface::PropertyValue* m_propertyValue;
      const DataAccess::Interface::GridMap* m_gridMap;

   };

   typedef std::shared_ptr<PrimarySurfaceProperty> PrimarySurfacePropertyPtr;

}

inline bool DerivedProperties::PrimarySurfaceProperty::isPrimary () const {

   return true;

}

#endif // DERIVED_PROPERTIES__PRIMARY_SURFACE_PROPERTY_H

