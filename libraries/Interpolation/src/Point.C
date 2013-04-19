//------------------------------------------------------------//

#include "Point.h"
#include <sstream>
#include <iomanip>
#include <cmath>

//------------------------------------------------------------//
 
Numerics::Point::Point () {
   zero ();
}

//------------------------------------------------------------//

Numerics::Point::Point ( const Point& p ) {
   values [ 0 ] = p ( 0 );
   values [ 1 ] = p ( 1 );
   values [ 2 ] = p ( 2 );
}

//------------------------------------------------------------//

Numerics::Point::Point ( const double initialX,
                         const double initialY,
                         const double initialZ ) {

   values [ 0 ] = initialX;
   values [ 1 ] = initialY;
   values [ 2 ] = initialZ;
}

//------------------------------------------------------------//

void Numerics::Point::zero () {
   values [ 0 ] = 0.0;
   values [ 1 ] = 0.0;
   values [ 2 ] = 0.0;
}

//------------------------------------------------------------//

std::string Numerics::Point::image () const {

   std::stringstream buffer;

   buffer.precision ( 5 );
   buffer.flags ( std::ios::scientific );

   buffer << "( " 
          << std::setw ( 12 ) << x () << ", " 
          << std::setw ( 12 ) << y () << ", " 
          << std::setw ( 12 ) << z () << " )";
   return buffer.str ();
}

//------------------------------------------------------------//

Numerics::FloatingPoint Numerics::separationDistanceSquared ( const Point& p1,
                                                              const Point& p2 ) {

  FloatingPoint result = 0.0;
  FloatingPoint difference;
  int i;

  for ( i = 0; i < Point::DIMENSION; ++i ) {
    difference = p1 ( i ) - p2 ( i );
    result += difference * difference;
  }

  return result;
}

//------------------------------------------------------------//

Numerics::FloatingPoint Numerics::separationDistance ( const Point& p1,
                                                       const Point& p2 ) {

  return std::sqrt ( separationDistanceSquared( p1, p2 ));
}

//------------------------------------------------------------//

std::ostream& Numerics::operator<< ( std::ostream& o, const Numerics::PointArray& array ) {

  unsigned int i;

  for ( i = 0; i < array.size (); i++ ){
     o << array [ i ].image () << std::endl;
  }

  return o;
}

//------------------------------------------------------------//

