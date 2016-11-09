//
// Classes to read and write buffer data
// from a hd5f file
//

#ifndef __buffer_readwrite__
#define __buffer_readwrite__

#include "h5_file_types.h"
#include "buffer.h"

class Buffer_ReadWrite
{
public:
   // ctor / dtor
   virtual ~Buffer_ReadWrite () {}

   // public methods

   bool writeDataset (H5_Write_File *h5File, hid_t locId, const char *dataset, const void *array, 
                      BufferDimensions &dims, hid_t dataType, H5_PropertyList *pList = NULL);
   
      
   bool readDataset (H5_ReadOnly_File *h5File, hid_t locId, const char *dataset, 
                     BufferDimensions &dims, void **data, H5_PropertyList *pList = NULL);

   bool readDataset (H5_ReadOnly_File *h5File, hid_t locId, const char *dataset, 
                     BufferDimensions &dims, char **data, H5_PropertyList *pList = NULL);


   bool writeAttribute (H5_Write_File *h5File, hid_t locId, const char *attribute, 
                        void *data, BufferDimensions &dims, hid_t dataType, 
                        H5_PropertyList *pList = NULL);

   bool readAttribute (H5_ReadOnly_File *h5File, hid_t locId, const char *attribute, 
                       BufferDimensions &dims, void **data, H5_PropertyList *pList = NULL); 

   bool readAttribute (H5_ReadOnly_File *h5File, hid_t locId, const char *attribute, 
                       BufferDimensions &dims, char **data, H5_PropertyList *pList = NULL);       
};

#endif
