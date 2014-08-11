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

   };

} // namespace DataModel

#endif // _DATA_MODEL__ABSTRACT_FORMATION_H_
