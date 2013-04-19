//------------------------------------------------------------//

#include "MapProcessingOperations.h"

//------------------------------------------------------------//

void AllochMod::erodeMap ( const BinaryMap& input,
                                 BinaryMap& eroded ) {

  int i;
  int j;

  eroded = input;

//   return;

  for ( i = input.first ( 1 ); i <= input.last ( 1 ); i++ ) {

    for ( j = input.first ( 2 ); j <= input.last ( 2 ); j++ ) {

      if ( erodeEntry ( input, i, j )) {
        eroded ( i, j ) = false;
      }

    }

  }

}

//------------------------------------------------------------//

void AllochMod::dilateMap ( const BinaryMap& input,
                                  BinaryMap& dilated ) {

  int i;
  int j;

  dilated = input;

//   return;

  for ( i = dilated.first ( 1 ); i <= dilated.last ( 1 ); i++ ) {

    for ( j = dilated.first ( 2 ); j <= dilated.last ( 2 ); j++ ) {

      if ( dilateEntry ( input, i, j )) {
        dilated ( i, j ) = true;
      }

    }

  }

}

//------------------------------------------------------------//

bool AllochMod::erodeEntry ( const BinaryMap& copy,
                             const int        i,
                             const int        j ) {

  static const int IndexDiff [ 3 ] = { -1, 0, 1 };

  int k;
  int l;
  bool erode = false;

  for ( k = 0; k < 3; k++ ) {

    if (( i + IndexDiff [ k ] < copy.first ( 1 )) || ( i + IndexDiff [ k ] > copy.last ( 1 ))) {
      continue;
    }

    for ( l = 0; l < 3; l++ ) {

      if (( j + IndexDiff [ l ] < copy.first ( 2 )) || ( j + IndexDiff [ l ] > copy.last ( 2 ))) {
        continue;
      }

      erode = erode || copy ( i + IndexDiff [ k ], j + IndexDiff [ l ]) != copy ( i, j );
    }

  }

  return erode;

}

//------------------------------------------------------------//

bool AllochMod::dilateEntry ( const BinaryMap& copy,
                              const int        i,
                              const int        j ) {

  static const int IndexDiff [ 3 ] = { -1, 0, 1 };

  int k;
  int l;
  bool dilate = false;

  for ( k = 0; k < 3; k++ ) {

    if (( i + IndexDiff [ k ] < copy.first ( 1 )) || ( i + IndexDiff [ k ] > copy.last ( 1 ))) {
      continue;
    }

    for ( l = 0; l < 3; l++ ) {

      if (( j + IndexDiff [ l ] < copy.first ( 2 )) || ( j + IndexDiff [ l ] > copy.last ( 2 ))) {
        continue;
      }

      dilate = dilate || copy ( i + IndexDiff [ k ], j + IndexDiff [ l ]) != copy ( i, j );
    }

  }

  return dilate;

}

//------------------------------------------------------------//

void AllochMod::detectEdge ( const BinaryMap& primalMap,
                                   BinaryMap& map ) {

  int i;
  int j;
//   bool allEqual;

  int sum;

  map.fill ( false );

  for ( i = map.first ( 1 ); i <= map.last ( 1 ); i++ ) {

    for ( j = map.first ( 2 ); j <= map.last ( 2 ); j++ ) {

      sum = primalMap ( i, j ) + primalMap ( i, j + 1 ) + primalMap ( i + 1, j + 1 ) + primalMap ( i + 1, j );


      // If all the nodes (in the primal map) of the element (in the dual map)
      // are then same then they will all equal the value of the first node.
//       allEqual = ( primalMap ( i, j ) == primalMap ( i, j + 1 )) && 
//                  ( primalMap ( i, j ) == primalMap ( i + 1, j + 1 )) && 
//                  ( primalMap ( i, j ) == primalMap ( i + 1, j ));

//       map ( i, j ) = ! allEqual;
      map ( i, j ) = ( sum == 2 ) || ( sum == 2 );
    }

  }
  
}

//------------------------------------------------------------//

void AllochMod::print ( const BinaryMap& map,
                              std::ostream& o,
                        const char falseChar,
                        const char trueChar ) {

  int i;
  int j;

  for ( i = map.first ( 1 ); i <= map.last ( 1 ); i++ ) {

    for ( j = map.first ( 2 ); j <= map.last ( 2 ); j++ ) {

      if ( map ( i, j )) {
        o << trueChar;
      } else {
        o << falseChar;
      }

    }

    o << std::endl;
  }

}

//------------------------------------------------------------//
