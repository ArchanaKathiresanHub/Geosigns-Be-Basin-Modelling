//
// Classes to read and write buffer data
// from a hd5f file
//

#include <string.h>
#include <iostream>

#include "buffer_readwrite.h"

using namespace std;

bool Buffer_ReadWrite::writeDataset (H5_Write_File *h5File, hid_t locId, 
                                     const char *dataset, const void *array, 
                                     BufferDimensions &dims, hid_t dataType)
{
   if ( ! array )
   {
      cout << endl << "Basin_Error: Buffer_ReadWrite::write Error: Empty Array" << endl;
      return false;
   }

   bool status = false;

   // create data space 
   H5_FixedSpace dataSpace (dims);
    
   // add dataset
   hid_t dataId = h5File->addDataset (dataset, locId, dataType, dataSpace);

   // write the array to dataset
   if ( dataId > -1 )
   {
      status = h5File->writeDataset (dataId, array);
   }
   
   return status;
}
     
bool Buffer_ReadWrite::writeAttribute (H5_Write_File *h5File, hid_t locId, const char *attribute, 
                                       void *data, BufferDimensions &dims, hid_t dataType)
{
   if ( ! data )
   {
      cout << endl << "Basin_Error: Buffer_ReadWrite::writeVoidAttribute Error: Empty Array" << endl;
      return false;
   }

   bool status = false;

   // create data space 
   H5_FixedSpace dataSpace (dims);
      
   // add attribute
   hid_t attId = h5File->addAttribute (attribute, locId, dataType, dataSpace);

   // write the array to dataset
   if ( attId > -1 )
   {
     status = h5File->writeAttribute (attId, dataSpace.space_id(), data);
   }
    
   return status;
}
