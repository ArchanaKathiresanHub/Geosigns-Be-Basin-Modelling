#ifndef _DATA_MODEL__ABSTRACT_PROPERTY_H_
#define _DATA_MODEL__ABSTRACT_PROPERTY_H_

#include <string>
#include <vector>

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

      /// \brief Get the PropertyOutputAttribute of the property.
      virtual PropertyOutputAttribute getPropertyOutputAttribute () const = 0;

   };

   typedef std::vector<const AbstractProperty*> AbstractPropertyList;

} // namespace DataModel

#endif // _DATA_MODEL__ABSTRACT_PROPERTY_H_
