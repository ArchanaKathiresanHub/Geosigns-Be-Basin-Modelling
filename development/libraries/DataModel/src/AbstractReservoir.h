#ifndef _DATA_MODEL__ABSTRACT_RESERVOIR_H_
#define _DATA_MODEL__ABSTRACT_RESERVOIR_H_

#include <string>

#include "AbstractFormation.h"

namespace DataModel {

   /// \brief 
   class AbstractReservoir {

   public :

      virtual ~AbstractReservoir () {}

      /// \brief Get the name of the reservoir.
      virtual const std::string& getName () const = 0;

      /// \brief Return the name of the Formation containing this Reservoir.
      virtual const std::string& getFormationName () const = 0;

      /// \brief Return the Formation containing this Reservoir.
      virtual const AbstractFormation* getFormation () const = 0;

   };

} // namespace DataModel

#endif // _DATA_MODEL__ABSTRACT_RESERVOIR_H_
