//
// Classes to read and write buffer data
// from a hd5f file
//

#ifndef __buffer_readwrite__
#define __buffer_readwrite__

#include "DllExport.h"
#include "h5_file_types.h"
#include "buffer.h"

class SERIALHDF5_DLL_EXPORT Buffer_ReadWrite
{
public:
   // ctor / dtor
   virtual ~Buffer_ReadWrite () {}

   // public methods

   template<class Type>
   bool writeDataset (H5_Write_File *h5File, hid_t locId, const char *dataset, 
                      Buffer<Type>* out_buffer,
                      hid_t dataType, H5_PropertyList *pList = NULL)
   {
      // convert data to 1D for writing
      Type *array = out_buffer->get1dVersion ();

      // create data space and dataset
      H5_FixedSpace dataSpace (out_buffer->getDimensions ());
      hid_t dataId = h5File->addDataset (dataset, locId, dataType, dataSpace);

      // write the data to the dataset
      return h5File->writeDataset (dataId, array, pList); 
   }

   bool writeDataset (H5_Write_File *h5File, hid_t locId, const char *dataset, const void *array, 
                      BufferDimensions &dims, hid_t dataType, H5_PropertyList *pList = NULL);
   
   template<class Type>
   bool readDataset (H5_ReadOnly_File *h5File, hid_t locId, const char *dataset, 
                     Buffer<Type>* in_buffer, H5_PropertyList *pList = NULL)
   {
      // open the dataset
      hid_t dataId = h5File->openDataset (dataset, locId); 

      if ( dataId < 0 )
      {    
         return false;
      }

      // get the dimensions
      const H5_FixedSpace fileSpace (H5Dget_space (dataId));
      BufferDimensions dims (fileSpace.h5_dims());
      in_buffer->setDimensions (dims);

      // read data into temporary 1D buffer
      Type *buffer = new Type [in_buffer->linearSize()];
      bool status = h5File->readDataset (dataId, buffer, pList);
 
      // convert data from 1d array into buffer type
      in_buffer->saveAs1dVersion (buffer, dims);

      return status;
   }
   
   bool readDataset (H5_ReadOnly_File *h5File, hid_t locId, const char *dataset, 
                     BufferDimensions &dims, void **data, H5_PropertyList *pList = NULL);

   bool readDataset (H5_ReadOnly_File *h5File, hid_t locId, const char *dataset, 
                     BufferDimensions &dims, char **data, H5_PropertyList *pList = NULL);

   template<class Type>
   bool writeAttribute (H5_Write_File *h5File, hid_t locId, const char *attribute, 
                        Buffer<Type>* out_buffer, hid_t dataType, H5_PropertyList *pList = NULL)
   {
      hid_t status = false;

      // convert data to 1D for writing
      Type *array = out_buffer->get1dVersion ();

      // create data space and attribute
      H5_FixedSpace dataSpace (out_buffer->getDimensions ());
      hid_t attributeId = h5File->addAttribute (attribute, locId, dataType, dataSpace);

      // write the data to the dataset
      if ( attributeId > -1 )
      {
         status = h5File->writeAttribute (attributeId, dataSpace.space_id(), array, pList); 
      }

      return status;
   }

   bool writeAttribute (H5_Write_File *h5File, hid_t locId, const char *attribute, 
                        void *data, BufferDimensions &dims, hid_t dataType, 
                        H5_PropertyList *pList = NULL);

   template<class Type>
   bool readAttribute (H5_ReadOnly_File *h5File, hid_t locId, const char *attribute, 
                       Buffer<Type>* in_buffer, H5_PropertyList *pList = NULL)              
   {
      // open the attribute
      hid_t attributeId = h5File->openAttribute (attribute, locId); 

      if ( attributeId < 0 )
      {
         return false;
      }

      // get the dimensions
      const H5_FixedSpace fileSpace (H5Dget_space (attributeId));
      BufferDimensions dims (fileSpace.h5_dims());
      in_buffer->setDimensions (dims);

      // read data into temporary 1D buffer
      Type *buffer = new Type [in_buffer->linearSize()];
      bool status = h5File->readAttribute (attributeId, fileSpace.space_id(), buffer, pList);
 
      // convert data from 1d array into buffer type
      in_buffer->saveAs1dVersion (buffer, dims);

      return status;
   }

   bool readAttribute (H5_ReadOnly_File *h5File, hid_t locId, const char *attribute, 
                       BufferDimensions &dims, void **data, H5_PropertyList *pList = NULL); 

   bool readAttribute (H5_ReadOnly_File *h5File, hid_t locId, const char *attribute, 
                       BufferDimensions &dims, char **data, H5_PropertyList *pList = NULL);       
};

#endif
