#ifndef _DATA_MODEL__ABSTRACT_PROPERTY_H_
#define _DATA_MODEL__ABSTRACT_PROPERTY_H_

#include <string>

namespace DataModel {

   class AbstractProperty {

   public :

      virtual ~AbstractProperty () {}

      virtual const std::string& getName () const = 0;

   };

} // namespace DataModel

#endif // _DATA_MODEL__ABSTRACT_PROPERTY_H_
