//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef ABSTRACTDERIVED_PROPERTIES__ABSTRACT_PROPERTY_VALUES_H
#define ABSTRACTDERIVED_PROPERTIES__ABSTRACT_PROPERTY_VALUES_H

#include <memory>

namespace AbstractDerivedProperties {

   /// \brief Base class for property values.
   class AbstractPropertyValues {

   public :

      /// \brief Destructor.
      virtual ~AbstractPropertyValues () = default;

      /// \brief Get the undefined value.
      virtual double getUndefinedValue () const = 0;

      /// \brief Determine if the map has been retrieved or not.
      virtual bool isRetrieved () const;

      /// \brief Retreive the grid map
      virtual void retrieveData () const;

      /// \brief Restore the grid map.
      virtual void restoreData () const;

   };

   typedef std::shared_ptr<const AbstractPropertyValues> AbstractPropertyValuesPtr;


}

//
// Inline functions.
//

inline void AbstractDerivedProperties::AbstractPropertyValues::retrieveData () const {
}

inline bool AbstractDerivedProperties::AbstractPropertyValues::isRetrieved () const {
   return true;
}

inline void AbstractDerivedProperties::AbstractPropertyValues::restoreData () const {
}

#endif // ABSTRACTDERIVED_PROPERTIES__ABSTRACT_PROPERTY_VALUES_H
