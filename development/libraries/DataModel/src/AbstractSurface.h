#ifndef _DATA_MODEL__ABSTRACT_SURFACE_H_
#define _DATA_MODEL__ABSTRACT_SURFACE_H_

#include <string>

namespace DataModel {

   /// \brief 
   class AbstractSurface {

   public :

      virtual ~AbstractSurface () {}

      /// \brief Get the name of the surface.
      virtual const std::string& getName () const = 0;

   };

} // namespace DataModel

#endif // _DATA_MODEL__ABSTRACT_SURFACE_H_
