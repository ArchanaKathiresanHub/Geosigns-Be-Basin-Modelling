//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef DERIVED_PROPERTIES__PROPERTY_RETRIEVER_H
#define DERIVED_PROPERTIES__PROPERTY_RETRIEVER_H

#include "AbstractPropertyValues.h"

namespace DerivedProperties {

   /// \brief Simplifies the retrieving and restoring of property values.
   class PropertyRetriever {

   public :

      /// \brief Constructor.
      PropertyRetriever ();

      /// \brief Constructor.
      ///
      /// \param [in] propertyValues The property value to be retrieved.
      /// The values will be retrieved only if they are in the un-retrieved state.
      PropertyRetriever ( const AbstractDerivedProperties::AbstractPropertyValuesPtr& propertyValues );

      /// \brief Destructor.
      ///
      /// On destruction the property value is restored if it were in the
      /// un-retrieved state when an object of this type was created.
      ~PropertyRetriever ();

      /// \brief Reset the property.
      ///
      /// \param [in] propertyValues The property value to be retrieved.
      /// The values will be retrieved only if they are in the un-retrieved state.
      void reset ( const AbstractDerivedProperties::AbstractPropertyValuesPtr& propertyValues );

   private :

      /// \brief The property values to be retrieved.
      AbstractDerivedProperties::AbstractPropertyValuesPtr m_propertyValues;

      /// \brief Indicate whether or not the property values should be restored on destruction of this object.
      bool  m_restoreOnExit;

   };

}

#endif // DERIVED_PROPERTIES__PROPERTY_RETRIEVER_H
