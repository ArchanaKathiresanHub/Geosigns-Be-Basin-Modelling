#include "stdafx.h"
//
// Classes with specific H5 file properties
//

#ifdef linux
int IBS_Use_ADIOI_Locking = 1;

bool IBS_SerializeIO = false;
#endif

#include "h5_file_types.h"

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
   suppressErrors ();

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

hid_t H5_Base_File::createPropertyList (H5_PropertyList *userPropertyType)
{
   // use user-specified property type or default (serial)
   hPropertyListType = (userPropertyType ? userPropertyType->clone () 
                                         : hSerialPropertyType.clone ()); 

   return hPropertyListType->createFilePropertyList ();
}

hid_t H5_Base_File::createDatasetPropertyList (H5_PropertyList *propertyType)
{
   return (propertyType ? propertyType->createDatasetPropertyList ()
                        : hPropertyListType->createDatasetPropertyList ());
}

hid_t H5_Base_File::openGroup (const char *name)
{
# if H5_VERS_MINOR == 6
   return H5Gopen (hFileId, name);
#else
   return H5Gopen (hFileId, name, NULL);
#endif
}

hid_t H5_Base_File::openGroup (const char *name, hid_t locId)
{
# if H5_VERS_MINOR == 6
   return H5Gopen (locId, name);
#else
   return H5Gopen (locId, name, NULL);
#endif
}

void H5_Base_File::closeGroup (hid_t grp)
{
   H5Gclose (grp);
}

hid_t H5_Base_File::openDataset (const char *dataname)
{
# if H5_VERS_MINOR == 6
   return H5Dopen (hFileId, dataname);
#else
   return H5Dopen (hFileId, dataname, NULL);
#endif
}

hid_t H5_Base_File::openDataset (const char *dataname, hid_t locId)
{
# if H5_VERS_MINOR == 6
   return H5Dopen (locId, dataname);
#else
   return H5Dopen (locId, dataname, NULL);
#endif
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
# if H5_VERS_MINOR == 6
   return H5Dcreate (hFileId, dataname, type, space.space_id(), propertyList);
#else
   return H5Dcreate (hFileId, dataname, type, space.space_id(), propertyList, H5P_DEFAULT, H5P_DEFAULT);
#endif
}

hid_t H5_Write_File::addDataset (const char *dataname, hid_t locId, hid_t type, 
                                 H5_FixedSpace& space, hid_t propertyList)
{
# if H5_VERS_MINOR == 6
   hid_t datasetId = H5Dopen (locId, dataname);
#else
   hid_t datasetId = H5Dopen (locId, dataname, NULL);
#endif
   if (datasetId >= 0)
   {
      H5Dclose (datasetId);
      H5Gunlink (locId, dataname);
   }
# if H5_VERS_MINOR == 6
   datasetId = H5Dcreate (locId, dataname, type, space.space_id(), propertyList);
#else
   datasetId = H5Dcreate (locId, dataname, type, space.space_id(), propertyList, H5P_DEFAULT, H5P_DEFAULT);
#endif

   if (datasetId < 0)
   {
      cerr << "creating dataset " << dataname <<  " failed" << endl;
   }

   return datasetId;
}

hid_t H5_Write_File::addGroup (const char *groupname)
{
# if H5_VERS_MINOR == 6
   return H5Gcreate (hFileId, groupname, 0);
#else
   return H5Gcreate (hFileId, groupname, 0, H5P_DEFAULT, H5P_DEFAULT);
#endif
}

hid_t H5_Write_File::addGroup (const char *groupname, hid_t locId)
{
# if H5_VERS_MINOR == 6
   return H5Gcreate (locId, groupname, 0);
#else
   return H5Gcreate (locId, groupname, 0, H5P_DEFAULT, H5P_DEFAULT);
#endif
}

hid_t H5_Write_File::addAttribute (const char *attributeName, hid_t locId, hid_t type, 
                                    H5_FixedSpace &space)
{
# if H5_VERS_MINOR == 6
   return H5Acreate (locId, attributeName, type, space.space_id(), H5P_DEFAULT);
#else
   return H5Acreate (locId, attributeName, type, space.space_id(), H5P_DEFAULT, NULL);
#endif
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
                                   H5_PropertyList *pList)
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
#ifdef linux
   if (IBS_SerializeIO) IBS_Use_ADIOI_Locking = false;
#endif
   hFileId = H5Fopen (filename, H5F_ACC_RDWR, hPropertyListId);
#ifdef linux
   if (IBS_SerializeIO) IBS_Use_ADIOI_Locking = true;
#endif
}

void H5_Unique_File::openInMode (const char *filename)
{
  // open only if file doesn't already exist
  hFileId = H5Fcreate (filename, H5F_ACC_EXCL, H5P_DEFAULT, hPropertyListId);
}

//
// Read only File Methods
//
void H5_ReadOnly_File::openInMode (const char *filename)
{
   // open file for reading
#ifdef linux
   if (IBS_SerializeIO) IBS_Use_ADIOI_Locking = false;
#endif
   hFileId = H5Fopen (filename, H5F_ACC_RDONLY, hPropertyListId);
#ifdef linux
   if (IBS_SerializeIO) IBS_Use_ADIOI_Locking = true;
#endif
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
                    createDatasetPropertyList (pList), 
                    buffer);

   // call safe readWrite
   return safeReadWrite (read, cout); 
}

bool H5_ReadOnly_File::readAttribute (hid_t attributeId, hid_t spaceId, void *buffer,  
                                       H5_PropertyList *pList)
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
#ifdef linux
   if (IBS_SerializeIO) IBS_Use_ADIOI_Locking = false;
#endif
   hFileId = H5Fopen (filename, H5F_ACC_RDWR, hPropertyListId);
#ifdef linux
   if (IBS_SerializeIO) IBS_Use_ADIOI_Locking = true;
#endif
}
