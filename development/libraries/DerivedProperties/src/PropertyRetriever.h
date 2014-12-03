#ifndef DERIVED_PROPERTIES__PROPERTY_RETRIEVER_H
#define DERIVED_PROPERTIES__PROPERTY_RETRIEVER_H

#include "AbstractPropertyValues.h"

namespace DerivedProperties {

   /// \brief Simplifies the retrieving and restoring of property values.
   class PropertyRetriever {

   public :

      /// \brief Constructor.
      ///
      /// \param [in] propertyValues The property value to be retrieved.
      /// The values will be retrieved only if they are in the un-retrieved state.
      PropertyRetriever ( const AbstractPropertyValuesPtr& propertyValues );

      /// \brief Destructor.
      ///
      /// On destruction the property value is restored if it were in the
      /// un-retrieved state when an object of this type was created.
      ~PropertyRetriever ();

   private :

      /// \brief The property values to be retrieved.
      const AbstractPropertyValuesPtr& m_propertyValues;

      /// \brief Indicate whether or not the property values should be restored on destruction of this object.
      bool  m_restoreOnExit;

   };

}

#endif // DERIVED_PROPERTIES__PROPERTY_RETRIEVER_H
