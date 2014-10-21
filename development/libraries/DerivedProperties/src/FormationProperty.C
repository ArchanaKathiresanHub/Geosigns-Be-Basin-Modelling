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
      return get ( i, j, bottomNode );
   } else {
      double bottomValue = get ( i, j, bottomNode );
      double topValue = get ( i, j, bottomNode + 1 );

      if ( bottomValue == getUndefinedValue () or topValue == getUndefinedValue ()) {
         return getUndefinedValue ();
      } else {
         return bottomValue + fraction * ( topValue - bottomValue );
      }

   }

}
