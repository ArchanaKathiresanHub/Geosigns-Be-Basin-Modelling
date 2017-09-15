//
// Copyright (C) 2017 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include <string>
#include <memory>

#include "../../DataAccess/src/Interface/Grid.h"
#include "../../DataAccess/src/Interface/Interface.h"
#include "../../SerialDataAccess/src/Interface/SerialGrid.h"
#include "../../SerialDataAccess/src/Interface/SerialGridMap.h"
#include "../../SerialDataAccess/src/Interface/SerialMapWriter.h"
#include "../../utilities/src/array.h"
#include "FilePath.h"

#include "boost/filesystem.hpp"

#include <gtest/gtest.h>

static const double       s_minI = 0.0;
static const double       s_minJ = -2.0;
static const double       s_maxI = 10.0;
static const double       s_maxJ = 2.0;
static const int          s_numI = 11;
static const int          s_numJ = 5;
static const unsigned int s_depth = 3;
static const double       s_value = 23.0;
static const float        s_time = 17.f;
static const double       s_depoAge = 18.;
static const string       s_propertyGrid = "property";
static const string       s_surfaceName = "surface";

static const std::string fName = "testSerialMapWriter.HDF";

class TestWriter : public ::testing::Test,  public DataAccess::Interface::SerialMapWriter
{
public:
   std::unique_ptr<DataAccess::Interface::Grid> grid;
   std::unique_ptr<DataAccess::Interface::SerialGridMap> sGridMap;

   TestWriter()
   {
      // Remove the file if it's already there
      ibs::FilePath fpath(fName);
      fpath.remove();

      // Init gridmap
      grid.reset( new DataAccess::Interface::SerialGrid( s_minI, s_minJ, s_maxI, s_maxJ, s_numI, s_numJ ) );
      float *** array = Array < float >::create3d (grid->numIGlobal(), grid->numJGlobal(), s_depth);
      for (int i = 0; i < grid->numIGlobal(); ++i)
      {
         for (int j = 0; j < grid->numJGlobal(); ++j)
         {
            for (unsigned int k = 0; k < s_depth; ++k)
            {
               array[i][j][k] = s_value;
            }
         }
      }
      sGridMap.reset( new DataAccess::Interface::SerialGridMap(nullptr, 0, grid.get(), DataAccess::Interface::DefaultUndefinedMapValue, s_depth, array) );
      Array < float >::delete3d (array);
   }

   ~TestWriter()
   {
      // Remove the file
      ibs::FilePath fpath(fName);
      fpath.remove();
   }

   void compare1Ddata( const hid_t fileId, const std::string & dataset, const float compVal )
   {
      hid_t datasetId = H5Dopen2(fileId, dataset.c_str(), H5P_DEFAULT);
      if( datasetId < 0 ) FAIL();
      float value;
      herr_t status = H5Dread(datasetId, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, &value);
      if( status < 0 ) FAIL();
      EXPECT_EQ( value, compVal );
      status = H5Dclose( datasetId );
   }

   void compare1Ddata( const hid_t fileId, const std::string & dataset, const int compVal )
   {
      hid_t datasetId = H5Dopen2(fileId, dataset.c_str(), H5P_DEFAULT);
      if( datasetId < 0 ) FAIL();
      int value;
      herr_t status = H5Dread(datasetId, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, &value);
      if( status < 0 ) FAIL();
      EXPECT_EQ( value, compVal );
      status = H5Dclose( datasetId );
   }

   void compare2Ddata( const hid_t fileId, const std::string & dataset )
   {
      hid_t datasetId = H5Dopen2(fileId, dataset.c_str(), H5P_DEFAULT);
      if( datasetId < 0 ) FAIL();

      EXPECT_TRUE( H5Dget_storage_size(datasetId) == s_numI*s_numJ*sizeof(float) );

      std::unique_ptr<float[]> valuesArr(new float[s_numI*s_numJ]);
      herr_t status = H5Dread(datasetId, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, valuesArr.get());
      if( status < 0 ) FAIL();
      for( int i = 0; i < s_numI*s_numJ; ++i )
         EXPECT_DOUBLE_EQ( valuesArr[i], s_value );
      status = H5Dclose( datasetId );
   }
};



TEST_F( TestWriter, Constructor )
{
   EXPECT_TRUE( getFileName().empty() );
   EXPECT_FALSE( open("") );
}


TEST_F( TestWriter, OpenClose )
{
   EXPECT_TRUE( getFileName().empty() );

   EXPECT_TRUE( open(fName) );
   EXPECT_TRUE( getFileName() == fName );
   EXPECT_TRUE( close() );
}


TEST_F( TestWriter, AppendInvalid )
{
   EXPECT_FALSE( open("MissingFile", true) );
}


TEST_F( TestWriter, AppendInvalidPermission )
{
   EXPECT_TRUE( open(fName) );
   EXPECT_TRUE( close() );
   
   const boost::filesystem::path path( fName );
   const boost::filesystem::perms oldPerm = boost::filesystem::status( path ).permissions();
   boost::filesystem::permissions( path, boost::filesystem::owner_read );
   EXPECT_FALSE( open(fName) );
   EXPECT_FALSE( open(fName, true) );
   
   boost::filesystem::permissions( path, oldPerm );
}


TEST_F( TestWriter, writeMapToHDF )
{
   EXPECT_TRUE( open(fName) );
   EXPECT_TRUE( writeMapToHDF( sGridMap.get(), s_time, s_depoAge, s_propertyGrid, s_surfaceName ) );
   EXPECT_TRUE( close() );

   // Check HDF file content
   hid_t fileId = H5Fopen(fName.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
   if( fileId < 0 ) FAIL();

   compare2Ddata(fileId, std::string(DataAccess::Interface::SerialMapWriter::LAYER_DATASET_PREFIX)+s_propertyGrid );

   H5Fclose(fileId);
}


TEST_F( TestWriter, writeMapToHDFappend )
{
   EXPECT_TRUE( open(fName) );
   EXPECT_TRUE( writeMapToHDF( sGridMap.get(), s_time, s_depoAge, std::string(s_propertyGrid+"1"), s_surfaceName ) );
   EXPECT_TRUE( close() );
   
   EXPECT_TRUE( open(fName, true) );
   EXPECT_TRUE( writeMapToHDF( sGridMap.get(), s_time, s_depoAge, std::string(s_propertyGrid+"2"), s_surfaceName ) );
   EXPECT_TRUE( close() );

   // Check HDF file content
   hid_t fileId = H5Fopen(fName.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
   if( fileId < 0 ) FAIL();

   for( int propIdx = 1; propIdx <= 2; ++propIdx )
      compare2Ddata(fileId, std::string(DataAccess::Interface::SerialMapWriter::LAYER_DATASET_PREFIX)+s_propertyGrid+std::to_string(propIdx) );

   H5Fclose(fileId);
}


TEST_F( TestWriter, writeInputMap )
{
   EXPECT_TRUE( open(fName) );
   EXPECT_TRUE( writeInputMap( sGridMap.get(), 0 ) );
   EXPECT_TRUE( close() );

   // Check HDF file content
   hid_t fileId = H5Fopen(fName.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
   if( fileId < 0 ) FAIL();
   
   compare1Ddata(fileId, DataAccess::Interface::SerialMapWriter::DELTA_I_DATASET_NAME,  static_cast<float>(sGridMap->deltaI()) );
   compare1Ddata(fileId, DataAccess::Interface::SerialMapWriter::DELTA_J_DATASET_NAME,  static_cast<float>(sGridMap->deltaJ()) );
   compare1Ddata(fileId, DataAccess::Interface::SerialMapWriter::VERSION_DATASET_NAME,  static_cast<int>(0) );
   compare1Ddata(fileId, DataAccess::Interface::SerialMapWriter::MAX_K_DATASET_NAME,    static_cast<int>(1) );
   compare1Ddata(fileId, DataAccess::Interface::SerialMapWriter::NULL_VALUE_NAME,       static_cast<float>(sGridMap->getUndefinedValue()) );
   compare1Ddata(fileId, DataAccess::Interface::SerialMapWriter::ORIGIN_I_DATASET_NAME, static_cast<float>(sGridMap->minI()) );
   compare1Ddata(fileId, DataAccess::Interface::SerialMapWriter::ORIGIN_J_DATASET_NAME, static_cast<float>(sGridMap->minJ()) );
   compare1Ddata(fileId, DataAccess::Interface::SerialMapWriter::NR_I_DATASET_NAME,     static_cast<int>(sGridMap->numI()) );
   compare1Ddata(fileId, DataAccess::Interface::SerialMapWriter::NR_J_DATASET_NAME,     static_cast<int>(sGridMap->numJ()) );
   compare2Ddata(fileId, std::string(DataAccess::Interface::SerialMapWriter::LAYER_DATASET_PREFIX)+std::string("00") );

   H5Fclose(fileId);
}
