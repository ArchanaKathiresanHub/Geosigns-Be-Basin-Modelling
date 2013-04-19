//
// Petsc Vector Read/Write with H5
//

//#include "petscvector_readwrite.h"
/*
void PetscVector_ReadWrite::createLocalInfo (DA& globalDa, Vec &globalVector, 
                                             Petsc_Array **localVec, 
                                             DALocalInfo &info, PetscDimensions *petscD)
{
   // create global vector
   petscD->createGlobalVector (globalDa, globalVector);

   // create local vector
   *localVec = petscD->createLocalVector (globalDa, globalVector);
    
   // get local info
   DAGetLocalInfo (globalDa, &info);
}
*/

/*
// write raw data to file, given local DA info
bool PetscVector_ReadWrite::writeRawData (H5_Write_File *h5File, hid_t locId, 
                                          const char *dataset, hid_t dataType, 
                                          DALocalInfo &localVecInfo, void *buffer, 
                                          H5_PropertyList *pList) 
{
   bool status = false;

   // create size and offset from Local Vec Info
   H5_VectorBoundaries dataBounds (localVecInfo);

   // create memSpace
   H5_FixedSpace memSpace (dataBounds.localBounds());

   // create file space
   H5_FixedSpace fileSpace (dataBounds.globalBounds());

   // set hyperslab
   fileSpace.setHyperslab (dataBounds.localBounds(), dataBounds.offsetBounds());

   // create dataset
   hid_t dataId = h5File->addDataset (dataset, locId, dataType, fileSpace);
      
   if ( dataId > -1 )
   {
      // write to the dataset
      status = h5File->writeDataset (dataId, buffer, pList, 
                                     fileSpace.space_id(), 
                                     memSpace.space_id());
   }
 
   return status;
}
*/
