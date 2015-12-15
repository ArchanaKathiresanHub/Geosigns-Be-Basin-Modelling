#ifndef _GEOPHYSICS__COMPOUND_PROPERTY_H_
#define _GEOPHYSICS__COMPOUND_PROPERTY_H_

#include "GeoPhysicalConstants.h"

namespace GeoPhysics {


   /// Compound property is used to hold the individual and mixed values
   /// of a compound lithology, e.g. when computing permeability, each of
   /// the individual simple lithology porosities is required.
   class CompoundProperty {

   public :

      double operator ()( const int position ) const;

      double& operator ()( const int position );

      void setMixedProperty ( const double newValue );

      double mixedProperty () const;

   private :

      double m_individualProperties [ MaximumNumberOfLithologies ];
      double m_mixedPropertyValue;

   };


}

//------------------------------------------------------------//
//   Inline functions.
//------------------------------------------------------------//


inline double GeoPhysics::CompoundProperty::operator ()( const int position ) const {
   return m_individualProperties [ position ];
}

inline double& GeoPhysics::CompoundProperty::operator ()( const int position ) {
   return m_individualProperties [ position ];
}

inline double GeoPhysics::CompoundProperty::mixedProperty () const {
   return m_mixedPropertyValue;
}


#endif // _GEOPHYSICS__COMPOUND_PROPERTY_H_
