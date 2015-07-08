#ifndef _DERIVED_PROPERTIES__MOCK_PROPERTY_H_
#define _DERIVED_PROPERTIES__MOCK_PROPERTY_H_

#include <string>

#include "AbstractProperty.h"

namespace DataModel {

   /// \brief Describes a property defined in the cauldron model.
   class MockProperty : public AbstractProperty {

   public :

      MockProperty ( const std::string& name );

      /// \brief Get the name of the property.
      virtual const std::string& getName () const;

   private :

      const std::string m_name;

   };

} // namespace DataModel

inline DataModel::MockProperty::MockProperty ( const std::string& name ) : m_name ( name ) {
}

inline const std::string& DataModel::MockProperty::getName () const {
   return m_name;
}


#endif // _DERIVED_PROPERTIES__DERIVED_PROPERTY_H_
