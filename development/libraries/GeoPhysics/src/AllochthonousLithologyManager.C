#include <iostream>
#include "AllochthonousLithologyManager.h"

#include "RBFGeneralOperations.h"

using namespace DataAccess;
using namespace std;


const std::string GeoPhysics::AllochthonousLithologyManager::ScalingDataSetName = "Scaling";
const std::string GeoPhysics::AllochthonousLithologyManager::TranslationDataSetName = "Translation";
const std::string GeoPhysics::AllochthonousLithologyManager::PointsDataSetName = "Points";
const std::string GeoPhysics::AllochthonousLithologyManager::CoefficientsDataSetName = "Coefficients";
const std::string GeoPhysics::AllochthonousLithologyManager::RHSDataSetName = "RHS";

//------------------------------------------------------------//

GeoPhysics::AllochthonousLithologyManager::AllochthonousLithologyManager ( ProjectHandle* projectHandle ) : m_projectHandle ( projectHandle ) {
}

//------------------------------------------------------------//

GeoPhysics::AllochthonousLithologyManager::~AllochthonousLithologyManager () {

  AllochthonousLithologyInterpolatorMap::iterator interpIter;
  IntervalInterpolatorSequence::iterator          intervalIter;

  for ( interpIter = interpolators.begin (); interpIter != interpolators.end (); ++interpIter ) {
    delete (*interpIter).second;
  }

  for ( intervalIter = intervalInterpolators.begin (); intervalIter != intervalInterpolators.end (); ++intervalIter ) {
    delete (*intervalIter);
  }

}

//------------------------------------------------------------//

void GeoPhysics::AllochthonousLithologyManager::setGridDescription ( const CauldronGridDescription& newDescription ) {

  AllochthonousLithologyInterpolatorMap::iterator iter;

  for ( iter = interpolators.begin (); iter != interpolators.end (); ++iter ) {
    iter->second->setGridDescription ( newDescription );
  }

}

//------------------------------------------------------------//

void GeoPhysics::AllochthonousLithologyManager::initialiseLayers () {

   AllochthonousLithologyInterpolatorMap::iterator iter;
   GeoPhysics::Formation*                          allochthonousLayer;

   for ( iter = interpolators.begin (); iter != interpolators.end (); ++iter ) {
      allochthonousLayer = dynamic_cast<GeoPhysics::Formation*>(const_cast<Interface::Formation*>(m_projectHandle->findFormation ( iter->first )));

      if ( allochthonousLayer == 0 ) {
         // Error
      }

      allochthonousLayer->setAllochthonousLayer ( iter->second );
   }

}

//------------------------------------------------------------//

bool GeoPhysics::AllochthonousLithologyManager::allochthonousModellingRequired ( database::Database* projectDatabase ) const {

  database::Table* runOptionsTable = projectDatabase->getTable ( "RunOptionsIoTbl" );

  return database::getAllochthonousModelling ( runOptionsTable, 0 ) == 1;
}

//------------------------------------------------------------//

bool GeoPhysics::AllochthonousLithologyManager::allochthonousModellingRequired ( database::Database* projectDatabase,
                                                                                 const std::string&  formationName ) const {

  size_t i;
  database::Table* stratTable = projectDatabase->getTable ( "StratIoTbl" );

  for ( i = 0; i < stratTable->size (); i++ ) {

    if ( database::getLayerName ( stratTable, i ) == formationName ) {
      return database::getHasAllochthonLitho ( stratTable, i ) == 1;
    }

  }

  return false;
}

//------------------------------------------------------------//

bool GeoPhysics::AllochthonousLithologyManager::initialiseInterpolators ( database::Database* projectDatabase,
                                                                          const std::string&  directoryName ) {

  if ( ! allochthonousModellingRequired ( projectDatabase )) {
    // If the global switch is OFF then do not read any of the data.
    return true;
  }

  database::Table* allochLithoTable = projectDatabase->getTable ( "AllochthonLithoIoTbl" );
  database::Table* allochInterpTable = projectDatabase->getTable ( "AllochthonLithoInterpIoTbl" );
  database::Table::iterator iter;
  AllochthonousLithologyInterpolator* lithoInterp;
  CompoundLithology* allochthonousLithology;
  bool initialisedOkay;
  hid_t fileId;
  std::string fullFileName;

  for ( iter = allochLithoTable->begin (); iter != allochLithoTable->end (); ++iter ) {
    allochthonousLithology = m_projectHandle->getLithologyManager ().getCompoundLithology ( database::getLithotype ( *iter ));

    if ( allochthonousLithology == 0 ) {
      cout << "MeSsAgE  ERROR Could not find the lithology " 
           << database::getLithotype ( *iter ) 
           << " required by the allochthonous modelling " << endl;
      return false;
    }

    if ( allochthonousModellingRequired ( projectDatabase, database::getLayerName ( *iter ))) {
      // If the layer switch is off then do not create the interpolator.
      lithoInterp = new AllochthonousLithologyInterpolator ( allochthonousLithology );
      interpolators [ database::getLayerName ( *iter )] = lithoInterp;
    }

  }

  for ( iter = allochInterpTable->begin (); iter != allochInterpTable->end (); ++iter ) {
    fullFileName = directoryName + database::getInterpFileName ( *iter );

    if ( interpolators.find ( database::getLayerName ( *iter )) != interpolators.end ()) {
      fileId = H5Fopen ( fullFileName.c_str (), H5F_ACC_RDONLY, H5P_DEFAULT );

      if ( fileId < 0 ) {
        cout << "MeSsAgE  ERROR Could not open HDF file " 
             << database::getInterpFileName ( *iter ) 
             << " required by the allochthonous modelling " << endl;
        return false;
      }

      initialisedOkay = initialiseInterpolator ( fileId, *iter );
      H5Fclose ( fileId );

      if ( ! initialisedOkay ) {
        cout << "MeSsAgE  ERROR Could not create the interpolator " << endl;
        return false;
      }

    }

  }

  return true;
}

//------------------------------------------------------------//

void GeoPhysics::AllochthonousLithologyManager::freeze ( const int optimisationLevel ) {

  AllochthonousLithologyInterpolatorMap::iterator interpIter;

  for ( interpIter = interpolators.begin (); interpIter != interpolators.end (); ++interpIter ) {
    (*interpIter).second->freeze ( optimisationLevel );
  }

}

//------------------------------------------------------------//

void GeoPhysics::AllochthonousLithologyManager::reset () {

  AllochthonousLithologyInterpolatorMap::iterator interpIter;

  for ( interpIter = interpolators.begin (); interpIter != interpolators.end (); ++interpIter ) {
    (*interpIter).second->reset ();
  }

}

//------------------------------------------------------------//

bool GeoPhysics::AllochthonousLithologyManager::initialiseInterpolator ( hid_t&            fileId,
                                                                         database::Record* interpRecord ) {

  int bufferCount;
  int i;
  int j;

  std::string formationName;
  std::string interpolationGroupName;
  int numberOfInterpolationPoints;
  int polynomialDegree;
  double endAge;
  double startAge;

  double* buffer;

  hid_t formationGroupId;
  hid_t interpolationGroupId;
  hid_t dataSetId;
  herr_t status;

  formationName = database::getLayerName ( interpRecord );
  numberOfInterpolationPoints = database::getNumberOfPoints ( interpRecord );
  polynomialDegree = database::getPolynomialDegree ( interpRecord );
  interpolationGroupName = database::getInterpGroupName ( interpRecord );
  startAge = database::getStartAge ( interpRecord );
  endAge = database::getEndAge ( interpRecord );

#if 0
  cout << " Formation name   : " << formationName << endl;
  cout << " number of points : " << numberOfInterpolationPoints<< endl;
  cout << " polynomialDegree : " << polynomialDegree<< endl;
  cout << " Group id         : " << interpolationGroupName<< endl;
  cout << " Interval         : " << startAge << "  " << endAge << endl;
#endif

  Numerics::IntervalInterpolator* interp = new Numerics::IntervalInterpolator;

  // Store recently allocated interval interpolator on interval interpolator sequence.
  intervalInterpolators.push_back ( interp );

  Numerics::PointArray interpolationPoints ( numberOfInterpolationPoints );
  Numerics::Vector     interpolationCoefficients ( numberOfInterpolationPoints + Numerics::numberOfPolynomialTerms<Numerics::Point::DIMENSION> ( polynomialDegree ));
  Numerics::GeometryVector translation;
  Numerics::GeometryVector scaling;

  // The buffer used when reading data from the HDF file.
  // The size of this buffer here is large enough for all data that is to be read.
  // So there is no need to de-allocate and re-allocate buffers of differing sizes.
  buffer = new double [ numberOfInterpolationPoints * Numerics::Point::DIMENSION ];

  std::string formationGroupStr = "/Formation=" + database::getLayerName ( interpRecord );

  formationGroupId = H5Gopen ( fileId, formationGroupStr.c_str (), NULL );
  interpolationGroupId = H5Gopen ( formationGroupId, interpolationGroupName.c_str (), NULL );

  // Read-in data from HDF file.

  // Read interpolation point data.
  dataSetId = H5Dopen ( interpolationGroupId, GeoPhysics::AllochthonousLithologyManager::PointsDataSetName.c_str (), NULL );

  status = H5Dread ( dataSetId, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, buffer );
  H5Dclose ( dataSetId );

  if ( status < 0 ) {
    cout << "MeSsAgE  ERROR Could not read in data for the 'Points' group from HDF file " << endl;
    return false;
  }

  // Copy the interpolation-points from the buffer.
  for ( i = 0, bufferCount = 0; i < numberOfInterpolationPoints; ++i ) {

    for ( j = 0; j < Numerics::Point::DIMENSION; ++j, ++bufferCount ) {
      interpolationPoints [ i ]( j ) = buffer [ bufferCount ];
    }

  }

  // Read interpolation coefficients.
  dataSetId = H5Dopen ( interpolationGroupId, GeoPhysics::AllochthonousLithologyManager::CoefficientsDataSetName.c_str (), NULL );

  // The buffer here will be more than large enough to hold all of the coefficients.
  status = H5Dread ( dataSetId, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, buffer );
  H5Dclose ( dataSetId );

  if ( status < 0 ) {
    cout << "MeSsAgE  ERROR Could not read in data for the 'Coefficients' group from HDF file " << endl;
    return false;
  }

  // Copy the interpolation-coefficients from the buffer.
  for ( i = 0; i < interpolationCoefficients.dimension (); ++i ) {
    interpolationCoefficients ( i ) = buffer [ i ];
  }

  // Read transformation data.
  //
  // Scaling
  dataSetId = H5Dopen ( interpolationGroupId, GeoPhysics::AllochthonousLithologyManager::ScalingDataSetName.c_str (), NULL );

  status = H5Dread ( dataSetId, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, buffer );
  H5Dclose ( dataSetId );

  if ( status < 0 ) {
    cout << "MeSsAgE  ERROR Could not read in data for the 'Scaling' group from HDF file " << endl;
    return false;
  }

  for ( i = 0; i < Numerics::Point::DIMENSION; ++i ) {
    scaling ( i ) = buffer [ i ];
  }

  // Translation
  dataSetId = H5Dopen ( interpolationGroupId, GeoPhysics::AllochthonousLithologyManager::TranslationDataSetName.c_str (), NULL );

  status = H5Dread ( dataSetId, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, buffer );
  H5Dclose ( dataSetId );

  if ( status < 0 ) {
    cout << "MeSsAgE  ERROR Could not read in data for the 'Translation' group from HDF file " << endl;
    return false;
  }

  for ( i = 0; i < Numerics::Point::DIMENSION; ++i ) {
    translation ( i ) = buffer [ i ];
  }

  // END OF Read-in data from HDF file.

  // Now that all of the data has been read from the file, the interval 
  // interpolator can be initialised.
  interp->setPolynomialDegree ( polynomialDegree );
  interp->setInterpolationPoints ( interpolationPoints );
  interp->setInterpolationCoefficients ( interpolationCoefficients );
  interp->setTransformation ( translation, scaling );
  interp->setIntervalBounds ( startAge, endAge );

  delete [] buffer;
  H5Gclose( interpolationGroupId );

  H5Gclose( formationGroupId );

  // Now add the interval interpolator to the interpolator sequence for the formation.
  AllochthonousLithologyInterpolatorMap::iterator allochthonousInterpolator = interpolators.find ( getLayerName ( interpRecord ));

  if ( allochthonousInterpolator != interpolators.end ()) {
    // Add the interpolant to the correct formation interpolant.
    allochthonousInterpolator->second->insert ( interp );
  } else {
    cout << "MeSsAgE  ERROR Could not find formation " << getLayerName ( interpRecord ) << " to add the interpolant." << endl;
    return false;
  }

  return true;
}
