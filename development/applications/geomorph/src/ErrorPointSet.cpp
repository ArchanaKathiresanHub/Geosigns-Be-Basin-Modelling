#include <algorithm>

#include "ErrorPointSet.h"

//------------------------------------------------------------//

void AllochMod::ErrorPointSet::addError ( const FloatingPoint error,
                                          const int           i,
                                          const int           j ) {
  ErrorPosition e;

  e.error = error;
  e.i = i;
  e.j = j;

  addError ( e );

}

//------------------------------------------------------------//

void AllochMod::ErrorPointSet::addError ( const ErrorPosition& p ) {
  errorPositions.push_back ( p );
}

//------------------------------------------------------------//

bool AllochMod::ErrorPointSet::hasNearby ( const ErrorPosition& p,
                                           const int            iStride,
                                           const int            jStride ) const {

  size_t k;

  for ( k = 0; k < errorPositions.size (); k++ ) {

    if (( abs ( p.i - errorPositions [ k ].i ) <= iStride ) &&
        ( abs ( p.j - errorPositions [ k ].j ) <= jStride )) {
      return true;
    }

  }

  return false;
}

//------------------------------------------------------------//

void AllochMod::ErrorPointSet::sortErrors () {

  ErrorPositionOp op;

  std::sort ( errorPositions.begin (), errorPositions.end (), op );
}

//------------------------------------------------------------//

const AllochMod::ErrorPointSet::ErrorPosition& AllochMod::ErrorPointSet::operator ()( const int i ) const {
  return errorPositions [ i ];
}

//------------------------------------------------------------//

int AllochMod::ErrorPointSet::size () const {
  return errorPositions.size ();
}

//------------------------------------------------------------//

void AllochMod::ErrorPointSet::clear () {
  errorPositions.clear ();
}

//------------------------------------------------------------//
