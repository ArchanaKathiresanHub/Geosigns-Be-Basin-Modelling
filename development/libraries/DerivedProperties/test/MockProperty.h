// 
// Copyright (C) 2017-2018 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

#ifndef _DERIVED_PROPERTIES__MOCK_PROPERTY_H_
#define _DERIVED_PROPERTIES__MOCK_PROPERTY_H_

#include <string>

#include "AbstractProperty.h"
#include "PropertyAttribute.h"

namespace DataModel {

   /// \brief Describes a property defined in the cauldron model.
   class MockProperty final: public AbstractProperty {

   public :

      MockProperty ( const std::string&      name,
                     const PropertyAttribute attr = UNKNOWN_PROPERTY_ATTRIBUTE,
                     const PropertyOutputAttribute attrOut = UNKNOWN_PROPERTY_OUTPUT_ATTRIBUTE);

      /// \brief Get the name of the property.
      virtual const std::string& getName () const;

      /// \brief Get the PropertyAttribute of the property.
      virtual PropertyAttribute getPropertyAttribute () const;

      /// \brief Get the PropertyOutputAttribute of the property.
      virtual PropertyOutputAttribute getPropertyOutputAttribute () const;

   private :

      const std::string m_name;
      PropertyAttribute m_propertyAttribute;
      PropertyOutputAttribute m_propertyOutputAttribute;
   };

} // namespace DataModel

inline DataModel::MockProperty::MockProperty ( const std::string& name,
                                               const DataModel::PropertyAttribute attr,
                                               const PropertyOutputAttribute attrOut) : 
   m_name ( name ), m_propertyAttribute ( attr ), m_propertyOutputAttribute( attrOut ) {
}

inline const std::string& DataModel::MockProperty::getName () const {
   return m_name;
}

inline DataModel::PropertyAttribute DataModel::MockProperty::getPropertyAttribute () const {
   return m_propertyAttribute;
}

inline DataModel::PropertyOutputAttribute DataModel::MockProperty::getPropertyOutputAttribute () const {
   return m_propertyOutputAttribute;
}

#endif // _DERIVED_PROPERTIES__DERIVED_PROPERTY_H_
