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

#include "UndefinedValues.h"
#include "NumericFunctions.h"

#include <algorithm>

using namespace ibs;

namespace DataExtraction
{

HDFReadManager::HDFReadManager( const DataAccess::Interface::ProjectHandle& projectHandle ) :
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

   const std::string outputPath = m_projectHandle.getFullOutputDir();
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
   hid_t dataspace = H5Dget_space( datasetId );
   hsize_t dims[2];
   H5Sget_simple_extent_dims( dataspace , dims, NULL);
   H5Sclose( dataspace );

   if ( dims[0] < 2 || dims[1] < 2 )
   {
      return coordinatePropertyVector;
   }

   float** propertyData = Array<float>::create2d(2, 2);

   for ( const DoublePair& coordinate : coordinates )
   {
      const double i = coordinate.first;
      const double j = coordinate.second;

      const unsigned int i1 = std::min(static_cast<unsigned int>(i), static_cast<unsigned int>(dims[0] - 2));
      const double di = i - i1;
      const unsigned int j1 = std::min(static_cast<unsigned int>(j), static_cast<unsigned int>(dims[1] - 2));
      const double dj = j - j1;

      hsize_t offset[2] = { i1, j1 };
      hsize_t count[2] = { 2, 2 };
      hid_t dataspace = H5Dget_space( datasetId );
      H5Sselect_hyperslab( dataspace, H5S_SELECT_SET, offset, NULL, count, NULL );
      hid_t memspace = H5Screate_simple(2, count, NULL);
      H5Dread(datasetId, datatype, memspace, dataspace, H5P_DEFAULT, &propertyData[0][0] );

      const double ll = propertyData[0][0];
      const double lr = propertyData[1][0];
      const double tl = propertyData[0][1];
      const double tr = propertyData[1][1];
      coordinatePropertyVector.push_back( interpolate2d( ll, lr, tl, tr, di, dj ) );

      H5Sclose( dataspace );
      H5Sclose( memspace );
   }
   Array<float>::delete2d(propertyData);

   H5Dclose( datasetId );

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

   hid_t dataspace = H5Dget_space( datasetId );
   hsize_t dims[3];
   H5Sget_simple_extent_dims( dataspace , dims, NULL);
   H5Sclose( dataspace );

   if ( dims[0] < 2 || dims[1] < 2 )
   {
      return coordinatePropertyMatrix;
   }

   float*** propertyData = Array<float>::create3d(2, 2, dims[2]);

   for ( const DoublePair& coordinate : coordinates )
   {
      const double i = coordinate.first;
      const double j = coordinate.second;

      const unsigned int i1 = std::min(static_cast<unsigned int>(i), static_cast<unsigned int>(dims[0] - 2));
      const double di = i - i1;
      const unsigned int j1 = std::min(static_cast<unsigned int>(j), static_cast<unsigned int>(dims[1] - 2));
      const double dj = j - j1;

      hsize_t offset[3] = { i1, j1, 0};
      hsize_t count[3] = { 2, 2, dims[2]};
      dataspace = H5Dget_space( datasetId );
      H5Sselect_hyperslab( dataspace, H5S_SELECT_SET, offset, NULL, count, NULL );
      hid_t memspace = H5Screate_simple(3, count, NULL);
      H5Dread(datasetId, datatype, memspace, dataspace, H5P_DEFAULT, &propertyData[0][0][0] );

      std::vector<double> propertyVec;
      for ( int zi = 0; zi < dims[2]; ++zi )
      {
         const double ll = propertyData[0][0][zi];
         const double lr = propertyData[1][0][zi];
         const double tl = propertyData[0][1][zi];
         const double tr = propertyData[1][1][zi];

         const double interpolatedValue = interpolate2d( ll, lr, tl, tr, di, dj );

         if (Utilities::isValueUndefined(ll)
             || Utilities::isValueUndefined(lr)
             || Utilities::isValueUndefined(tl)
             || Utilities::isValueUndefined(tr))
         {
            //The requested point could be right on the edge. In that case, the value should still be written.
            if ( !(NumericFunctions::isEqual(interpolatedValue,ll,Utilities::Numerical::DefaultNumericalTolerance)
                   || NumericFunctions::isEqual(interpolatedValue,lr,Utilities::Numerical::DefaultNumericalTolerance)
                   || NumericFunctions::isEqual(interpolatedValue,tl,Utilities::Numerical::DefaultNumericalTolerance)
                   || NumericFunctions::isEqual(interpolatedValue,tr,Utilities::Numerical::DefaultNumericalTolerance)))
            {
               propertyVec.push_back( Utilities::Numerical::CauldronNoDataValue);
               continue;
            }
         }
         propertyVec.push_back( interpolatedValue );
      }

      coordinatePropertyMatrix.push_back(propertyVec);

      H5Sclose( dataspace );
      H5Sclose( memspace );
   }
   Array<float>::delete3d(propertyData);

   H5Dclose( datasetId );

   return coordinatePropertyMatrix;
}

double HDFReadManager::interpolate2d( const double ll, const double lr, const double tl, const double tr, const double di, const double dj )
{
   const double x1 = 1.0-di;
   const double x2 = di;

   return ((x1*ll)+(x2*lr))*(1-dj) + ((x1*tl)+(x2*tr))*(dj);
}

} // namespace DataExtraction
