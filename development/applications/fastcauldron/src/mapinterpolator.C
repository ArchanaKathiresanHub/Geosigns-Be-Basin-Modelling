//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

#include "mapinterpolator.h"

#include "ConstantsFastcauldron.h"

#include <iomanip>
#include "utils.h"

#include "GeoPhysicalConstants.h"
using namespace GeoPhysics;

// utilities library
#include "ConstantsNumerical.h"
using Utilities::Numerical::CauldronNoDataValue;
using Utilities::Numerical::IbsNoDataValue;

MapInterpolator::MapInterpolator () {
}

//------------------------------------------------------------//

MapInterpolator::~MapInterpolator () {
}

//------------------------------------------------------------//

#ifdef INTERPOLATION_INDEXING_FIXED
double MapInterpolator::getReferenceValue ( const int     highResIndex,
                                            const int     highResStartIncrement,
                                            const int     subsamplingRatio,
                                            const int     maxLowResIndex,
                                            const double* referenceValues ) const {

  int position;
  int predictedLowResIndex;
 
  /// Find I position in array of low res property
  predictedLowResIndex = ( highResIndex - ( highResIndex - highResStartIncrement ) % subsamplingRatio ) / subsamplingRatio;

  /// Get position in reference array
  if ( predictedLowResIndex > maxLowResIndex ) {
    position = subsamplingRatio;
  } else {
    position = ( highResIndex - highResStartIncrement ) % subsamplingRatio;
  }

  return referenceValues [ position ];
}
#endif

//------------------------------------------------------------//

void MapInterpolator::interpolateFromLowResolutionMap ( DM    lowResDA,
                                                        Vec   lowResVec,
                                                        const CauldronGridDescription& lowResDescription,
                                                        DM    highResDA,
                                                        Vec   highResVec,
                                                        const CauldronGridDescription& highResDescription ) const {

  const int xSubsamplingRatio = int ( lowResDescription.deltaI / highResDescription.deltaI );
  const int ySubsamplingRatio = int ( lowResDescription.deltaJ / highResDescription.deltaJ );

  const int xHighResStartIncrement = int (( lowResDescription.originI - highResDescription.originI ) / highResDescription.deltaI );
  const int yHighResStartIncrement = int (( lowResDescription.originJ - highResDescription.originJ ) / highResDescription.deltaJ );


  int highResI;
  int highResJ;
  int lowResI;
  int lowResJ;
  int K;

  PETSC_3D_Array highResProperty ( highResDA, highResVec );
  PETSC_3D_Array lowResProperty  ( lowResDA,  lowResVec, INSERT_VALUES, true );

  DMDALocalInfo highResInfo;
  DMDALocalInfo lowResInfo;

  double* xReferenceArray;
  double* yReferenceArray;

  double referenceX;
  double referenceY;

  /// The properties from the low resolution map.
  double nodalPropertyVector [ 4 ];
  double basisVector [ 4 ];

  int xHighResEnd;
  int yHighResEnd;

  int predictedLowResI;
  int predictedLowResJ;
  int maxLowResI;
  int maxLowResJ;

  /// Allocation of xReferenceArray and yReferenceArray arrays is done here.
  setReferenceArray ( xSubsamplingRatio, xReferenceArray );
  setReferenceArray ( ySubsamplingRatio, yReferenceArray );

  VecSet ( highResVec, CauldronNoDataValue );

  DMDAGetLocalInfo ( highResDA, &highResInfo );
  DMDAGetLocalInfo ( lowResDA,  &lowResInfo );

  xHighResEnd = Integer_Min ( highResInfo.xs + highResInfo.xm, xHighResStartIncrement + xSubsamplingRatio * ( lowResInfo.mx - 1 ) + 1 );
  yHighResEnd = Integer_Min ( highResInfo.ys + highResInfo.ym, yHighResStartIncrement + ySubsamplingRatio * ( lowResInfo.my - 1 ) + 1 );

  /// Arrays in C++ range from 0 .. n-1, so 1 must be subtract for the max low resI (and J),
  /// also since we want the max index to be 1 less than this, another 1 must be subtracted
  /// for the max value.
  maxLowResI = lowResInfo.mx - 2;
  maxLowResJ = lowResInfo.my - 2;

  int xHighResStart;
  int yHighResStart;

  if ( highResInfo.xs == 0 ) {
    xHighResStart = highResInfo.xs + xHighResStartIncrement;
  } else {
    xHighResStart = highResInfo.xs;
  }

  if ( highResInfo.ys == 0 ) {
    yHighResStart = highResInfo.ys + yHighResStartIncrement;
  } else {
    yHighResStart = highResInfo.ys;
  }

  for ( highResI = xHighResStart; highResI < xHighResEnd; highResI++ ) {

    /// Find I position in array of low res property
    predictedLowResI = ( highResI - ( highResI - xHighResStartIncrement ) % xSubsamplingRatio ) / xSubsamplingRatio;
    lowResI = Integer_Min ( predictedLowResI, maxLowResI );

    /// Get x-value in reference element
    if ( predictedLowResI > maxLowResI ) {
      referenceX = xReferenceArray [ xSubsamplingRatio ];
    } else {
      referenceX = xReferenceArray [ ( highResI - xHighResStartIncrement ) % xSubsamplingRatio ];
    }

    for ( highResJ = yHighResStart; highResJ < yHighResEnd; highResJ++ ) {

      /// Find J position in array of low res property
      predictedLowResJ = ( highResJ - ( highResJ - yHighResStartIncrement ) % ySubsamplingRatio ) / ySubsamplingRatio;
      lowResJ = Integer_Min ( predictedLowResJ, maxLowResJ );

      /// Get y-value in reference element
      if ( predictedLowResJ > maxLowResJ ) {
        referenceY = yReferenceArray [ ySubsamplingRatio ];
      } else {
        referenceY = yReferenceArray [ ( highResJ - yHighResStartIncrement ) % ySubsamplingRatio ];
      }

      /// Set basis functions on reference element
      setBasis ( referenceX, referenceY, basisVector );

      /// Interpolation is not performed in the z-direction.
      for ( K = highResInfo.zs; K < highResInfo.zs + highResInfo.zm; K++ ) {
        getProperty ( lowResProperty, lowResI, lowResJ, K, nodalPropertyVector );

        if ( containsNullValue ( nodalPropertyVector )) {
          highResProperty ( K, highResJ, highResI ) = CauldronNoDataValue;
        } else {
          highResProperty ( K, highResJ, highResI ) = innerProduct ( basisVector, nodalPropertyVector );
        }

      }

    }

  }

  delete [] xReferenceArray;
  delete [] yReferenceArray;
}

//------------------------------------------------------------//

void MapInterpolator::interpolateFromHighResolutionMap ( DM    highResDA,
                                                         Vec   highResVec,
                                                         const CauldronGridDescription& highResDescription,
                                                         DM    lowResDA,
                                                         Vec   lowResVec,
                                                         const CauldronGridDescription& lowResDescription ) const {
  /// TO DO
}

//------------------------------------------------------------//

void MapInterpolator::operator ()( DM    fromDA,
                                   Vec   fromVec,
                                   const CauldronGridDescription& fromDescription,
                                   DM    toDA,
                                   Vec   toVec,
                                   const CauldronGridDescription& toDescription ) const {

//  void MapInterpolator::interpolateMap ( DA    fromDA,
//                                         Vec   fromVec,
//                                         const CauldronGridDescription& fromDescription,
//                                         DA    toDA,
//                                         Vec   toVec,
//                                         const CauldronGridDescription& toDescription ) {

  if (( fromDescription.deltaI < toDescription.deltaI && fromDescription.deltaJ > toDescription.deltaJ ) ||
      ( fromDescription.deltaI > toDescription.deltaI && fromDescription.deltaJ < toDescription.deltaJ )) {

//    } else if ( fromDescription.deltaI == toDescription.deltaI &&
//                fromDescription.deltaJ == toDescription.deltaJ &&
//                fromDescription.originI == toDescription.originI &&
//                fromDescription.originJ == toDescription.originJ ) {


  } else if ( fromDescription.deltaI >= toDescription.deltaI || fromDescription.deltaJ >= toDescription.deltaJ ) {

    interpolateFromLowResolutionMap ( fromDA, fromVec, fromDescription, toDA, toVec, toDescription );

  } else {

    interpolateFromHighResolutionMap ( fromDA, fromVec, fromDescription, toDA, toVec, toDescription );

  }

}

//------------------------------------------------------------//

void MapInterpolator::setReferenceArray ( const int      resolutionDelta,
                                                double*& referenceValues ) const {

  int    I;
  double H;

  referenceValues = new double [ resolutionDelta + 1 ];

  H = 2.0 / double ( resolutionDelta );

  referenceValues [ 0 ] = -1.0;
  referenceValues [ resolutionDelta ] = 1.0;

  for ( I = 1; I < resolutionDelta; I++ ) {
    referenceValues [ I ] = -1.0 + double ( I ) * H;
  }
  
}

//------------------------------------------------------------//

void MapInterpolator::getProperty ( const PETSC_3D_Array& propertyVec,
                                    const int             startI,
                                    const int             startJ,
                                    const int             K,
                                          double*         property ) const {

  property [ 0 ] = propertyVec ( K, startJ,     startI );
  property [ 1 ] = propertyVec ( K, startJ,     startI + 1 );
  property [ 2 ] = propertyVec ( K, startJ + 1, startI + 1 );
  property [ 3 ] = propertyVec ( K, startJ + 1, startI );

}

//------------------------------------------------------------//

void MapInterpolator::setBasis ( const double  x,
                                 const double  y,
                                       double* basis ) const {

  basis [ 0 ]  = 0.25 * ( 1.0 - x ) * ( 1.0 - y );
  basis [ 1 ]  = 0.25 * ( 1.0 + x ) * ( 1.0 - y );
  basis [ 2 ]  = 0.25 * ( 1.0 + x ) * ( 1.0 + y );
  basis [ 3 ]  = 0.25 * ( 1.0 - x ) * ( 1.0 + y );
}

//------------------------------------------------------------//

bool MapInterpolator::containsNullValue ( const double* property ) const {

  int I;

  for ( I = 0; I < 4; I++ ) {

    if ( property [ I ] == IbsNoDataValue || property [ I ] == CauldronNoDataValue ) {
      return true;
    }

  } 

  return false;
}

//------------------------------------------------------------//

double MapInterpolator::innerProduct ( const double* bases, 
                                       const double* property ) const {

  double result = 0.0;
  int    I;

  for ( I = 0; I < 4; I++ ){
    result = result + bases [ I ] * property [ I ];
  }

  return result;
}

//------------------------------------------------------------//
