#ifndef _DERIVED_PROPERTIES__MOCK_PROPERTY_H_
#define _DERIVED_PROPERTIES__MOCK_PROPERTY_H_

#include <string>

#include "AbstractProperty.h"
#include "PropertyAttribute.h"

namespace DataModel {

   /// \brief Describes a property defined in the cauldron model.
   class MockProperty : public AbstractProperty {

   public :

      MockProperty ( const std::string&      name,
                     const PropertyAttribute attr = DataModel::UNKNOWN_PROPERTY_ATTRIBUTE );

      /// \brief Get the name of the property.
      virtual const std::string& getName () const;

      /// \brief Get the PropertyAttribute of the property.
      virtual PropertyAttribute getPropertyAttribute () const;

   private :

      const std::string m_name;
      PropertyAttribute m_propertyAttribute;

   };

} // namespace DataModel

inline DataModel::MockProperty::MockProperty ( const std::string& name,
                                               const PropertyAttribute attr ) : m_name ( name ), m_propertyAttribute ( attr ) {
}

inline const std::string& DataModel::MockProperty::getName () const {
   return m_name;
}

inline DataModel::PropertyAttribute DataModel::MockProperty::getPropertyAttribute () const {
   return m_propertyAttribute;
}

#endif // _DERIVED_PROPERTIES__DERIVED_PROPERTY_H_
