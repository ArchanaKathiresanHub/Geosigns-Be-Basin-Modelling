#include "Interface/DistributedMapWriter.h"
#include "Interface/DistributedGrid.h"
#include "Interface/DistributedGridMap.h"
#include "h5_parallel_file_types.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <ctime>
#include <sstream>
#include <iomanip>

#include "database.h"
#include "cauldronschema.h"

using namespace DataAccess;
using namespace Interface;
using namespace database;

const char *DistributedMapWriter::VERSION_DATASET_NAME = "/gioHDFfile version";
const char *DistributedMapWriter::NULL_VALUE_NAME = "/null value";
const char *DistributedMapWriter::NR_I_DATASET_NAME = "/number in I dimension";
const char *DistributedMapWriter::NR_J_DATASET_NAME = "/number in J dimension";
const char *DistributedMapWriter::ORIGIN_I_DATASET_NAME = "/origin in I dimension";
const char *DistributedMapWriter::ORIGIN_J_DATASET_NAME = "/origin in J dimension";
const char *DistributedMapWriter::DELTA_I_DATASET_NAME = "/delta in I dimension";
const char *DistributedMapWriter::DELTA_J_DATASET_NAME = "/delta in J dimension";
const char *DistributedMapWriter::PROJECT_DATASET_NAME = "/projectName";
const char *DistributedMapWriter::PROJECT_COMMENT_DATASET_NAME = "/projectComment";
const char *DistributedMapWriter::PROJECT_MISC_COMMENT_DATASET_NAME = "/projectMiscComment";
const char *DistributedMapWriter::PROJECT_PILOT_FILE_DATASET_NAME = "/projectPilotFile";
const char *DistributedMapWriter::STRATTOP_AGE_DATASET_NAME = "/StratTopAge";
const char *DistributedMapWriter::LAYER_DATASET_PREFIX = "/Layer=";
const char *DistributedMapWriter::MAX_K_DATASET_NAME = "/maxKsize";

const char *DistributedMapWriter::STRATTOP_AGE_ATTRIBUTE = "StratTopAge";
const char *DistributedMapWriter::STRATTOP_NAME_ATTRIBUTE = "StratTopName";
const char *DistributedMapWriter::STRATTOP_PROPERTY_ATTRIBUTE = "StratTopProp";
const char *DistributedMapWriter::PROPERTY_NAME_ATTRIBUTE = "PropertyName";
const char *DistributedMapWriter::GRID_NAME_ATTRIBUTE = "GridName";

DistributedMapWriter::DistributedMapWriter (void)
{
   m_fileName = "";
   m_outFile = 0;
   m_writer = 0;
}

bool DistributedMapWriter::open (const string & fileName, bool append)
{
#ifdef _WIN32
   size_t slashPos = fileName.rfind( '\\' );
#else
   size_t slashPos = fileName.rfind ('/');
#endif
   if (slashPos != string::npos)
   {
      m_fileName = fileName.substr (slashPos + 1, string::npos);
   }
   else
   {
      m_fileName = fileName;
   }

   if (append)
   {
      m_outFile = new H5_Append_File ();
   }
   else
   {
      m_outFile = new H5_New_File ();
   }

   // open in parallel mode
   H5_Parallel_PropertyList pList;

   bool opened = m_outFile->open (fileName.c_str (), &pList);

   // switch off hdf5 errors
   H5Eset_auto (NULL, NULL, NULL);

   // set up writer for parallel and serial
   m_writer = new PetscVector_ReadWrite<float> ();

   return opened;
}
void DistributedMapWriter::setChunking() {

   if( m_outFile != 0 and ( H5_Parallel_PropertyList::isPrimaryPodEnabled () or H5_Parallel_PropertyList::isOneFileLustreEnabled () )) {
      m_outFile->setChunking ( true );
   }
      
}
const string & DistributedMapWriter::getFileName (void)
{
   return m_fileName;
}

bool DistributedMapWriter::writeMapToHDF (GridMap * gridMap, float time, double depoAge,
                                          const string & propertyGrid, const string& surfaceName, const bool saveAsPrimary )
{
	return writeMapToHDF (dynamic_cast<DistributedGridMap*> (gridMap)->getDA (), dynamic_cast<DistributedGridMap*>(gridMap)->getVec (), 
                              time, depoAge, propertyGrid, surfaceName, saveAsPrimary); 
}
bool DistributedMapWriter::writeMapToHDF (DM & da, Vec & vec, float time, double depoAge,
                                          const string & propertyGrid, const string& surfaceName, const bool saveAsPrimary )
{
   bool returnVal = true;
   // Get the info about our part of the distributed array
   int start[2];
   int count[2];

   DMDAGetCorners (da, &start[0], &start[1], PETSC_IGNORE, &count[0], &count[1], PETSC_IGNORE);
   //*** get the info about the total model
   int size[2];

   DMDAGetInfo (da, PETSC_IGNORE, &size[0], &size[1], PETSC_IGNORE,
                PETSC_IGNORE, PETSC_IGNORE, PETSC_IGNORE, PETSC_IGNORE, PETSC_IGNORE, PETSC_IGNORE, PETSC_IGNORE, PETSC_IGNORE, PETSC_IGNORE);

   // create dataset name
   string dataSetName = LAYER_DATASET_PREFIX;

   dataSetName.insert (dataSetName.length (), "0");
   dataSetName = LAYER_DATASET_PREFIX + propertyGrid;

   // write the 2d data to file
   DMDALocalInfo localVecInfo;

   DMDAGetLocalInfo (da, &localVecInfo);
   localVecInfo.dim = 2;

   if( saveAsPrimary ) {
      double *data = new double[count[0] * count[1]];
      int writeIndex;
      
      PETSC_2D_Array x (da, vec, INSERT_VALUES, true);
      
      for (int yCnt = start[1]; yCnt < (start[1] + count[1]); yCnt++)
      {
         for (int xCnt = start[0]; xCnt < (start[0] + count[0]); xCnt++)
         {
            writeIndex = (yCnt - start[1]) + ((xCnt - start[0]) * count[1]);
            
            data[writeIndex] = x (yCnt, xCnt);
         }
      }
      PetscVector_ReadWrite<double> writeObj;
      returnVal = writeObj.writeRawData (m_outFile, m_outFile->fileId (), dataSetName.c_str (),
                                         H5T_NATIVE_DOUBLE, localVecInfo, (void *) data);
      delete [] data;
   } else {
      float *data = new float[count[0] * count[1]];
      int writeIndex;
      
      PETSC_2D_Array x (da, vec, INSERT_VALUES, true);
      
      for (int yCnt = start[1]; yCnt < (start[1] + count[1]); yCnt++)
      {
         for (int xCnt = start[0]; xCnt < (start[0] + count[0]); xCnt++)
         {
            writeIndex = (yCnt - start[1]) + ((xCnt - start[0]) * count[1]);
            
            data[writeIndex] = static_cast<float>( x (yCnt, xCnt) );
         }
      }
      returnVal = m_writer->writeRawData (m_outFile, m_outFile->fileId (), dataSetName.c_str (),
                                          H5T_NATIVE_FLOAT, localVecInfo, (void *) data);
      delete[]data;
  }

 //mademlis
//#if 0
   string tmpString = "NOP";

   returnVal = writeAttribute (dataSetName, PROPERTY_NAME_ATTRIBUTE, H5T_C_S1, 3, (void *) tmpString.c_str ());

   returnVal = writeAttribute (dataSetName, GRID_NAME_ATTRIBUTE, H5T_C_S1,
                               propertyGrid.length (), (void *) propertyGrid.c_str ());

   tmpString = "IMAGE";
   returnVal = writeAttribute (dataSetName, "CLASS", H5T_C_S1, 6, (void *) tmpString.c_str ());

   float fdepoAge = float (depoAge);

   Write1DDataSet (1, STRATTOP_AGE_DATASET_NAME, H5T_NATIVE_FLOAT, &fdepoAge);

   returnVal = writeAttribute (dataSetName, STRATTOP_AGE_ATTRIBUTE, H5T_NATIVE_FLOAT, 1, &time);

   tmpString = "RDO";
   returnVal = writeAttribute (dataSetName, STRATTOP_PROPERTY_ATTRIBUTE, H5T_C_S1, 3, (void *) tmpString.c_str ());


   if ( surfaceName != "" ) {
      string surfaceNameCopy = surfaceName;

      string::size_type pos;
      for (pos=0; pos<surfaceNameCopy.length(); pos++) {

         if (surfaceNameCopy[pos] == '_') {
            surfaceNameCopy[pos] = ' ';
         }

      }

      returnVal = writeAttribute (dataSetName, STRATTOP_NAME_ATTRIBUTE, H5T_C_S1,
                                  surfaceNameCopy.length (), (void *) surfaceNameCopy.c_str ());
   }

   // blankSpaceUnderscore (surfaceName);
    //mademlis
   //mademlis returnVal = writeAttribute (dataSetName, STRATTOP_NAME_ATTRIBUTE, H5T_C_S1,
   //mademlis                            surfaceName.length (), (void *) surfaceName.c_str ());
//mademlis #endif
   return returnVal;
}

bool DistributedMapWriter::writeInputMap( GridMap * gridMap, int mapSeqNumber )
{
   string dataSetName = std::string( LAYER_DATASET_PREFIX ) + ( mapSeqNumber < 10 ? "0" : "" ) + std::to_string( mapSeqNumber );

   // get map info
   DM & da = dynamic_cast<DistributedGridMap*>( gridMap )->getDA();
   int start[2];
   int count[2];

   DMDAGetCorners( da, &start[0], &start[1], PETSC_IGNORE, &count[0], &count[1], PETSC_IGNORE );

   // get the info about the total model
   int size[2];
   DMDAGetInfo( da, PETSC_IGNORE, &size[0], &size[1], PETSC_IGNORE, PETSC_IGNORE, PETSC_IGNORE, PETSC_IGNORE, PETSC_IGNORE, 
                PETSC_IGNORE, PETSC_IGNORE, PETSC_IGNORE, PETSC_IGNORE, PETSC_IGNORE );

   const Vec & vec = dynamic_cast<DistributedGridMap*>( gridMap )->getVec(); 

   float * data = new float[ count[0] * count[1] ];
      
   PETSC_2D_Array x( da, vec, INSERT_VALUES, true );
      
   for ( int yCnt = start[1]; yCnt < (start[1] + count[1]); ++yCnt )
   {
      for ( int xCnt = start[0]; xCnt < (start[0] + count[0]); ++xCnt )
      {
         int writeIndex = (yCnt - start[1]) + ((xCnt - start[0]) * count[1]);           
         data[writeIndex] = static_cast<float>( x( yCnt, xCnt ) );
      }
   }

   DMDALocalInfo localVecInfo;
   DMDAGetLocalInfo( da, &localVecInfo );
   localVecInfo.dim = 2;

   bool ok = m_writer->writeRawData( m_outFile, m_outFile->fileId(), dataSetName.c_str(), H5T_NATIVE_FLOAT, localVecInfo, data );
   delete [] data;

   if ( ok && mapSeqNumber == 0 ) // if this is a first map - write a description
   {
      ok = saveDescription( gridMap->getGrid() );
   }
   return ok;
}


// Added by V.R. Ambati (13/07/2011):
bool DistributedMapWriter::writeVolumeToHDF (GridMap * gridMap, const string & propertyName, const string & layerName, const bool primaryFlag )
{
   bool status = writeVolumeToHDF (dynamic_cast<DistributedGridMap*> (gridMap)->getDA(), dynamic_cast<DistributedGridMap*> (gridMap)->getVec(), propertyName, layerName, primaryFlag );
    return status;
}

bool DistributedMapWriter::writeVolumeToHDF (DM & da, Vec & vec, const string & propertyName, const string & layerName, const bool primaryFlag )
{
   if (!m_outFile) return false;

   PetscDimensions *petscD = new Petsc_3D;
   if (!petscD) return false;

   MPI_Barrier (PETSC_COMM_WORLD);

   hid_t propertyGroupId = m_outFile->openGroup (propertyName.c_str ());

   if (propertyGroupId < 0)
   {
      propertyGroupId = m_outFile->addGroup (propertyName.c_str ());
   }

   if (propertyGroupId < 0)
   {
      delete petscD;
      return false;
   }

   bool status;
   // write data 
   if( primaryFlag ) {
      hid_t dataType = H5T_NATIVE_DOUBLE;
      PetscVector_ReadWrite<double> writeObj;
      
      status = writeObj.write (m_outFile, propertyGroupId, layerName.c_str (), da, vec, petscD, dataType );
   } else {
      hid_t dataType = H5T_NATIVE_FLOAT;
      PetscVector_ReadWrite<float> writeObj;
      
      status = writeObj.write (m_outFile, propertyGroupId, layerName.c_str (), da, vec, petscD, dataType );
   }
   assert (status);

   H5Gclose (propertyGroupId);

   delete petscD;

   return status;
}


bool DistributedMapWriter::Write1DDataSet (const long size, const string & dataSetName, const hid_t dataType, const void *data)
{
   // for some reason the dataset fails to be created. Not sure why yet

   if (size == 0)
      return false;

   // create dimensions
   BufferDimensions h5dims (1, size);

   // write array to file
   bool ret = m_writer->writeDataset (m_outFile, m_outFile->fileId (), dataSetName.c_str (),
                                     data, h5dims, dataType);

   return ret;
}

bool DistributedMapWriter::writeAttribute (const string & dataSetName,
                                const char *attributeName, const hid_t attributeType, const hsize_t dims, void *data)
{
   // open dataset
   hid_t dataId = m_outFile->openDataset (dataSetName.c_str ());

   if (dataId > -1)
   {
      // create attribute space
      BufferDimensions h5Dim (1, dims);

      bool ret = m_writer->writeAttribute (m_outFile, dataId, attributeName, data,
                                          h5Dim, attributeType);

      m_outFile->closeDataset (dataId);

      return ret;
   }
   else
   {
      H5Eprint ( H5E_DEFAULT, 0 );

      cerr << endl << "DistributedMapWriter::writeAttribute Error: Cannot open dataset : " << dataSetName << endl;
      return false;
   }
}

bool DistributedMapWriter::saveDescription (const Grid * grid)
{
   int nrI = grid->numIGlobal ();

   Write1DDataSet (1, NR_I_DATASET_NAME, H5T_NATIVE_INT, &nrI);
   int nrJ = grid->numJGlobal ();

   Write1DDataSet (1, NR_J_DATASET_NAME, H5T_NATIVE_INT, &nrJ);

   float originI = static_cast<float>( grid->minIGlobal () );

   Write1DDataSet (1, ORIGIN_I_DATASET_NAME, H5T_NATIVE_FLOAT, &originI);
   float originJ = static_cast<float>( grid->minJGlobal () );

   Write1DDataSet (1, ORIGIN_J_DATASET_NAME, H5T_NATIVE_FLOAT, &originJ);

   float deltaI = static_cast<float>( grid->deltaIGlobal () );

   Write1DDataSet (1, DELTA_I_DATASET_NAME, H5T_NATIVE_FLOAT, &deltaI);
   float deltaJ = static_cast<float>( grid->deltaJGlobal () );

   Write1DDataSet (1, DELTA_J_DATASET_NAME, H5T_NATIVE_FLOAT, &deltaJ);

   float nullValue = DefaultUndefinedMapValue;

   Write1DDataSet (1, NULL_VALUE_NAME, H5T_NATIVE_FLOAT, &nullValue);

   int maxKsize = 1;

   Write1DDataSet (1, MAX_K_DATASET_NAME, H5T_NATIVE_INT, &maxKsize);

   int version = 0;

   Write1DDataSet (1, VERSION_DATASET_NAME, H5T_NATIVE_INT, &version);

   return true;
}

bool DistributedMapWriter::close (void)
{
   if (m_outFile)
   {
      if (m_outFile->isOpen ())
      {
	 m_outFile->close ();
      }
      delete m_outFile;
      m_outFile = 0;
   }

   m_fileName = "";

   if (m_writer)
   {
      delete m_writer;
      m_writer = 0;
   }

   return true;
}

DistributedMapWriter::~DistributedMapWriter ()
{
    close ();
}

