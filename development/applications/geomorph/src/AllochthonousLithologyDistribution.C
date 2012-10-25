#include <fstream>
#include <iomanip>
#include <cstdlib>

#include <algorithm>
#include <vector>

#include "database.h"
#include "cauldronschema.h"
#include "cauldronschemafuncs.h"

#include "AllochthonousLithologyDistribution.h"

#include "UniformRandomGenerator.h"
#include "PointSelectionArbiter.h"

#include "Interface/ProjectHandle.h"
#include "Interface/GridMap.h"
#include "Interface/Grid.h"
#include "Interface/Interface.h"


using namespace Numerics;
using namespace DataAccess;
using Interface::GridMap;
using Interface::Grid;

//------------------------------------------------------------//

AllochMod::AllochthonousLithologyDistribution::AllochthonousLithologyDistribution (Interface::ProjectHandle * projectHandle, database::Record * record) : 
  Interface::AllochthonousLithologyDistribution ( projectHandle, record ){

}

//------------------------------------------------------------//

AllochMod::AllochthonousLithologyDistribution::~AllochthonousLithologyDistribution () {
}

//------------------------------------------------------------//

void AllochMod::AllochthonousLithologyDistribution::pickOutAllBoundaryPoints ( PointArray& boundaryPoints ) const {

  int i;
  int j;

  Numerics::Point p;

  // The age is the same for all points in the map.
  p ( 2 ) = age;

  for ( i = boundaryMap.first ( 1 ); i <= boundaryMap.last ( 1 ); i++ ) {

    for ( j = boundaryMap.first ( 2 ); j <= boundaryMap.last ( 2 ); j++ ) {

      if ( boundaryMap ( i, j )) {
        p ( 0 ) = dualXCoordinates [ i ];
        p ( 1 ) = dualYCoordinates [ j ];
        boundaryPoints.push_back ( p );
      }

    }

  }


}

//------------------------------------------------------------//

void AllochMod::AllochthonousLithologyDistribution::pickOutAllInteriorPoints ( PointArray& interiorPoints ) const {

  int i;
  int j;

  Numerics::Point p;

  // The age is the same for all points in the map.
  p ( 2 ) = age;

  for ( i = erodedDistributionMap.first ( 1 ); i <= erodedDistributionMap.last ( 1 ); i++ ) {

    for ( j = erodedDistributionMap.first ( 2 ); j <= erodedDistributionMap.last ( 2 ); j++ ) {

      if ( erodedDistributionMap ( i, j )) {
        p ( 0 ) = primalXCoordinates [ i ];
        p ( 1 ) = primalYCoordinates [ j ];
        interiorPoints.push_back ( p );
      }

    }

  }

}

//------------------------------------------------------------//

void AllochMod::AllochthonousLithologyDistribution::pickOutAllExteriorPoints ( PointArray& exteriorPoints ) const {

  int i;
  int j;

  Numerics::Point p;

  // The age is the same for all points in the map.
  p ( 2 ) = age;

  for ( i = dilatedDistributionMap.first ( 1 ); i <= dilatedDistributionMap.last ( 1 ); i++ ) {

    for ( j = dilatedDistributionMap.first ( 2 ); j <= dilatedDistributionMap.last ( 2 ); j++ ) {

      if ( ! dilatedDistributionMap ( i, j )) {
        p ( 0 ) = primalXCoordinates [ i ];
        p ( 1 ) = primalYCoordinates [ j ];
        exteriorPoints.push_back ( p );
      }

    }

  }

}

//------------------------------------------------------------//

void AllochMod::AllochthonousLithologyDistribution::pickOutBoundaryPoints ( PointArray& boundaryPoints ) const {

  int i;
  int j;

  Numerics::Point p;

  // The age is the same for all points in the map.
  p ( 2 ) = age;

  for ( i = boundaryMap.first ( 1 ); i <= boundaryMap.last ( 1 ); i++ ) {

    for ( j = boundaryMap.first ( 2 ); j <= boundaryMap.last ( 2 ); j++ ) {

      if ( boundaryMap ( i, j ) && dualMapPointHasBeenChosen ( i, j )) {
        p ( 0 ) = dualXCoordinates [ i ];
        p ( 1 ) = dualYCoordinates [ j ];
        boundaryPoints.push_back ( p );
      }

    }

  }

}

//------------------------------------------------------------//

void AllochMod::AllochthonousLithologyDistribution::pickOutInteriorPoints ( PointArray& interiorPoints ) const {

  int i;
  int j;

  Numerics::Point p;

  // The age is the same for all points in the map.
  p ( 2 ) = age;

  for ( i = erodedDistributionMap.first ( 1 ); i <= erodedDistributionMap.last ( 1 ); i++ ) {

    for ( j = erodedDistributionMap.first ( 2 ); j <= erodedDistributionMap.last ( 2 ); j++ ) {

      if ( erodedDistributionMap ( i, j ) && primalMapPointHasBeenChosen ( i, j )) {
        p ( 0 ) = primalXCoordinates [ i ];
        p ( 1 ) = primalYCoordinates [ j ];
        interiorPoints.push_back ( p );
      }

    }

  }

}

//------------------------------------------------------------//

void AllochMod::AllochthonousLithologyDistribution::pickOutExteriorPoints ( PointArray& exteriorPoints ) const {

  int i;
  int j;

  Numerics::Point p;

  // The age is the same for all points in the map.
  p ( 2 ) = age;

  for ( i = dilatedDistributionMap.first ( 1 ); i <= dilatedDistributionMap.last ( 1 ); i++ ) {

    for ( j = dilatedDistributionMap.first ( 2 ); j <= dilatedDistributionMap.last ( 2 ); j++ ) {

      if ( ! dilatedDistributionMap ( i, j ) && primalMapPointHasBeenChosen ( i, j )) {
        p ( 0 ) = primalXCoordinates [ i ];
        p ( 1 ) = primalYCoordinates [ j ];
        exteriorPoints.push_back ( p );
      }

    }

  }

}

//------------------------------------------------------------//

void AllochMod::AllochthonousLithologyDistribution::inputMap ( const std::string&            fileName,
                                                               const Numerics::FloatingPoint spaceScaling,
                                                               const Numerics::FloatingPoint timeScaling ) {

  static const char MapChar [ 2 ] = { '.', 'o' };
 

  const bool SaltExists = true;

  std::ifstream mapFile;
  mapFile.open ( fileName.c_str ());

  int i;
  int j;
  int numberX;
  int numberY;
  char c;

  FloatingPoint originX;
  FloatingPoint deltaX;
  FloatingPoint originY;
  FloatingPoint deltaY;

  AllochMod::BinaryMap saltMap;

  mapFile >> age;
  mapFile >> originX >> originY;
  mapFile >> deltaX  >> deltaY;
  mapFile >> numberX >> numberY;

  age = age * timeScaling;
  deltaX = deltaX * spaceScaling;
  deltaY = deltaY * spaceScaling;

  setPrimalCoordinates ( numberX, originX, deltaX, primalXCoordinates );
  setPrimalCoordinates ( numberY, originY, deltaY, primalYCoordinates );

  setDualCoordinates ( primalXCoordinates, dualXCoordinates );
  setDualCoordinates ( primalYCoordinates, dualYCoordinates );

  distributionMap.setSize ( 0, numberX + 1, 0, numberY + 1 );
  erodedDistributionMap.setSize ( 0, numberX + 1, 0, numberY + 1 );
  dilatedDistributionMap.setSize ( 0, numberX + 1, 0, numberY + 1 );
  boundaryMap.setSize ( 0, numberX, 0, numberY );

  primalMapPointHasBeenChosen.setSize ( 0, numberX + 1, 0, numberY + 1 );
  dualMapPointHasBeenChosen.setSize (  0, numberX, 0, numberY );

  distributionMap.fill ( false );

  insideBodyCount = 0;
  outsideBodyCount = 0;

  for ( i = 1; i <= numberX; i++ ) {

    for ( j = 1; j <= numberY; j++ ) {
      mapFile >> c;
      distributionMap ( i, j ) = ( c == MapChar [ SaltExists ]);

      insideBodyCount += ( distributionMap ( i, j ) ? 1 : 0 );
      outsideBodyCount += ( distributionMap ( i, j ) ? 0 : 1 );

    }

  }

  mapFile.close ();

  initialise ();

}

//------------------------------------------------------------//

void AllochMod::AllochthonousLithologyDistribution::loadGridMap () {


  unsigned int lowResI;
  unsigned int lowResJ;
  unsigned int highResI;
  unsigned int highResJ;

  int numberI;
  int numberJ;

  int loopI;
  int loopJ;

  database::Table* projectIoTbl = getProjectHandle ()->getTable ("ProjectIoTbl");

  assert (projectIoTbl);

  assert (projectIoTbl->size () != 0);
  database::Record *projectIoRecord = projectIoTbl->getRecord (0);

  assert (projectIoRecord);


  FloatingPoint originX;
  FloatingPoint deltaX;
  FloatingPoint originY;
  FloatingPoint deltaY;

  int subScalingFactorI  = 1;
  int subScalingFactorJ  = 1;

  int startI;
  int startJ;

  distributionGridMap = loadMap ( DataAccess::Interface::AllochthonousLithologyDistributionMap );

  const Interface::Grid* distributionGrid = distributionGridMap->getGrid ();

  age = getAge ();

  // Number of nodes in grid.
  numberI = distributionGrid->numI ();
  numberJ = distributionGrid->numJ ();

  // Start position in grid.
  startI = distributionGrid->firstI ();
  startJ = distributionGrid->firstJ ();

  // Origin of the grid.
  originX = distributionGrid->minI ();
  originY = distributionGrid->minJ ();

  // delta in grid.
  deltaX = distributionGrid->deltaI ();
  deltaY = distributionGrid->deltaJ ();

  setPrimalCoordinates ( numberI, originX, deltaX, primalXCoordinates );
  setPrimalCoordinates ( numberJ, originY, deltaY, primalYCoordinates );

  setDualCoordinates ( primalXCoordinates, dualXCoordinates );
  setDualCoordinates ( primalYCoordinates, dualYCoordinates );

  // Set map sizes.
  //
  // Distribution maps are same as input maps except an extra layer of nodes around border.
  distributionMap.setSize ( 0, numberI + 1, 0, numberJ + 1 );
  erodedDistributionMap.setSize ( 0, numberI + 1, 0, numberJ + 1 );
  dilatedDistributionMap.setSize ( 0, numberI + 1, 0, numberJ + 1 );
  primalMapPointHasBeenChosen.setSize ( 0, numberI + 1, 0, numberJ + 1 );

  // The bounday maps are the dual of the distribution maps and so have as many
  // nodes as the distribution maps have elements.
  boundaryMap.setSize ( 0, numberI, 0, numberJ );
  dualMapPointHasBeenChosen.setSize ( 0, numberI, 0, numberJ );

  distributionMap.fill ( false );

  insideBodyCount = 0;
  outsideBodyCount = 0;

  // Copy data from the input map into the distribution map.
  for ( loopI = 0, highResI = startI, lowResI = 1; loopI < numberI; ++loopI, highResI += subScalingFactorI, ++lowResI ) {

    for ( loopJ = 0, highResJ = startJ, lowResJ = 1; loopJ < numberJ; ++loopJ, highResJ += subScalingFactorJ, ++lowResJ ) {
      // Here, we are only concerned with those points that are defined to be within the salt body.
      // if the point is a missing data points (i.e. = null value) then it is assumed to be outside 
      // of the salt body.

       if ( distributionGridMap->valueIsDefined ( highResI, highResJ ) and distributionGridMap->getValue ( highResI, highResJ ) >= 0.5 ) {
          distributionMap ( lowResI, lowResJ ) = true;
          ++insideBodyCount;
       } else {
          distributionMap ( lowResI, lowResJ ) = false;
          ++outsideBodyCount;
       }

    }

  }

  distributionGridMap->release ();
}

//------------------------------------------------------------//

void AllochMod::AllochthonousLithologyDistribution::initialise () {

  primalMapPointHasBeenChosen.fill ( false );
  dualMapPointHasBeenChosen.fill ( false );
  erodedDistributionMap.fill ( false );
  dilatedDistributionMap.fill ( false );
  boundaryMap.fill ( false );

  AllochMod::erodeMap   ( distributionMap, erodedDistributionMap );
  AllochMod::dilateMap  ( distributionMap, dilatedDistributionMap );
  AllochMod::detectEdge ( distributionMap, boundaryMap ); 


}

//------------------------------------------------------------//

void AllochMod::AllochthonousLithologyDistribution::setPrimalCoordinates ( const int                 number,
                                                                           const FloatingPoint       origin,
                                                                           const FloatingPoint       delta,
                                                                                 FloatingPointArray& coordinates ) {

  int i;
  FloatingPoint boundaryDelta = BoundaryExtensionFactor * delta;

  coordinates.resize ( number + 2 );

  coordinates [ 0 ] = origin - boundaryDelta;

  for ( i = 1; i <= number; ++i ) {
    coordinates [ i ] = origin + FloatingPoint ( i - 1 ) * delta;
  }

  coordinates [ number + 1 ] = coordinates [ number ] + boundaryDelta;

}


//------------------------------------------------------------//

void AllochMod::AllochthonousLithologyDistribution::setDualCoordinates ( const FloatingPointArray& primalCoordinates,
                                                                               FloatingPointArray& dualCoordinates ) {

  size_t i;

  dualCoordinates.resize ( primalCoordinates.size () - 1 );

  for ( i = 0; i < dualCoordinates.size (); ++i ) {
    dualCoordinates [ i ] = 0.5 * ( primalCoordinates [ i ] + primalCoordinates [ i + 1 ]);
  }

}

//------------------------------------------------------------//

// Numerics::FloatingPoint AllochMod::AllochthonousLithologyDistribution::getAge () const {
//   return age;
// }

//------------------------------------------------------------//

const AllochMod::BinaryMap& AllochMod::AllochthonousLithologyDistribution::getDistributionMap () const {
  return distributionMap;
}

//------------------------------------------------------------//

const AllochMod::BinaryMap& AllochMod::AllochthonousLithologyDistribution::getErodedDistributionMap () const {
  return erodedDistributionMap;
}

//------------------------------------------------------------//

const AllochMod::BinaryMap& AllochMod::AllochthonousLithologyDistribution::getDilatedDistributionMap () const {
  return dilatedDistributionMap;
}

//------------------------------------------------------------//

const AllochMod::BinaryMap& AllochMod::AllochthonousLithologyDistribution::getBoundaryMap () const {
  return boundaryMap;
}

//------------------------------------------------------------//

const Numerics::FloatingPointArray& AllochMod::AllochthonousLithologyDistribution::getXCoordinates ( const MapType m ) const {

  if ( m == PRIMAL_MAP ) {
    return primalXCoordinates;
  } else {
    return dualXCoordinates;
  }

}

//------------------------------------------------------------//

const Numerics::FloatingPointArray& AllochMod::AllochthonousLithologyDistribution::getYCoordinates ( const MapType m ) const {

  if ( m == PRIMAL_MAP ) {
    return primalYCoordinates;
  } else {
    return dualYCoordinates;
  }

}

//------------------------------------------------------------//

Numerics::Point AllochMod::AllochthonousLithologyDistribution::getPoint ( const MapType m,
                                                                          const int     i,
                                                                          const int     j ) const {

  Numerics::Point p;

  p ( 2 ) = age;

  if ( m == PRIMAL_MAP ) {
    p ( 0 ) = primalXCoordinates [ i ];
    p ( 1 ) = primalYCoordinates [ j ];
  } else {
    p ( 0 ) = dualXCoordinates [ i ];
    p ( 1 ) = dualYCoordinates [ j ];
  }

  return p;
}

//------------------------------------------------------------//

bool AllochMod::AllochthonousLithologyDistribution::pointIsExterior ( const int i,
                                                                      const int j ) const {
  return ! dilatedDistributionMap ( i, j );
}

//------------------------------------------------------------//

bool AllochMod::AllochthonousLithologyDistribution::pointIsInterior ( const int i,
                                                                      const int j ) const {
  return erodedDistributionMap ( i, j );
}

//------------------------------------------------------------//

bool AllochMod::AllochthonousLithologyDistribution::pointIsOnBoundary ( const int i,
                                                                        const int j ) const {
  return boundaryMap ( i, j );
}

//------------------------------------------------------------//

bool AllochMod::AllochthonousLithologyDistribution::pointHasBeenSelected ( const MapType m,
                                                                           const int     i,
                                                                           const int     j ) const {

  if ( m == PRIMAL_MAP ) {
    return primalMapPointHasBeenChosen ( i, j );
  } else {
    return dualMapPointHasBeenChosen ( i, j );
  }

}

//------------------------------------------------------------//

int AllochMod::AllochthonousLithologyDistribution::getNumberOfPointsInsideDomain () const {
  return insideBodyCount;
}

//------------------------------------------------------------//

int AllochMod::AllochthonousLithologyDistribution::getNumberOfPointsOutsideDomain () const {
  return outsideBodyCount;
}

//------------------------------------------------------------//

int AllochMod::AllochthonousLithologyDistribution::mapXDimension () const {
  return distributionMap.length ( 1 ) - 2;
}

//------------------------------------------------------------//

int AllochMod::AllochthonousLithologyDistribution::mapYDimension () const {
  return distributionMap.length ( 2 ) - 2;
}

//------------------------------------------------------------//

void AllochMod::AllochthonousLithologyDistribution::printInteriorMap ( std::ostream& o ) const {

  int i;
  int j;

  o << std::endl;

  for ( i = erodedDistributionMap.first ( 1 ) + 1; i <= erodedDistributionMap.last ( 1 ) - 1; i++ ) {

    for ( j = erodedDistributionMap.first ( 2 ) + 1; j <= erodedDistributionMap.last ( 2 ) - 1; j++ ) {

      if ( erodedDistributionMap ( i, j )) {
        o << 'o';
      } else {
        o << '.';
      }

    }

    o << std::endl;
  }

  o << std::endl;
  o << std::endl;
}

//------------------------------------------------------------//

void AllochMod::AllochthonousLithologyDistribution::printExteriorMap ( std::ostream& o ) const {

  int i;
  int j;

  o << std::endl;

  for ( i = dilatedDistributionMap.first ( 1 ) + 1; i <= dilatedDistributionMap.last ( 1 ) - 1; i++ ) {

    for ( j = dilatedDistributionMap.first ( 2 ) + 1; j <= dilatedDistributionMap.last ( 2 ) - 1; j++ ) {

      // Here, only if the map has a false value then is it outside the salt body.
      if ( dilatedDistributionMap ( i, j )) {
        o << '.';
      } else {
        o << 'o';
      }

    }

    o << std::endl;
  }

  o << std::endl;
  o << std::endl;
}

//------------------------------------------------------------//

void AllochMod::AllochthonousLithologyDistribution::printBoundaryMap ( std::ostream& o ) const {

  int i;
  int j;

  o << std::endl;

  for ( i = boundaryMap.first ( 1 ); i <= boundaryMap.last ( 1 ); i++ ) {

    for ( j = boundaryMap.first ( 2 ); j <= boundaryMap.last ( 2 ); j++ ) {

      if ( boundaryMap ( i, j )) {
        o << 'o';
      } else {
        o << '.';
      }

    }

    o << std::endl;
  }

  o << std::endl;
  o << std::endl;
}

//------------------------------------------------------------//
