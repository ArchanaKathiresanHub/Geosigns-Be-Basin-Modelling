#include "DeviatedWell.h"

#include "Interface.h"

#include "NumericFunctions.h"

DataAccess::Mining::DeviatedWell::DeviatedWell ( const std::string& name ) : CauldronWell ( name ) {
   m_length = 0.0;
}

DataAccess::Mining::DeviatedWell::~DeviatedWell () {
}

/// Add a point to the deviated well.
///
/// (x,y,z) is the point in space, s is the depth-along-hole.
void DataAccess::Mining::DeviatedWell::addLocation ( const double x,
                                                     const double y,
                                                     const double z,
                                                     const double s ) {

   // Add check for frozen wells.

   m_x.addPoint ( s, x );
   m_y.addPoint ( s, y );
   m_z.addPoint ( s, z );

   m_length = NumericFunctions::Maximum<double>( m_length, s );
}

void DataAccess::Mining::DeviatedWell::addLocation ( const Numerics::Point& p,
                                                     const double           s ) {
   addLocation ( p ( 0 ), p ( 1 ), p ( 2 ), s );
}


/// No more points can be added after this is called.
void DataAccess::Mining::DeviatedWell::freeze ( const Numerics::PieceWiseInterpolator1D::InterpolationKind kind,
                                                const bool                                                 allowExtrapolation ) {

   m_x.freeze ( kind, allowExtrapolation );
   m_y.freeze ( kind, allowExtrapolation );
   m_z.freeze ( kind, allowExtrapolation );
}

Numerics::Point DataAccess::Mining::DeviatedWell::getTop () const {

   Numerics::Point top;

   top ( 0 ) = m_x ( 0.0 );
   top ( 1 ) = m_y ( 0.0 );
   top ( 2 ) = m_z ( 0.0 );

   return top;
}

Numerics::Point DataAccess::Mining::DeviatedWell::getLocation ( const double depthAlongHole ) const {

   Numerics::Point result;
   
   result ( 0 ) = m_x ( depthAlongHole );
   result ( 1 ) = m_y ( depthAlongHole );
   result ( 2 ) = m_z ( depthAlongHole );

   if ( result ( 0 ) == Numerics::PieceWiseInterpolator1D::NullValue or
        result ( 1 ) == Numerics::PieceWiseInterpolator1D::NullValue or
        result ( 2 ) == Numerics::PieceWiseInterpolator1D::NullValue ) {

      result ( 0 ) = Interface::DefaultUndefinedMapValue;
      result ( 1 ) = Interface::DefaultUndefinedMapValue;
      result ( 2 ) = Interface::DefaultUndefinedMapValue;
   }

   return result;
}

double DataAccess::Mining::DeviatedWell::getLength () const {
   return m_length;
}

Numerics::Point DataAccess::Mining::DeviatedWell::getBottom () const {

   Numerics::Point bottom;

   bottom ( 0 ) = m_x ( m_length );
   bottom ( 1 ) = m_y ( m_length );
   bottom ( 2 ) = m_z ( m_length );

   return bottom;
}
