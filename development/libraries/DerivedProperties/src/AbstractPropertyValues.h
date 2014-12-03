#ifndef DERIVED_PROPERTIES__ABSTRACT_PROPERTY_VALUES_H
#define DERIVED_PROPERTIES__ABSTRACT_PROPERTY_VALUES_H

#include <boost/shared_ptr.hpp>

namespace DerivedProperties {

   /// \brief Base class for property values.
   class AbstractPropertyValues {

   public :

      /// \brief Destructor.
      virtual ~AbstractPropertyValues () {}

      /// \brief Get the undefined value.
      virtual double getUndefinedValue () const = 0;

      /// \brief Determine if the map has been retrieved or not.
      virtual bool isRetrieved () const;

      /// \brief Retreive the grid map
      virtual void retrieveData () const;

      /// \brief Restore the grid map.
      virtual void restoreData () const;

   };

   typedef boost::shared_ptr<const AbstractPropertyValues> AbstractPropertyValuesPtr;


}

//
// Inline functions.
//

inline void DerivedProperties::AbstractPropertyValues::retrieveData () const {
}

inline bool DerivedProperties::AbstractPropertyValues::isRetrieved () const {
   return true;
}

inline void DerivedProperties::AbstractPropertyValues::restoreData () const {
}

#endif // DERIVED_PROPERTIES__ABSTRACT_PROPERTY_VALUES_H
