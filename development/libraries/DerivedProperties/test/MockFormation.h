#ifndef _DERIVED_PROPERTIES__MOCK_FORMATION_H_
#define _DERIVED_PROPERTIES__MOCK_FORMATION_H_

#include <string>

#include "AbstractFormation.h"

namespace DataModel {

   /// \brief A formation.
   class MockFormation : public AbstractFormation {

   public :

      MockFormation ( const std::string& name );

      /// \brief Get the name of the formation.
      virtual const std::string& getName () const;

   private :

      const std::string m_name;

   };

} // namespace DataModel

inline DataModel::MockFormation::MockFormation ( const std::string& name ) : m_name ( name ) {
}

inline const std::string& DataModel::MockFormation::getName () const {
   return m_name;
}

#endif // _DERIVED_PROPERTIES__MOCK_FORMATION_H_
