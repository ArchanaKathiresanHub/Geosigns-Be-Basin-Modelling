//
// Copyright ( C ) 2013-2019 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "hdfReadManager.h"

#include "ProjectHandle.h"
#include "Property.h"

namespace DataExtraction
{

HDFReadManager::HDFReadManager( const DataAccess::Interface::ProjectHandle* projectHandle ) :
  m_projectHandle( projectHandle ),
  m_mapsFileId( 0 ),
  m_snapshotFileId( 0 )
{
}

HDFReadManager::~HDFReadManager()
{
  closeSnapshotFile();
  closeMapsFile();
}

bool HDFReadManager::openSnapshotFile( const std::string& snapshotFileName )
{
  m_snapshotFileId = H5Fopen( snapshotFileName.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT );
  return m_snapshotFileId > 0;
}

bool HDFReadManager::checkDataGroup( const std::string& dataGroup )
{
  return ( H5Lexists( m_snapshotFileId, dataGroup.c_str(), NULL ) > 0 );
}

void HDFReadManager::closeSnapshotFile()
{
  if ( m_snapshotFileId > 0 )
  {
    H5Fclose( m_snapshotFileId );
    m_snapshotFileId = 0;
  }
}

void HDFReadManager::openMapsFile( const std::string& mapFileName )
{
  if ( mapFileName.empty() )
  {
    m_mapsFileId = 0;
    return;
  }

  const std::string outputPath = m_projectHandle->getFullOutputDir();
  const std::string mapsFilePath = outputPath + "/" + mapFileName;
  m_mapsFileId = H5Fopen( mapsFilePath.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT );
  if ( m_mapsFileId <= 0 )
  {
    std::cerr << "Unknown simulator mode used for fastcauldron" << std::endl;
  }
}

void HDFReadManager::closeMapsFile()
{
  if ( m_mapsFileId > 0 )
  {
    H5Fclose( m_mapsFileId );
    m_mapsFileId = 0;
  }
}

DoubleVector HDFReadManager::get2dCoordinatePropertyVector( const DoublePairVector& coordinates,
                                                            const std::string& propertyFormationDataGroup )
{
  DoubleVector coordinatePropertyVector;

  if ( m_mapsFileId <= 0 || H5Lexists( m_mapsFileId, propertyFormationDataGroup.c_str(), NULL ) <= 0 )
  {
    return coordinatePropertyVector;
  }

  const hid_t datasetId = H5Dopen2( m_mapsFileId, propertyFormationDataGroup.c_str(), H5P_DEFAULT );
  const hid_t datatype = H5Dget_type( datasetId );
  const hid_t dataspace = H5Dget_space( datasetId );
  hsize_t dims[2];
  H5Sget_simple_extent_dims( dataspace, dims, NULL );
  float** propertyData = Array<float>::create2d(dims[0], dims[1]);
  H5Dread( datasetId, datatype, H5S_ALL, H5S_ALL, H5P_DEFAULT, &propertyData[0][0] );
  H5Sclose( dataspace );
  H5Dclose( datasetId );

  for ( const DoublePair& coordinate : coordinates )
  {
    const double i = coordinate.first;
    const double j = coordinate.second;

    const unsigned int i1 = static_cast<unsigned int>(i);
    const double di = i - i1;
    const unsigned int j1 = static_cast<unsigned int>(j);
    const double dj = j - j1;

    const double ll = propertyData[i1  ][j1];
    const double lr = propertyData[i1+1][j1];
    const double tl = propertyData[i1  ][j1+1];
    const double tr = propertyData[i1+1][j1+1];
    coordinatePropertyVector.push_back( interpolate2d( ll, lr, tl, tr, di, dj ) );
  }
  Array<float>::delete2d(propertyData);

  return coordinatePropertyVector;
}

DoubleMatrix HDFReadManager::get3dCoordinatePropertyMatrix( const DoublePairVector& coordinates,
                                                            const std::string& propertyFormationDataGroup )
{
  DoubleMatrix coordinatePropertyMatrix;

  if ( m_snapshotFileId <= 0 || !checkDataGroup( propertyFormationDataGroup ) )
  {
    return coordinatePropertyMatrix;
  }

  const hid_t datasetId = H5Dopen2( m_snapshotFileId, propertyFormationDataGroup.c_str(), H5P_DEFAULT );
  const hid_t datatype = H5Dget_type( datasetId );
  const hid_t dataspace = H5Dget_space( datasetId );
  hsize_t dims[3];
  H5Sget_simple_extent_dims( dataspace , dims, NULL);
  float*** propertyData = Array<float>::create3d(dims[0], dims[1], dims[2]);
  H5Dread( datasetId, datatype, H5S_ALL, H5S_ALL, H5P_DEFAULT, &propertyData[0][0][0] );
  H5Sclose( dataspace );
  H5Dclose( datasetId );

  for ( const DoublePair& coordinate : coordinates )
  {
    const double i = coordinate.first;
    const double j = coordinate.second;

    const unsigned int i1 = static_cast<unsigned int>(i);
    const double di = i - i1;
    const unsigned int j1 = static_cast<unsigned int>(j);
    const double dj = j - j1;

    std::vector<double> propertyVec;
    for ( int zi = 0; zi < dims[2]; ++zi )
    {
      const double ll = propertyData[i1  ][j1  ][zi];
      const double lr = propertyData[i1+1][j1  ][zi];
      const double tl = propertyData[i1  ][j1+1][zi];
      const double tr = propertyData[i1+1][j1+1][zi];
      propertyVec.push_back( interpolate2d( ll, lr, tl, tr, di, dj ) );
    }
    coordinatePropertyMatrix.push_back(propertyVec);
  }
  Array<float>::delete3d(propertyData);

  return coordinatePropertyMatrix;
}

double HDFReadManager::interpolate2d( const double ll, const double lr, const double tl, const double tr, const double di, const double dj )
{
  const double x1 = 1.0-di;
  const double x2 = di;

  return ((x1*ll)+(x2*lr))*(1-dj) + ((x1*tl)+(x2*tr))*(dj);
}

} // namespace DataExtraction
