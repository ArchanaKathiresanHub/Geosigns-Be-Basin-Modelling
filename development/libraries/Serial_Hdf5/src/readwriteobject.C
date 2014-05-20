#include "stdafx.h"
#include <iostream>

// 
// function object to handle read/write of H5 Files
// 

#include "readwriteobject.h"
using namespace std;

#ifdef linux
extern int IBS_Use_ADIOI_Locking;
extern bool IBS_SerializeIO;
#endif
//
// Read/Write Methods
//
bool ReadWriteObject::checkIdsOkay (ostream& os)
{
   if ( rDataId         < 0 || 
        rFileSpaceId    < 0 || 
        rMemSpaceId     < 0 ||
        rPropertyListId < 0 ||
        rBuffer == NULL )
   {
      os << endl << "ReadWriteObject::checkIdsOkay"
         << endl
         << endl << "   Negative Id or Null Buffer:"
         << endl
         << endl << "Data Id: "          << rDataId
         << endl << "File Space Id: "    << rFileSpaceId
         << endl << "Mem Space Id: "     << rMemSpaceId
         << endl << "Property List Id: " << rPropertyListId
         << endl << "Buffer Addr: "      << rBuffer
         << endl;

      return false;
   }
   else
   {
      return true;
   }
}

void ReadWriteObject::cleanUp (void)
{
  if ( rFileSpaceId    > 0 ) H5Sclose (rFileSpaceId);
  if ( rMemSpaceId     > 0 ) H5Sclose (rMemSpaceId);
  if ( rDataId         > 0 ) H5Dclose (rDataId);
  if ( rPropertyListId > 0 ) H5Pclose (rPropertyListId);
}

// protected methods
hid_t ReadWriteObject::openType (hid_t datasetId)
{
  hid_t type = H5Dget_type (datasetId);
  H5_Type::convertToNativeType (type);
  return type;
}

//
// Write Methods
//
bool  WriteObject::operator() (void)
{
   hid_t typeId = openType (rDataId);
#ifdef linux
   if (IBS_SerializeIO) IBS_Use_ADIOI_Locking = false;
#endif
   herr_t status = H5Dwrite (rDataId, typeId, rMemSpaceId, rFileSpaceId, 
                             rPropertyListId, rBuffer);
#ifdef linux
   if (IBS_SerializeIO) IBS_Use_ADIOI_Locking = true;
#endif
   closeType (typeId);
   return status > -1;
}

//
// Read Methods
//
bool ReadObject::operator() (void)
{
   hid_t typeId = openType (rDataId);
   herr_t status = H5Dread (rDataId, typeId, rMemSpaceId, rFileSpaceId,
                            rPropertyListId, rBuffer);
   closeType (typeId);
   return status > -1;
}
