#include "SerialMapWriter.h"
#include "SerialGridMap.h"
#include "SerialGrid.h"

#include "hdf5funcs.h"

using namespace DataAccess;
using namespace Interface;

#include "database.h"
#include "cauldronschema.h"

using namespace database;

const char *SerialMapWriter::VERSION_DATASET_NAME = "/gioHDFfile version";
const char *SerialMapWriter::NULL_VALUE_NAME = "/null value";
const char *SerialMapWriter::NR_I_DATASET_NAME = "/number in I dimension";
const char *SerialMapWriter::NR_J_DATASET_NAME = "/number in J dimension";
const char *SerialMapWriter::ORIGIN_I_DATASET_NAME = "/origin in I dimension";
const char *SerialMapWriter::ORIGIN_J_DATASET_NAME = "/origin in J dimension";
const char *SerialMapWriter::DELTA_I_DATASET_NAME = "/delta in I dimension";
const char *SerialMapWriter::DELTA_J_DATASET_NAME = "/delta in J dimension";
const char *SerialMapWriter::PROJECT_DATASET_NAME = "/projectName";
const char *SerialMapWriter::PROJECT_COMMENT_DATASET_NAME = "/projectComment";
const char *SerialMapWriter::PROJECT_MISC_COMMENT_DATASET_NAME = "/projectMiscComment";
const char *SerialMapWriter::PROJECT_PILOT_FILE_DATASET_NAME = "/projectPilotFile";
const char *SerialMapWriter::STRATTOP_AGE_DATASET_NAME = "/StratTopAge";
const char *SerialMapWriter::LAYER_DATASET_PREFIX = "/Layer=";
const char *SerialMapWriter::MAX_K_DATASET_NAME = "/maxKsize";

const char *SerialMapWriter::STRATTOP_AGE_ATTRIBUTE = "StratTopAge";
const char *SerialMapWriter::STRATTOP_NAME_ATTRIBUTE = "StratTopName";
const char *SerialMapWriter::STRATTOP_PROPERTY_ATTRIBUTE = "StratTopProp";
const char *SerialMapWriter::PROPERTY_NAME_ATTRIBUTE = "PropertyName";
const char *SerialMapWriter::GRID_NAME_ATTRIBUTE = "GridName";

SerialMapWriter::SerialMapWriter (void)
{
   m_fileName = "";
   m_fileHandle = -1;
}

bool SerialMapWriter::open (const string & fileName, bool append)
{
   if (m_fileHandle >= 0) return false;
   
   if (!append)
   {
      m_fileHandle = H5Fcreate (fileName.c_str (), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
   }
   else
   {
      m_fileHandle = H5Fopen (fileName.c_str (), H5F_ACC_RDWR, H5P_DEFAULT);
   }

   if (m_fileHandle < 0)
      return false;
#ifdef _WIN32
   size_t slashPos = fileName.rfind ('\\');
#else
   size_t slashPos = fileName.rfind( '/' );
#endif
   if (slashPos != string::npos)
   {
      m_fileName = fileName.substr (slashPos + 1, string::npos);
   }
   else
   {
      m_fileName = fileName;
   }

   return true;
}

const string & SerialMapWriter::getFileName (void)
{
   return m_fileName;
}

bool SerialMapWriter::writeMapToHDF (GridMap * gridMap, float time, double depoAge,
                                     const string & propertyGrid, const string &, const bool)
{
   return writeMapToHDF(gridMap, time, depoAge, propertyGrid);
}

bool SerialMapWriter::writeMapToHDF (GridMap * gridMap, float time, double depoAge,
                            const string & propertyGrid)
{
   bool returnVal;



   string tmpString = "NOP";

   // create dataset name
   string dataSetName = LAYER_DATASET_PREFIX + propertyGrid;
   bool newDataset = true;
   returnVal = writeMapData( dataSetName, gridMap, newDataset );

   if ( newDataset )
   {
      returnVal = writeAttribute( dataSetName, PROPERTY_NAME_ATTRIBUTE, H5T_C_S1, 3, (void *)tmpString.c_str( ) );

      returnVal = writeAttribute( dataSetName, GRID_NAME_ATTRIBUTE, H5T_C_S1,
         propertyGrid.length( ), (void *)propertyGrid.c_str( ) );

      tmpString = "IMAGE";
      returnVal = writeAttribute( dataSetName, "CLASS", H5T_C_S1, 6, (void *)tmpString.c_str( ) );
      returnVal = writeAttribute( dataSetName, STRATTOP_AGE_ATTRIBUTE, H5T_NATIVE_FLOAT, 1, &time );

      tmpString = "RDO";
      returnVal = writeAttribute( dataSetName, STRATTOP_PROPERTY_ATTRIBUTE, H5T_C_S1, 3, (void *)tmpString.c_str( ) );
   }

   float fdepoAge = float( depoAge );
   Write1DDataSet( 1, STRATTOP_AGE_DATASET_NAME, H5T_NATIVE_FLOAT, &fdepoAge );

   return returnVal;
}

bool SerialMapWriter::writeInputMap( GridMap * gridMap, int mapSeqNumber )
{
    // The below code is changed to the code in the next line in order to make the mapSeqNumber consistent in GridMapIoTbl of Project3d file and the mapSeqNumber in HDF file
   //string dataSetName = std::string(LAYER_DATASET_PREFIX) + (mapSeqNumber < 10 ? "0" : "") + std::to_string(mapSeqNumber);
   string dataSetName = std::string( LAYER_DATASET_PREFIX ) + std::to_string( mapSeqNumber );
   bool newDataset = true;
   bool ok = writeMapData( dataSetName, gridMap, newDataset );

   ok = ok && newDataset; // this map must be written always as a new dataset!

   if ( ok && mapSeqNumber == 0 ) // if this is a first map - write a description
   {
      ok = saveDescription( gridMap->getGrid() );
   }
   return ok;
}

bool SerialMapWriter::writeMapData( const string & dataSetName, const GridMap * gridMap, bool & newDataset )
{
   int numI = gridMap->getGrid ()->numI ();
   int numJ = gridMap->getGrid ()->numJ ();

   float * dataArray = new float[numI * numJ];

   for (int i = 0; i < numI; ++i)
   {
      for (int j = 0; j < numJ; ++j)
      {
         dataArray[i * numJ + j] = static_cast<float>(gridMap->getValue ((unsigned int) i, (unsigned int) j, (unsigned int) 0));
      }
   }

   HDF5::writeData2D( m_fileHandle, numI, numJ, dataSetName.c_str( ), H5T_NATIVE_FLOAT, dataArray, newDataset );

   delete [] dataArray;

   return true;
}

bool SerialMapWriter::writeVolumeToHDF (GridMap *, const string &, const string &, const bool )
{
   // not yet implemented
   return false;
}

bool SerialMapWriter::Write1DDataSet (const long size, const string & dataSetName, const hid_t dataType, const void *data)
{
   return HDF5::writeData1D (m_fileHandle, size, dataSetName.c_str (), dataType, data);
}

bool SerialMapWriter::writeAttribute (const string & dataSetName,
                                const char *attributeName, const hid_t attributeType, const hsize_t dims, void *data)
{
   // open dataset
   return HDF5::writeAttribute (m_fileHandle, dataSetName.c_str (), attributeName, attributeType, dims, data);
}

bool SerialMapWriter::saveDescription (const Grid * grid)
{
   int nrI = grid->numIGlobal ();

   Write1DDataSet (1, NR_I_DATASET_NAME, H5T_NATIVE_INT, &nrI);
   int nrJ = grid->numJGlobal ();

   Write1DDataSet (1, NR_J_DATASET_NAME, H5T_NATIVE_INT, &nrJ);

   float originI = static_cast<float>(grid->minIGlobal ());

   Write1DDataSet (1, ORIGIN_I_DATASET_NAME, H5T_NATIVE_FLOAT, &originI);
   float originJ = static_cast<float>(grid->minJGlobal ());

   Write1DDataSet (1, ORIGIN_J_DATASET_NAME, H5T_NATIVE_FLOAT, &originJ);

   float deltaI = static_cast<float>(grid->deltaIGlobal ());

   Write1DDataSet (1, DELTA_I_DATASET_NAME, H5T_NATIVE_FLOAT, &deltaI);
   float deltaJ = static_cast<float>(grid->deltaJGlobal ());

   Write1DDataSet (1, DELTA_J_DATASET_NAME, H5T_NATIVE_FLOAT, &deltaJ);

   float nullValue = DefaultUndefinedMapValue;

   Write1DDataSet (1, NULL_VALUE_NAME, H5T_NATIVE_FLOAT, &nullValue);

   int maxKsize = 1;

   Write1DDataSet (1, MAX_K_DATASET_NAME, H5T_NATIVE_INT, &maxKsize);

   int version = 0;

   Write1DDataSet (1, VERSION_DATASET_NAME, H5T_NATIVE_INT, &version);

   return true;
}

bool SerialMapWriter::close (void)
{
   bool status = false;
   if (m_fileHandle >= 0)
   {
      H5Fclose (m_fileHandle);
      status = true;
   }
   m_fileHandle = -1;
   m_fileName = "";

   return status;
}

SerialMapWriter::~SerialMapWriter ()
{
    close ();
}

