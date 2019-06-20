#ifndef _DATAACCESS_INTERPOLATED_PROPERTY_VALUES_H_
#define _DATAACCESS_INTERPOLATED_PROPERTY_VALUES_H_

#include <map>

#include "Property.h"

#include "ElementPosition.h"

namespace DataAccess {

   namespace Mining {

      /// Contains a mapping from property to a floating-point number.
      ///
      /// The floating-point number will usually be the property 
      /// interpolated at some point in space.
      class InterpolatedPropertyValues {

         /// \typedef PropertyEvaluation
         /// 
         /// Provide a mapping from the property to its interpolated value at a point.
         typedef std::map <const Interface::Property*, double > PropertyEvaluation;

      public :

         InterpolatedPropertyValues ();
         ~InterpolatedPropertyValues ();

         /// Clears all stored properties and associated interpolated values.
         void clear ();

         void setElement ( const ElementPosition& elem );

         const ElementPosition& getElement () const;

         /// Return the value for the property.
         ///
         /// If the property is not found in the mapping, then the null-value will be returned.
         double operator ()( const Interface::Property* property ) const;

         /// Return the value for the property.
         ///
         /// If the property is not found in the mapping, then a new one will be added.
         /// If value already exists then it will be over-written!
         void setValue ( const Interface::Property* property,
                         const double               value );

         /// determine if this object contains the property.
         bool contains ( const Interface::Property* property ) const;

         /// Return a string representation of the object.
         std::string image () const;

      private :

         ElementPosition    m_evaluationPosition;
         PropertyEvaluation m_evaluations;

      };

   }

}

// Inline functions.

inline const DataAccess::Mining::ElementPosition& DataAccess::Mining::InterpolatedPropertyValues::getElement () const {
   return m_evaluationPosition;
}

#endif // _DATAACCESS_INTERPOLATED_PROPERTY_VALUES_H_
