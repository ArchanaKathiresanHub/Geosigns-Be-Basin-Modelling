#ifndef _DATA_MODEL__ABSTRACT_SURFACE_H_
#define _DATA_MODEL__ABSTRACT_SURFACE_H_

#include <string>

namespace DataModel {

   class AbstractSurface {

   public :

      virtual ~AbstractSurface () {}

      virtual const std::string& getName () const = 0;

   };

} // namespace DataModel

#endif // _DATA_MODEL__ABSTRACT_SURFACE_H_
