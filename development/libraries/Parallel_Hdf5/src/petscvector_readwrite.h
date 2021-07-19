//
// Petsc Vector Read/Write with H5
//

#ifndef __petscvector_readwrite__
#define __petscvector_readwrite__

#include "h5_parallel_file_types.h"
#include "buffer_readwrite.h"
#include "petscdimensions.h"
#include "h5_vector_conversions.h"
#include "h5_file_types.h"

template <class Type> 
class PetscVector_ReadWrite : public Buffer_ReadWrite
{
private:
   typedef pair <H5_FixedSpace*, H5_FixedSpace*> DataSpace;

public:
   PetscVector_ReadWrite (void) : Buffer_ReadWrite () {}

   bool write (H5_Write_File *h5File, hid_t locId, const char *dataset, DM& globalDa,
               Vec &globalVector, PetscDimensions *petscD,
               hid_t dataType)
   {

      // create local vector
      Petsc_Array *localVec = petscD->createLocalVector (globalDa, globalVector);

      // create 1d buffer for writing
      VectorToBuffer<Type> vecToBuff;
      Type *buffer = vecToBuff.convert (localVec);

      // get local info
      DMDALocalInfo localVecInfo;
      DMDAGetLocalInfo (globalDa, &localVecInfo);
     
      bool status = writeRawData (h5File, locId, dataset, dataType, localVecInfo, 
                                 (void*)buffer);

      if (!status)
      {
         status = overWriteRawData (h5File, locId, dataset, dataType, localVecInfo, (void*)buffer);
      }

      // clean up
      delete localVec;

      return status;
   }
  
   bool read (H5_ReadOnly_File *h5File, const char *dataset,
              DM& globalDa, Vec &globalVector, PetscDimensions *petscD)
   {
      // get data set from file
      hid_t dataId = h5File->openDataset (dataset);
      if ( dataId < 0 ) return false;

      // get local info
      Petsc_Array *localVec;
      DMDALocalInfo localVecInfo;
      createLocalInfo (globalDa, globalVector, &localVec, localVecInfo, petscD);

      Type *buffer = 0;
      bool status = collectRawData (h5File, dataId, localVecInfo, &buffer, 
                                    localVec->linearSize());

      // convert 1D buffer into local Vector data
      BufferToVector<Type> buffToVec;
      buffToVec.convert (buffer, localVec);

      // clean up
      delete [] buffer;
      delete localVec;
    
      return status;
   }
  
   // collect raw data based on DA size and offsets
   bool collectRawData (H5_ReadOnly_File *h5File, hid_t dataId, DMDALocalInfo &localVecInfo, 
                        Type **buffer, int size, bool allocateBuffer = true )
   {
      // get file space from data set
      H5_FixedSpace fileSpace (H5Dget_space (dataId));

      // set up offset and sizes for hyperslab
      H5_VectorBoundaries dataBounds (localVecInfo);

      // set hyperslab
      fileSpace.setHyperslab (dataBounds.localBounds(),
                              dataBounds.offsetBounds());

      // create data space
      H5_FixedSpace memSpace (dataBounds.localBounds());

      // read data from the dataset
      if ( allocateBuffer ) {
	    *buffer = new Type [size];
      }
     
      return h5File->readDataset (dataId, *buffer,
                                  fileSpace.space_id(),
                                  memSpace.space_id());
      //return h5File->readDataset (dataId, *buffer, pList);
   }  

   // create local info based on global info
   static void createLocalInfo (DM& globalDa, Vec &globalVector, Petsc_Array **localVec, 
                                DMDALocalInfo &info, PetscDimensions *petscD)
   {
      // create global vector
      // globalVector should be already allocated
      if( globalVector == PETSC_IGNORE ) {
         petscD->createGlobalVector (globalDa, globalVector);
      }
      // create local vector
      *localVec = petscD->createLocalVector (globalDa, globalVector);
    
      // get local info
      DMDAGetLocalInfo (globalDa, &info);
   }

   // write raw data to file, given local DA info
   static bool writeRawData (H5_Write_File *h5File, hid_t locId, const char *dataset, 
                             hid_t dataType, DMDALocalInfo &localVecInfo, void *buffer)
   {
      bool status = false;

      // create file space
      DataSpace dataSpace = createHyperslabFilespace (localVecInfo, h5File->chunks() );

      // create dataset
      hid_t dataId = h5File->addDataset (dataset, locId, dataType, *(dataSpace.first), dataSpace.second);
      

      if ( dataId > -1 )
      {
          // write to the dataset
          status = h5File->writeDataset (dataId, buffer,
                                         (dataSpace.first)->space_id(), 
                                         (dataSpace.second)->space_id());
          H5Dclose (dataId);
      }
 
      delete dataSpace.first;
      delete dataSpace.second;

      return status;
   } 

   static bool overWriteRawData (H5_Write_File *h5File, hid_t locId, const char *dataset, 
                             hid_t dataType, DMDALocalInfo &localVecInfo, void *buffer)
   {
      bool status = false;


      // create dataset
      hid_t dataId = h5File->openDataset (dataset, locId);
      
      if ( dataId > -1 )
      {

        // create file space
        DataSpace dataSpace = createHyperslabFilespace (localVecInfo, h5File->chunks());

        // write to the dataset
        status = h5File->writeDataset (dataId, buffer,
                                       (dataSpace.first)->space_id(), 
                                       (dataSpace.second)->space_id());

	    H5Dclose (dataId);
        delete dataSpace.first;
        delete dataSpace.second;

      }
 

      return status;
   } 

   static DataSpace createHyperslabFilespace (DMDALocalInfo &localVecInfo, const bool useChunks = false )
   {
     // create size and offset from Local Vec Info
     H5_VectorBoundaries dataBounds (localVecInfo);

     // create memSpace
     H5_FixedSpace *memSpace = new H5_FixedSpace (dataBounds.localBounds());

     // create file space
     H5_FixedSpace *fileSpace = new H5_FixedSpace (dataBounds.globalBounds());

     // set hyperslab
     if( useChunks ) {
        fileSpace->setChunkedHyperslab (dataBounds.localBounds(), dataBounds.offsetBounds());
     } else {
        fileSpace->setHyperslab (dataBounds.localBounds(), dataBounds.offsetBounds());
     }

     return DataSpace (fileSpace, memSpace);
   }
};

#endif



