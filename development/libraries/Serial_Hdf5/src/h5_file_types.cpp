//
// Classes with specific H5 file properties
//

#include "h5_file_types.h"
#include "mpi.h"

#include <iostream>
using namespace std;

//
// H5_File GENERAL METHODS
//
bool H5_Base_File::open (const char *filename, H5_PropertyList *propertyType)
{
   // create property list
   hPropertyListId = createPropertyList (propertyType);

   // create or open file 
   openInMode (filename);

   // close property list id
   H5Pclose (hPropertyListId);

   // check file okay
//   if ( hFileId < 1 ) 
//   {
//      cout << endl << "Could not open file " << filename << endl;
//      return false;
//   }

   // turn off errors by default
   H5Eset_auto( H5E_DEFAULT, 0, 0);

   return hFileId > 0;
}

void H5_Base_File::close (void)
{
   if ( ! isOpen () ) return;

   H5Fclose (hFileId);
   hFileId = (hid_t)0;

   if ( hPropertyListType )
   { 
      delete hPropertyListType;
   }
}

void H5_Base_File::setChunking( const bool useChunks )
{
   m_useChunks = useChunks;
}

hid_t H5_Base_File::createPropertyList (H5_PropertyList *userPropertyType)
{
   // use user-specified property type or default (serial)
   hPropertyListType = (userPropertyType ? userPropertyType->clone () 
                                         : hSerialPropertyType.clone ()); 

   return hPropertyListType->createFilePropertyList ( false );
}
hid_t H5_Base_File::createDatasetPropertyList (H5_PropertyList *propertyType )
{
   return (propertyType ? propertyType->createDatasetPropertyList ( false )
                        : hPropertyListType->createDatasetPropertyList ( false ));
}

hid_t H5_Base_File::openGroup (const char *name)
{
   return H5Gopen (hFileId, name, H5P_DEFAULT);
}

hid_t H5_Base_File::openGroup (const char *name, hid_t locId)
{
   return H5Gopen (locId, name, H5P_DEFAULT);
}

void H5_Base_File::closeGroup (hid_t grp)
{
   H5Gclose (grp);
}

hid_t H5_Base_File::openDataset (const char *dataname)
{
   return H5Dopen (hFileId, dataname, H5P_DEFAULT);
}

hid_t H5_Base_File::openDataset (const char *dataname, hid_t locId)
{
   return H5Dopen (locId, dataname, H5P_DEFAULT);
}

void H5_Base_File::closeDataset (hid_t dset)
{
   H5Dclose (dset);
}

bool H5_Base_File::getDimensions (hid_t datasetId, SpaceDimensions &dims)
{
   // read space dimensions from dataset and assign to Dimension object
   hid_t spaceId = H5Dget_space (datasetId);
   if ( spaceId > -1 )
   {
      dims.setNumDimensions (H5Sget_simple_extent_ndims (spaceId));
      H5Sget_simple_extent_dims (spaceId, dims.dimensions (), NULL);
      H5Sclose (spaceId);
      return true;
   }
   return false;
}

bool H5_Base_File::safeReadWrite (ReadWriteObject &fileOp, ostream &os)
{
   bool status = false;

   if ( fileOp.checkIdsOkay (os) )
   {
      status = fileOp ();
   }

   fileOp.cleanUp ();
   return status;
}

//
// H5_Write_File METHODS
//
hid_t H5_Write_File::addDataset (const char *dataname, hid_t type, 
                                 H5_FixedSpace& space, hid_t propertyList)
{
   return H5Dcreate (hFileId, dataname, type, space.space_id(), propertyList, H5P_DEFAULT, H5P_DEFAULT);
}

hid_t H5_Write_File::addDataset (const char *dataname, hid_t locId, hid_t type, 
                                 H5_FixedSpace& space, H5_FixedSpace * memspace, hid_t propertyList)
{
   int numDimensions = space.numDimensions();

   // do not output map data in chunks
   if( not m_useChunks or numDimensions < 3 or memspace == NULL ) {
      return addDataset (dataname, locId, type, space, propertyList);
   }
   
   hid_t datasetId = H5Dopen (locId, dataname, H5P_DEFAULT);
   if (datasetId >= 0)
   {
      H5Dclose (datasetId);
      H5Gunlink (locId, dataname);
   }
   hid_t dcpl = H5Pcreate(H5P_DATASET_CREATE);
 
   int rank = 0;
   MPI_Comm_rank ( MPI_COMM_WORLD, &rank );

   const hsize_t  * dims =  memspace->dims();
            
   hsize_t * chunk= new hsize_t[numDimensions];

   for( int i = 0; i < numDimensions; ++ i ) {
      chunk[i] =  dims[i];
   }

   // chunk size must be the same for all ranks
   MPI_Allreduce ( (void *)dims, chunk, numDimensions, MPI_UNSIGNED_LONG_LONG, MPI_MIN, MPI_COMM_WORLD);

   // H5Pset_cache //chunking cache size. Works only for reading

   // H5Pset_layout(dcpl, H5D_CHUNKED);
   
   // hsize_t totsize = 1;
   // for( int i = 0; i < numDimensions; ++ i ) {
   //    totsize *= chunk[i];
   // }
   // hsize_t cachesize = totsize * sizeof( int );
   // H5Pset_chunk_cache (dcpl, totsize, cachesize, 1 );
   
   H5Pset_chunk(dcpl, numDimensions, chunk);

   delete []chunk;


   datasetId = H5Dcreate (locId, dataname, type, space.space_id(), propertyList, dcpl, H5P_DEFAULT);

   H5Pclose(dcpl);

   if (datasetId < 0)
   {
      
      H5Eprint ( H5E_DEFAULT, 0 );
      
      cerr << "creating dataset " << dataname <<  " failed, with error code: " << datasetId << endl;
   }
   
   return datasetId;
}


hid_t H5_Write_File::addDataset (const char *dataname, hid_t locId, hid_t type, 
                                 H5_FixedSpace& space, hid_t propertyList)
{
   hid_t datasetId = H5Dopen (locId, dataname, H5P_DEFAULT);
   if (datasetId >= 0)
   {
      H5Dclose (datasetId);
      H5Gunlink (locId, dataname);
   }
   datasetId = H5Dcreate (locId, dataname, type, space.space_id(), propertyList, H5P_DEFAULT, H5P_DEFAULT);

   if (datasetId < 0)
   {

      H5Eprint ( H5E_DEFAULT, 0 );

      cerr << "creating dataset " << dataname <<  " failed, with error code: " << datasetId << endl;
   }

   return datasetId;
}

hid_t H5_Write_File::addGroup (const char *groupname)
{
   return H5Gcreate (hFileId, groupname, 0, H5P_DEFAULT, H5P_DEFAULT);
}

hid_t H5_Write_File::addGroup (const char *groupname, hid_t locId)
{
   return H5Gcreate (locId, groupname, 0, H5P_DEFAULT, H5P_DEFAULT);
}

hid_t H5_Write_File::addAttribute (const char *attributeName, hid_t locId, hid_t type, 
                                    H5_FixedSpace &space)
{
   return H5Acreate (locId, attributeName, type, space.space_id(), H5P_DEFAULT, H5P_DEFAULT);
}

bool H5_Write_File::writeDataset (hid_t dataId, const void *buffer, H5_PropertyList *pList,
                                  hid_t fileSpace, hid_t memSpace)
{
   // create write object 
   WriteObject write (dataId, fileSpace, memSpace, 
                      createDatasetPropertyList (pList),
                      const_cast<void*>(buffer));

   // call safe readWrite
   return safeReadWrite (write, cout); 
}

bool H5_Write_File::writeAttribute (hid_t attributeId, hid_t spaceId, void *buffer, 
                                   H5_PropertyList *)
{
   herr_t status = -1;

   if ( attributeId > -1 && buffer )
   {
      hid_t type = H5Aget_type (attributeId);
      status = H5Awrite (attributeId, type, buffer);
      H5Sclose (spaceId);
      H5Aclose (attributeId);
   }   

   return status != -1;
}

//
// New File Methods
//
void H5_New_File::openInMode (const char *filename)
{
   // create new file 
   hFileId = H5Fcreate (filename, H5F_ACC_TRUNC, H5P_DEFAULT, hPropertyListId);
}

//
// Append File Methods
//
void H5_Append_File::openInMode (const char *filename)
{
   // open file for appending
   hFileId = H5Fopen (filename, H5F_ACC_RDWR, hPropertyListId);
}

void H5_Unique_File::openInMode (const char *filename)
{
  // open only if file doesn't already exist
  hFileId = H5Fcreate (filename, H5F_ACC_EXCL, H5P_DEFAULT, hPropertyListId);
}

//
// Read only File Methods
//

hid_t H5_ReadOnly_File::createPropertyList (H5_PropertyList *userPropertyType )
{
   // use user-specified property type or default (serial)
   hPropertyListType = (userPropertyType ? userPropertyType->clone () 
                                         : hSerialPropertyType.clone ()); 

   return hPropertyListType->createFilePropertyList ( true );
}

hid_t H5_ReadOnly_File::createDatasetPropertyList (H5_PropertyList *propertyType )
{
   return (propertyType ? propertyType->createDatasetPropertyList ( true )
                        : hPropertyListType->createDatasetPropertyList ( true ));
}

void H5_ReadOnly_File::openInMode (const char *filename)
{
   // open file for reading
   hFileId = H5Fopen (filename, H5F_ACC_RDONLY, hPropertyListId);
}

hid_t H5_ReadOnly_File::openAttribute (const char *attributeName, hid_t locId)
{
   return H5Aopen_name (locId, attributeName);
}

bool H5_ReadOnly_File::readDataset (hid_t dataId, void *buffer, H5_PropertyList *pList,
                                    hid_t fileSpace, hid_t memSpace)
{
   // create read object
   ReadObject read (dataId, fileSpace, memSpace, 
                    createDatasetPropertyList ( pList ), 
                    buffer);

   // call safe readWrite
   return safeReadWrite (read, cout); 
}

bool H5_ReadOnly_File::readAttribute (hid_t attributeId, hid_t spaceId, void *buffer,  
                                       H5_PropertyList *)
{
   herr_t status = -1;

   if ( attributeId > -1 && buffer )
   {	
      hid_t type = H5Aget_type (attributeId);
      hid_t nativetype = type;
      H5_Type::convertToNativeType (nativetype);
      status = H5Aread (attributeId, nativetype, buffer);
      H5Sclose (spaceId);
      H5Aclose (attributeId);
   }

   return status != -1;
}

//
// Read/Write File Methods
//
void H5_ReadWrite_File::openInMode (const char *filename)
{
   // open file for reading and writing
   hFileId = H5Fopen (filename, H5F_ACC_RDWR, hPropertyListId);
}