#include "FormationProperty.h"
#include <cmath>

DerivedProperties::FormationProperty::FormationProperty ( const DataModel::AbstractProperty*  property,
                                                          const DataModel::AbstractSnapshot*  snapshot,
                                                          const DataModel::AbstractFormation* formation,
                                                          const DataModel::AbstractGrid*      grid,
                                                          const unsigned int                  nk ) :
   m_property ( property ),
   m_snapshot ( snapshot ),
   m_formation ( formation ),
   m_grid ( grid ),
   m_numberK ( nk )
{
}

DerivedProperties::FormationProperty::~FormationProperty () {
}

double DerivedProperties::FormationProperty::interpolate ( unsigned int i,
                                                           unsigned int j,
                                                           double       k ) const {

   if ( k < 0.0 ) {
      return getUndefinedValue ();
   }

   unsigned int bottomNode = static_cast<unsigned int>(std::floor ( k ));
   double fraction = k - static_cast<double> ( bottomNode );

   if ( fraction == 0.0 ) {
      return getA ( i, j, bottomNode );
   } else {
      double bottomValue = getA ( i, j, bottomNode );
      double topValue = getA ( i, j, bottomNode + 1 );

      if ( bottomValue == getUndefinedValue () or topValue == getUndefinedValue ()) {
         return getUndefinedValue ();
      } else {
         return bottomValue + fraction * ( topValue - bottomValue );
      }

   }

}

double DerivedProperties::FormationProperty::interpolate ( double i,
                                                           double j,
                                                           double k ) const {

   double undefinedValue = getUndefinedValue ();

   if ( i < 0.0 or j < 0.0 or k < 0.0 ) {
      return undefinedValue;
   }

   const double MinOffset = 1e-6;

   double values [ 8 ];
   double weight [ 8 ];

   unsigned int baseI = (unsigned int) i;
   unsigned int baseJ = (unsigned int) j;
   unsigned int baseK = (unsigned int) k;

   double fractionI = i - (double) baseI;
   double fractionJ = j - (double) baseJ;
   double fractionK = k - (double) baseK;

   if (fractionI <= MinOffset) fractionI = 0;
   if (fractionJ <= MinOffset) fractionJ = 0;
   if (fractionK <= MinOffset) fractionK = 0;

   if (fractionI >= 1 - MinOffset) fractionI = 1;
   if (fractionJ >= 1 - MinOffset) fractionJ = 1;
   if (fractionK >= 1 - MinOffset) fractionK = 1;

   if (fractionI == 0 && fractionJ == 0 && fractionK == 0)
   {
      return getA (baseI, baseJ, baseK);
   }

   weight [ 0 ] = fractionI         * fractionJ         * fractionK;
   weight [ 1 ] = fractionI         * fractionJ         * (1.0 - fractionK);
   weight [ 2 ] = fractionI         * (1.0 - fractionJ) * fractionK;
   weight [ 3 ] = fractionI         * (1.0 - fractionJ) * (1.0 - fractionK);
   weight [ 4 ] = (1.0 - fractionI) * fractionJ         * fractionK;
   weight [ 5 ] = (1.0 - fractionI) * fractionJ         * (1.0 - fractionK);
   weight [ 6 ] = (1.0 - fractionI) * (1.0 - fractionJ) * fractionK;
   weight [ 7 ] = (1.0 - fractionI) * (1.0 - fractionJ) * (1.0 - fractionK);

   values [ 0 ] = checkedGet ( baseI + 1, baseJ + 1, baseK + 1);
   values [ 1 ] = checkedGet ( baseI + 1, baseJ + 1, baseK    );
   values [ 2 ] = checkedGet ( baseI + 1, baseJ,     baseK + 1);
   values [ 3 ] = checkedGet ( baseI + 1, baseJ,     baseK    );
   values [ 4 ] = checkedGet ( baseI,     baseJ + 1, baseK + 1);
   values [ 5 ] = checkedGet ( baseI,     baseJ + 1, baseK    );
   values [ 6 ] = checkedGet ( baseI,     baseJ,     baseK + 1);
   values [ 7 ] = checkedGet ( baseI,     baseJ,     baseK    );

   if (fractionI < 1 && fractionJ < 1 && fractionK < 1 &&
       undefinedValue == values [ 7 ]) return undefinedValue;

   if (fractionI < 1 && fractionJ < 1 && fractionK > 0 &&
         undefinedValue == values [ 6 ]) return undefinedValue;

   if (fractionI < 1 && fractionJ > 0 && fractionK < 1 &&
         undefinedValue == values [ 5 ]) return undefinedValue;

   if (fractionI < 1 && fractionJ > 0 && fractionK > 0 &&
         undefinedValue == values [ 4 ]) return undefinedValue;

   if (fractionI > 0 && fractionJ < 1 && fractionK < 1 &&
         undefinedValue == values [ 3 ]) return undefinedValue;

   if (fractionI > 0 && fractionJ < 1 && fractionK > 0 &&
         undefinedValue == values [ 2 ]) return undefinedValue;

   if (fractionI > 0 && fractionJ > 0 && fractionK < 1 &&
         undefinedValue == values [ 1 ]) return undefinedValue;

   if (fractionI > 0 && fractionJ > 0 && fractionK > 0 &&
         undefinedValue == values [ 0 ]) return undefinedValue;

   double result = 0;

   for ( int i = 0; i < 8; ++i ) {
      result += values [ i ] * weight [ i ];
   }

   return result;
}
