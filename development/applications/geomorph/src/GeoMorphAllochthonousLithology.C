#include "GeoMorphAllochthonousLithology.h"

#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <sstream>

#include <algorithm>

#include "WallTime.h"

#include "UniformRandomGenerator.h"
#include "PointSelectionArbiter.h"

#include "cauldronschemafuncs.h"
#include "hdf5funcs.h"

#include "ProjectHandle.h"
#include "Formation.h"
#include "Surface.h"
#include "Snapshot.h"
#include "GridMap.h"
#include "Grid.h"
#include "AttributeValue.h"
#include "LithoType.h"
#include "Interface.h"
#include "AllochthonousLithologyInterpolation.h"


using namespace Numerics;
using namespace DataAccess;

using Interface::GridMap;
using Interface::Grid;
using Interface::Snapshot;
using Interface::Formation;
using Interface::Surface;
using Interface::AttributeValue;
using Interface::LithoType;

namespace AllochMod
{
//------------------------------------------------------------//

GeoMorphAllochthonousLithology::GeoMorphAllochthonousLithology (Interface::ProjectHandle * projectHandle, database::Record * record) :
  Interface::AllochthonousLithology ( projectHandle, record ){
  
}

//------------------------------------------------------------//

GeoMorphAllochthonousLithology::~GeoMorphAllochthonousLithology () {

  size_t i;

  for ( i = 0; i < interpolators.size (); ++i ) {
    delete interpolators [ i ];
  }

}

//------------------------------------------------------------//

void GeoMorphAllochthonousLithology::computeInterpolant ( AllochthonousLithologyDistributionSequence& inputMaps,
                                                             const int                                   debugLevel ) {

  WallTime::Time startTime;
  WallTime::Time endTime;
  WallTime::Duration executionTime;
  int    hours;
  int    minutes;
  double seconds;

  Numerics::GeometryVector translation;
  Numerics::GeometryVector scaling;
  MapIntervalInterpolator* interp;
  size_t i;

  const unsigned int oldPrecision = std::cout.precision ( 3 );
  const std::ios::fmtflags oldFlags = std::cout.flags(std::ios::fixed);

  // For each of the intervals in the input map sequence, compute an interpolant.
  for ( i = 1; i < inputMaps.size (); ++i ) {
    startTime = WallTime::clock ();

    interp = allocateIntervalInterpolator ();
    computeTransformation ( inputMaps [ i - 1 ], inputMaps [ i ], translation, scaling );
    interp->setTransformation ( translation, scaling );

    // reset the maps holding which points have been picked.
    inputMaps [ i - 1 ]->initialise ();
    inputMaps [ i ]->initialise ();

    interp->computeInterpolator ( inputMaps [ i - 1 ], inputMaps [ i ], debugLevel );

    // Add interpolator to set of interpolators for intervals.
    interpolators.push_back ( interp );

    endTime = WallTime::clock ();
    executionTime = endTime - startTime;

    executionTime.separate ( hours, minutes, seconds );

    std::cout << "    Interval [ " 
              << std::setw ( 7 ) << inputMaps [ i - 1 ]->getAge ()
              << ", "
              << std::setw ( 7 ) << inputMaps [ i ]->getAge ()
              << " ] took " 
              << std::setw ( 3 ) << hours << " hours " 
              << std::setw ( 2 ) << minutes << " minutes " 
              << std::setw ( 6 ) << std::setiosflags(std::ios::right) << seconds << " seconds " 
              << std::endl;

  }

  std::cout.precision ( oldPrecision );
  std::cout.flags( oldFlags );
}

//------------------------------------------------------------//

void GeoMorphAllochthonousLithology::printDistributionMaps ( AllochthonousLithologyDistributionSequence& inputMaps,
                                                                std::ostream&                               output ) {

  size_t i;

  // For each of the intervals in the input map sequence, compute an interpolant.
  for ( i = 0; i < inputMaps.size (); ++i ) {

    // reset the maps holding which points have been picked.
    inputMaps [ i ]->initialise ();
    inputMaps [ i ]->printBoundaryMap ( output );
    inputMaps [ i ]->printInteriorMap ( output );
    inputMaps [ i ]->printExteriorMap ( output );
  }

}

//------------------------------------------------------------//

void GeoMorphAllochthonousLithology::computeTransformation ( const GeoMorphAllochthonousLithologyDistribution* lower,
                                                                const GeoMorphAllochthonousLithologyDistribution* upper,
                                                                      Numerics::GeometryVector&           translation,
                                                                      Numerics::GeometryVector&           scaling ) const {

  FloatingPoint deltaX;
  FloatingPoint deltaY;

  FloatingPoint mapWidth;
  FloatingPoint mapHeight;

  // Do not use the first entries, the 0th entries, since these are not of the delt of the input mesh.
  // The map has been extended by some multiple of the input map delta.
  deltaX = lower->getXCoordinates ()[ 2 ] - lower->getXCoordinates ()[ 1 ];
  deltaY = lower->getYCoordinates ()[ 2 ] - lower->getYCoordinates ()[ 1 ];

  // Again, note the disregard of the end points, since these are extensions of the input map.
  mapWidth  = lower->getXCoordinates ()[ lower->mapXDimension ()] - lower->getXCoordinates ()[ 1 ];
  mapHeight = lower->getYCoordinates ()[ lower->mapYDimension ()] - lower->getYCoordinates ()[ 1 ];

  translation ( 2 ) = upper->getAge ();
  scaling ( 2 ) = TimeDimensionScaling / ( lower->getAge () - upper->getAge ());

  translation ( 0 ) = lower->getXCoordinates ()[ 1 ];
  translation ( 1 ) = lower->getYCoordinates ()[ 1 ];

  scaling ( 0 ) = SpaceDimensionScaling / mapWidth;
  scaling ( 1 ) = SpaceDimensionScaling / mapHeight;
}

//------------------------------------------------------------//

void GeoMorphAllochthonousLithology::saveInterpolant ( hid_t& fileId,
                                                          const std::string& fileName ) {

  database::Record* allochthonousLithologyInterpolationRecord;

  database::Table* allochTbl = m_projectHandle->getTable ( "AllochthonLithoInterpIoTbl" );

  std::string formationGroupName;
  std::stringstream interpolationGroupName;
  unsigned int i;
  unsigned int j;
  unsigned int k;
  int bufferCount;

  formationGroupName = "/Formation=" + (dynamic_cast<const Interface::Formation*>(m_formation))->getName ();

  // Create a group named /MyFormation in the file.
  hid_t formationGroupId = H5Gcreate(fileId, formationGroupName.c_str (), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

  hid_t interpolationGroupId;

  for ( i = 0; i < interpolators.size (); ++i ) {

    const IntervalInterpolator::RBFInterpolator& interpolator = interpolators [ i ]->getInterpolator ();
    const PointArray& interpolationPoints = interpolator.getInterpolationPoints ();

    allochthonousLithologyInterpolationRecord = allochTbl->createRecord ();

    interpolationGroupName.str ( "" );
    interpolationGroupName << "Interpolant=" << i + 1;

    // Create a group named /Interpolation='some-number' in the file.
    interpolationGroupId = H5Gcreate ( formationGroupId, interpolationGroupName.str ().c_str (), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );

    // Set the allochthonous lithology results table.
    database::setLayerName ( allochthonousLithologyInterpolationRecord, (dynamic_cast<const Interface::Formation*>(m_formation))->getName ());

    database::setInterpFileName ( allochthonousLithologyInterpolationRecord, fileName );
    database::setNumberOfPoints ( allochthonousLithologyInterpolationRecord, interpolationPoints.size ());
    database::setPolynomialDegree ( allochthonousLithologyInterpolationRecord, interpolator.getPolynomialDegree ());
    database::setInterpGroupName ( allochthonousLithologyInterpolationRecord, interpolationGroupName.str ());
    database::setStartAge ( allochthonousLithologyInterpolationRecord, interpolators [ i ]->getStartAge ());
    database::setEndAge ( allochthonousLithologyInterpolationRecord, interpolators [ i ]->getEndAge ());
    
    // Allocate a buffer for the interpolation points.
    double* pointBuffer = new double [ interpolationPoints.size () * Numerics::Point::DIMENSION ];

    // Copy the interpolation-points into the buffer.
    for ( j = 0, bufferCount = 0; j < interpolationPoints.size (); ++j ) {

      for ( k = 0; k < Numerics::Point::DIMENSION; ++k, ++bufferCount ) {
        pointBuffer [ bufferCount ] = interpolationPoints [ j ]( k );
      }

    }
    bool newDataSet = true;

    // Write the data to the results file.
    HDF5::writeData2D ( interpolationGroupId,
                        Numerics::Point::DIMENSION,
                        1,
                        Interface::AllochthonousLithologyInterpolation::ScalingDataSetName.c_str (),
                        H5T_NATIVE_DOUBLE,
                        interpolators [ i ]->getScaling ().data (), newDataSet );

    HDF5::writeData2D ( interpolationGroupId,
                        Numerics::Point::DIMENSION,
                        1,
                        Interface::AllochthonousLithologyInterpolation::TranslationDataSetName.c_str (),
                        H5T_NATIVE_DOUBLE,
                        interpolators [ i ]->getTranslation ().data (), newDataSet );

    HDF5::writeData2D ( interpolationGroupId,
                        interpolationPoints.size (),
                        Numerics::Point::DIMENSION,
                        Interface::AllochthonousLithologyInterpolation::PointsDataSetName.c_str (),
                        H5T_NATIVE_DOUBLE,
                        pointBuffer, newDataSet );

    HDF5::writeData2D ( interpolationGroupId,
                        interpolator.getCoefficients ().dimension (),
                        1,
                        Interface::AllochthonousLithologyInterpolation::CoefficientsDataSetName.c_str (),
                        H5T_NATIVE_DOUBLE, 
                        interpolator.getCoefficients ().data (), newDataSet );

    HDF5::writeData2D ( interpolationGroupId,
                        interpolators [ i ]->getInterpolationVector ().dimension (),
                        1,
                        Interface::AllochthonousLithologyInterpolation::RHSDataSetName.c_str (),
                        H5T_NATIVE_DOUBLE, 
                        interpolators [ i ]->getInterpolationVector ().data (), newDataSet );

    H5Gclose( interpolationGroupId );

    delete [] pointBuffer;
  }

  // Close the formation group
  H5Gclose(formationGroupId);

}

//------------------------------------------------------------//

MapIntervalInterpolator* GeoMorphAllochthonousLithology::allocateIntervalInterpolator () const {
  return new MapIntervalInterpolator;
}

}

//------------------------------------------------------------//
