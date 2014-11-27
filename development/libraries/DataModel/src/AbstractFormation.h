#ifndef _DATA_MODEL__ABSTRACT_FORMATION_H_
#define _DATA_MODEL__ABSTRACT_FORMATION_H_

#include <string>

namespace DataModel {

   /// \brief A formation.
   class AbstractFormation {

   public :

      virtual ~AbstractFormation () {}

      /// \brief Get the name of the formation.
      virtual const std::string& getName () const = 0;

      /// \brief Return the name of the Surface at the top of this Formation.
      ///
      /// If there is no surface above then a null string ("") will be returned.
      virtual const std::string& getTopSurfaceName () const = 0;

      /// \brief Return the name of the Surface at the bottom of this Formation.
      ///
      /// If there is no surface below then a null string ("") will be returned.
      virtual const std::string& getBottomSurfaceName () const = 0;

   };

} // namespace DataModel

#endif // _DATA_MODEL__ABSTRACT_FORMATION_H_
