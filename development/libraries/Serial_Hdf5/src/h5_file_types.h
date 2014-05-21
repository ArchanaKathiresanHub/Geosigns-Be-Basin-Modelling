//
// Classes with specific H5 file properties
//

#ifndef __h5_serial_types__
#define __h5_serial_types__

#include "readwriteobject.h"
#include "h5_types.h" 

typedef H5_Dimensions<hsize_t> SpaceDimensions;

class H5_PropertyList
{
public:   
   // ctor / dtor
   H5_PropertyList (void) {} 
   virtual ~H5_PropertyList (void) {}
   virtual H5_PropertyList* clone (void) const { return new H5_PropertyList (*this); } 

   // public methods

   // returns property list (default: serial)
   virtual hid_t createFilePropertyList (void) const
   { 
      // hope this works across the board
      return H5P_DEFAULT; 
   }

   // returns data property list
   virtual hid_t createDatasetPropertyList (void) const
   {
      return H5P_DEFAULT; 
   }
};

//
// Abstract base class for H5 Read/Write
//
class H5_Base_File
{
public:
   // ctor / dtor
   H5_Base_File () : hFileId ((hid_t)0) {}
   virtual ~H5_Base_File () {} 
   
   // public methods
   bool  open           (const char *filename, H5_PropertyList *propertyType = NULL);
   void  close          (void);
   bool  isOpen         (void) { return hFileId > 0; }
   hid_t fileId         (void) const { return hFileId; }
   hid_t propertyListId (void) const { return hPropertyListId; }
   hid_t openGroup      (const char *name);
   hid_t openGroup      (const char *name, hid_t locId);
   void  closeGroup     (hid_t grp);

   hid_t openDataset    (const char *dataname);
   hid_t openDataset    (const char *dataname, hid_t locId);
   void  closeDataset   (hid_t dset);
   void  closeDataspace (hid_t space) { H5Sclose (space); }
   bool  getDimensions  (hid_t datasetId, SpaceDimensions &dims);

   void  suppressErrors (void) { H5Eset_auto (NULL, NULL, NULL); }

protected:
   virtual void  openInMode (const char *filename) = 0;
   
   hid_t createPropertyList        (H5_PropertyList *propertyType);
   hid_t createDatasetPropertyList (H5_PropertyList *propertyType);

   bool safeReadWrite (ReadWriteObject &fileOp, ostream &os);
 
   hid_t hFileId;
   hid_t hPropertyListId; 
   const H5_PropertyList *hPropertyListType;
   const H5_PropertyList hSerialPropertyType;
 
private:
   // not allowing copy or assignment
   H5_Base_File& operator= (const H5_Base_File& rhs);
   H5_Base_File (const H5_Base_File& rhs);
};

//
// Class H5_Write inherits the Base class and adds functionality
// for writing to a file
//
class H5_Write_File : public virtual H5_Base_File
{
public:   
   H5_Write_File () : H5_Base_File () {}

   // public methods
   hid_t addDataset    (const char *dataname, hid_t type, H5_FixedSpace &space, 
                        hid_t propertyList = H5P_DEFAULT);
   hid_t addDataset    (const char *dataname, hid_t locId, hid_t type, H5_FixedSpace &space, 
                        hid_t propertyList = H5P_DEFAULT);

   hid_t addGroup      (const char *groupname);
   hid_t addGroup      (const char *groupname, hid_t locId);

   hid_t addAttribute  (const char *attributeName, hid_t locId, hid_t type, H5_FixedSpace &space);
   
   bool writeDataset   (hid_t dataId, const void *buffer, H5_PropertyList *pList = NULL,
                        hid_t fileSpace = H5S_ALL, hid_t memSpace = H5S_ALL);

   bool writeAttribute (hid_t attributeId, hid_t spaceId, void *buffer, 
                        H5_PropertyList *pList = NULL);
};

class H5_New_File : public H5_Write_File
{
public:
   // ctor / dtor
   H5_New_File () : H5_Write_File () {}

private:
   void openInMode (const char *filename);
};

class H5_Append_File : public H5_Write_File
{
public:
   // ctor / dtor
   H5_Append_File () : H5_Write_File () {}

private:
   void openInMode (const char *filename);
};

class H5_Unique_File : public H5_Write_File
{
public:
   // ctor / dtor
   H5_Unique_File () : H5_Write_File () {}

private:
   void openInMode (const char *filename);
};

//
// Read Only inherits from the base class and adds functions
// for reading from a file
//
class H5_ReadOnly_File : public virtual H5_Base_File
{
public:
   // ctor / dtor
   H5_ReadOnly_File () : H5_Base_File () {}

   // public methods   
   hid_t openAttribute (const char *attributeName, hid_t locId);

   bool readDataset    (hid_t dataId, void *buffer, H5_PropertyList *pList = NULL,
                        hid_t fileSpace = H5S_ALL, hid_t memSpace = H5S_ALL);

   bool readAttribute  (hid_t attributeId, hid_t spaceId, void *buffer,  
                        H5_PropertyList *pList = NULL);

private:
   void openInMode (const char *filename);
};

class H5_ReadWrite_File : public H5_Write_File, public H5_ReadOnly_File 
{
public:
   // ctor / dtor
   H5_ReadWrite_File () : H5_Write_File (), H5_ReadOnly_File () {}

private:
   void openInMode (const char *filename);
};

#endif
