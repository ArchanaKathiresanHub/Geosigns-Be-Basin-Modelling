//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#include "snapshotinterpolator.h"
#include "array.h"

#include "petsc.h"

#include "h5_types.h"
#include "h5_file_types.h"

#include "ConstantsFastcauldron.h"
#include "utils.h"


//------------------------------------------------------------//

PiecewiseLinearInterpolantCalculator::PiecewiseLinearInterpolantCalculator ( const int panelCount ) :  m_maximumNumberOfPanels ( panelCount ) {
  snapshotTimes.reserve ( m_maximumNumberOfPanels );
}

//------------------------------------------------------------//

PiecewiseLinearInterpolantCalculator::~PiecewiseLinearInterpolantCalculator () { 
  snapshotTimes.clear ();
}

//------------------------------------------------------------//

int PiecewiseLinearInterpolantCalculator::maximumNumberOfPanels () const {
  return m_maximumNumberOfPanels;
}

//------------------------------------------------------------//

int PiecewiseLinearInterpolantCalculator::numberOfCoefficientsRequired () const {

  return ( m_maximumNumberOfPanels - 1 ) * 2;
}

//------------------------------------------------------------//

void PiecewiseLinearInterpolantCalculator::clearTimes () {
  snapshotTimes.clear ();
}

//------------------------------------------------------------//

void PiecewiseLinearInterpolantCalculator::addTime ( const double newTime ) {
  snapshotTimes.push_back ( newTime );
}

//------------------------------------------------------------//

int PiecewiseLinearInterpolantCalculator::numberOfSnapshotTimes () const {
  return int ( snapshotTimes.size ());
}

//------------------------------------------------------------//

void PiecewiseLinearInterpolantCalculator::computeInterpolant ( const DoubleArray& propertyValues,
                                                                      double*      memoryChunk ) {

  double ratio;
  size_t i;

  ///
  /// DO NOT DEALLOCATE THESE.
  ///
  double*     As;
  double*     Bs;

  As = &memoryChunk [ 0 ];
  Bs = &memoryChunk [ m_maximumNumberOfPanels - 1 ];

  for ( i = 0; i < propertyValues.size () - 1; i++ ) {
    ratio = 1.0 / ( snapshotTimes [ i + 1 ] - snapshotTimes [ i ]);
    As [ i ] = ( snapshotTimes [ i + 1 ] * propertyValues [ i ]  - snapshotTimes [ i ] * propertyValues [ i + 1 ]) * ratio;
    Bs [ i ] = ( propertyValues [ i + 1 ] - propertyValues [ i ]) * ratio;
  }

}

//------------------------------------------------------------//

void PiecewiseLinearInterpolantCalculator::setCurrentTime ( const double newTime ) {

  unsigned int I;

  currentPanel = -1;
  currentTime = newTime;

  for ( I = 0; I < snapshotTimes.size () - 1; I++ ) {

    if ( snapshotTimes [ I + 1 ] <= currentTime ) {
      currentPanel = I;
      break;
    }

  }

}

//------------------------------------------------------------//

double PiecewiseLinearInterpolantCalculator::operator ()( const double* memoryChunk ) const {

  double result;

  ///
  /// DO NOT DEALLOCATE THESE.
  ///
  const double* As = &memoryChunk [ 0 ];
  const double* Bs = &memoryChunk [ m_maximumNumberOfPanels - 1 ];

  result = As [ currentPanel ] + Bs [ currentPanel ] * currentTime;

  return result;
}

//------------------------------------------------------------//

ConstantInterpolantCalculator::ConstantInterpolantCalculator ( const int panelCount ) {
  snapshotTime = -1.0;
  currentTime = -1.0;
}

//------------------------------------------------------------//

ConstantInterpolantCalculator::~ConstantInterpolantCalculator () { 
}

//------------------------------------------------------------//

int ConstantInterpolantCalculator::maximumNumberOfPanels () const {
  return 1;
}

//------------------------------------------------------------//

int ConstantInterpolantCalculator::numberOfCoefficientsRequired () const {

  return 1;
}

//------------------------------------------------------------//

void ConstantInterpolantCalculator::clearTimes () {
  snapshotTime = -1.0;
}

//------------------------------------------------------------//

void ConstantInterpolantCalculator::addTime ( const double newTime ) {
  snapshotTime = newTime;
}

//------------------------------------------------------------//

int ConstantInterpolantCalculator::numberOfSnapshotTimes () const {
  return 1;
}

//------------------------------------------------------------//

void ConstantInterpolantCalculator::computeInterpolant ( const DoubleArray& propertyValue,
                                                               double*      memoryChunk ) {

  ///
  /// DO NOT DEALLOCATE THIS.
  ///
  double* value = &memoryChunk [ 0 ];

  value [ 0 ] = propertyValue [ 0 ];
}

//------------------------------------------------------------//

void ConstantInterpolantCalculator::setCurrentTime ( const double newTime ) {

  currentTime = newTime;
}

//------------------------------------------------------------//

double ConstantInterpolantCalculator::operator ()( const double* memoryChunk ) const {

  double result;

  ///
  /// DO NOT DEALLOCATE THIS.
  ///
  const double* value = &memoryChunk [ 0 ];

  result = value [ 0 ];

  return result;
}

//------------------------------------------------------------//

void SnapshotInterpolator::setPropertyNames ( const std::string& newLayerName,
                                              const std::string& newPropertyName ) {

  layerName    = removeNonUsableCharacters ( newLayerName );
  propertyName = newPropertyName;

}

//------------------------------------------------------------//
