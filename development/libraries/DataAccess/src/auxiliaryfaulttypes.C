//#include <cmath>
#include <math.h>

#include "Grid.h"
#include "auxiliaryfaulttypes.h"

using namespace DataAccess;
using namespace Interface;


//------------------------------------------------------------//

void DataAccess::Interface::convertDistanceUnits ( const DistanceUnit From, 
                            const DistanceUnit To,
                                  double&      Value ) {

  static double ConversionFactors [2][2] = {{ 1.0,    3.280839895 },  // From Metres
                                            { 0.3048, 1.0 }};         // From Feet

  Value = ConversionFactors [ From ][ To ] * Value;

}

//------------------------------------------------------------//


bool ElementLess::operator()( const Element& E1, const Element& E2 ) const {

   if( E1 ( Interface::X_COORD ) < E2 ( Interface::X_COORD ) ||
       ( E1 ( Interface::X_COORD ) == E2 ( Interface::X_COORD ) && E1 ( Interface::Y_COORD ) < E2 ( Interface::Y_COORD ) ) )
    return true;
   else 
  return false;
}


//------------------------------------------------------------//




ostream& DataAccess::Interface::operator<< ( ostream& o, const Point& P ) {
  o  << P ( Interface::X_COORD ) << "  " << P ( Interface::Y_COORD ) << endl;
  return o;
}

Point DataAccess::Interface::midPoint ( const Point& P1, const Point& P2 ) {

  Point MP;

  MP ( Interface::X_COORD ) = 0.5 * ( P1 ( Interface::X_COORD ) + P2 ( Interface::X_COORD ));
  MP ( Interface::Y_COORD ) = 0.5 * ( P1 ( Interface::Y_COORD ) + P2 ( Interface::Y_COORD ));

  return MP;
}

bool DataAccess::Interface::operator== ( const Point& P1, const Point& P2 ) {
  return P1 ( Interface::X_COORD ) == P2 ( Interface::X_COORD ) && P1 ( Interface::Y_COORD ) == P2 ( Interface::Y_COORD );
}

bool DataAccess::Interface::operator!= ( const Point& P1, const Point& P2 ) {
  bool isEqual = (P1 == P2);
  return ! isEqual;
}

Point DataAccess::Interface::operator-( const Point& P1, const Point& P2 ) {

  Point Result;

  Result ( Interface::X_COORD ) = P1 ( Interface::X_COORD ) - P2 ( Interface::X_COORD );
  Result ( Interface::Y_COORD ) = P1 ( Interface::Y_COORD ) - P2 ( Interface::Y_COORD );

  return Result;
}


Point DataAccess::Interface::operator+( const Point& P1, const Point& P2 ) {

  Point Result;

  Result ( Interface::X_COORD ) = P1 ( Interface::X_COORD ) + P2 ( Interface::X_COORD );
  Result ( Interface::Y_COORD ) = P1 ( Interface::Y_COORD ) + P2 ( Interface::Y_COORD );

  return Result;
}


Point DataAccess::Interface::operator*( const double X, const Point& P1 ) {

  Point Result;

  Result ( Interface::X_COORD ) = X * P1 ( Interface::X_COORD );
  Result ( Interface::Y_COORD ) = X * P1 ( Interface::Y_COORD );

  return Result;
}


Point DataAccess::Interface::operator/( const Point& P1, const double X ) {

  Point Result;

  Result ( Interface::X_COORD ) = P1 ( Interface::X_COORD ) / X;
  Result ( Interface::Y_COORD ) = P1 ( Interface::Y_COORD ) / X;

  return Result;
}


double DataAccess::Interface::separationDistance ( const Point& P1, const Point& P2 ) {

  Point DV = P1 - P2;

  return sqrt ( DV ( Interface::X_COORD ) * DV ( Interface::X_COORD ) + DV ( Interface::Y_COORD ) * DV ( Interface::Y_COORD ));

}


//------------------------------------------------------------//


ostream& DataAccess::Interface::operator<< ( ostream& o, const Element& E ) {
  o  << E ( Interface::Y_COORD ) << "  " << E ( Interface::X_COORD );
  return o;
}


bool DataAccess::Interface::operator== ( const Element& E1, const Element& E2 ) {
  return E1 ( Interface::X_COORD ) == E2 ( Interface::X_COORD ) && E1 ( Interface::Y_COORD ) == E2 ( Interface::Y_COORD );
}

bool DataAccess::Interface::operator!= ( const Element& E1, const Element& E2 ) {
  return ! ( E1 == E2 );
}


Element DataAccess::Interface::whichElement (const Grid & grid, const Point & P)
{

   Element E;

   E (Interface::X_COORD) = int ((P (Interface::X_COORD) - grid.minIGlobal ()) / grid.deltaI ());
   E (Interface::Y_COORD) = int ((P (Interface::Y_COORD) - grid.minJGlobal ()) / grid.deltaJ ());

   return E;
}

//------------------------------------------------------------//

std::string DataAccess::Interface::pressurefaultStatusImage ( const DataAccess::Interface::PressureFaultStatus faultStatus ) {

  static std::string pressurefaultStatusStrings [ 2 ] = { "PASS_WATER", "SEAL_WATER" };

  return pressurefaultStatusStrings [ faultStatus ];
}

//------------------------------------------------------------//

ostream& DataAccess::Interface::operator << ( ostream& o, const ElementSet& ES ) {

  ElementSet::const_iterator elemIter;

  o << " ElementSet size " << ES.size () << endl;

  for ( elemIter = ES.begin(); elemIter != ES.end (); ++elemIter ) {
    o << (*elemIter) << endl;
  }

  return o;
}

//------------------------------------------------------------//

ostream& DataAccess::Interface::operator << ( ostream& o, const PointSequence& PS ) {

  PointSequence::const_iterator pntIter;

  o << " ElementSet size " << PS.size () << endl;

  for ( pntIter = PS.begin(); pntIter != PS.end (); ++pntIter ) {
    o << (*pntIter) << endl;
  }

  return o;
}

//------------------------------------------------------------//
