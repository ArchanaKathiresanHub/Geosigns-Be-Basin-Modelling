#ifndef DERIVED_PROPERTIES__INTERPOLATE_2D_PROPERTY__H
#define DERIVED_PROPERTIES__INTERPOLATE_2D_PROPERTY__H

namespace DerivedProperties {

   template<class PropertyValue>
   class Interpolate2DProperty {

   public :

      double interpolate ( const PropertyValue& property,
                           const double         i,
                           const double         j ) const;

   };

}


template<class PropertyValue>
double DerivedProperties::Interpolate2DProperty<PropertyValue>::interpolate ( const PropertyValue& property,
                                                                              const double         i,
                                                                              const double         j ) const {

   double undefinedValue = property.getUndefinedValue ();

   if ( i < 0.0 or j < 0.0 ) {
      return undefinedValue;
   }

   const double MinOffset = 1e-6;

   double values [ 4 ];
   double weight [ 4 ];

   unsigned int baseI = static_cast<unsigned int>(i);
   unsigned int baseJ = static_cast<unsigned int>(j);

   double fractionI = i - static_cast<double>( baseI );
   double fractionJ = j - static_cast<double>( baseJ );

   if ( fractionI <= MinOffset ) {
      fractionI = 0.0;
   }

   if ( fractionJ <= MinOffset ) {
      fractionJ = 0.0;
   }

   if ( fractionI >= 1 - MinOffset ) {
      fractionI = 1.0;
   }

   if ( fractionJ >= 1 - MinOffset ) {
      fractionJ = 1.0;
   }

   if ( fractionI == 0.0 and fractionJ == 0 ) {
      return property.get ( baseI, baseJ );
   }

   weight [ 0 ] = (1.0 - fractionI) * (1.0 - fractionJ);
   weight [ 1 ] = fractionI         * (1.0 - fractionJ);
   weight [ 2 ] = fractionI         * fractionJ;
   weight [ 3 ] = (1.0 - fractionI) * fractionJ;

   values [ 0 ] = property.get ( baseI,     baseJ );
   values [ 1 ] = property.get ( baseI + 1, baseJ );
   values [ 2 ] = property.get ( baseI + 1, baseJ + 1 );
   values [ 3 ] = property.get ( baseI,     baseJ + 1 );

   double result = 0;

   for ( int i = 0; i < 4; ++i ) {

      if ( weight [ i ] != 0.0 and values [ i ] == undefinedValue ) {
         result = undefinedValue;
         break;
      }

      result += values [ i ] * weight [ i ];
   }

   return result;
}

#endif // DERIVED_PROPERTIES__INTERPOLATE_2D_PROPERTY__H
