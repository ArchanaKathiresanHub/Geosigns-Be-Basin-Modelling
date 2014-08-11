#ifndef _DATA_MODEL__ABSTRACT_FORMATION_H_
#define _DATA_MODEL__ABSTRACT_FORMATION_H_

#include <string>

namespace DataModel {

   class AbstractFormation {

   public :

      virtual ~AbstractFormation () {}

      virtual const std::string& getName () const = 0;

   };

} // namespace DataModel

#endif // _DATA_MODEL__ABSTRACT_FORMATION_H_
