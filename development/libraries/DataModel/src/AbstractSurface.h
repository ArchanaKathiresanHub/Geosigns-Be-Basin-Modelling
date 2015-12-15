#ifndef _DATA_MODEL__ABSTRACT_SURFACE_H_
#define _DATA_MODEL__ABSTRACT_SURFACE_H_

#include <string>

#include "AbstractFormation.h"

namespace DataModel {

   /// \brief 
   class AbstractSurface {

   public :

      virtual ~AbstractSurface () {}

      /// \brief Get the name of the surface.
      virtual const std::string& getName () const = 0;

      /// \brief Return the name of the Formation found above this Surface if there is one.
      ///
      /// If there is no formation above then a null string ("") will be returned.
      virtual const std::string& getTopFormationName () const = 0;

      /// \brief Return the name of the Formation found below this Surface if there is one.
      ///
      /// If there is no formation below then a null string ("") will be returned.
      virtual const std::string& getBottomFormationName () const = 0;

      /// \brief Return the formation that lies directly above the surface.
      ///
      /// If there is none, i.e. when the surface is the top surface of the domain,
      /// then a null value will be returned.
      virtual const AbstractFormation* getTopFormation () const = 0;

      /// \brief Return the formation that lies directly below the surface.
      ///
      /// If there is none, i.e. when the surface is the bottom surface of the basement,
      /// then a null value will be returned.
      virtual const AbstractFormation* getBottomFormation () const = 0;


   };

} // namespace DataModel

#endif // _DATA_MODEL__ABSTRACT_SURFACE_H_
