#ifndef _DATA_MODEL__ABSTRACT_PROPERTY_H_
#define _DATA_MODEL__ABSTRACT_PROPERTY_H_

#include <string>

#include "PropertyAttribute.h"

namespace DataModel {

   /// \brief Describes a property defined in the cauldron model.
   class AbstractProperty {

   public :

      virtual ~AbstractProperty () {}

      /// \brief Get the name of the property.
      virtual const std::string& getName () const = 0;

      /// \brief Get the PropertyAttribute of the property.
      virtual PropertyAttribute getPropertyAttribute () const = 0;

   };

} // namespace DataModel

#endif // _DATA_MODEL__ABSTRACT_PROPERTY_H_
