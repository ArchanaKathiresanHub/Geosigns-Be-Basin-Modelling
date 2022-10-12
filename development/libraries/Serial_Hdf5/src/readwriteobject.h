// 
// function object to handle read/write of H5 Files
// 

#ifndef __readwriteobject__
#define __readwriteobject__

#include "h5_types.h"
#include <iostream>


class ReadWriteObject
{
public:
   // ctor / dtor
   ReadWriteObject (hid_t dataId, hid_t fileSpaceId, hid_t memSpaceId,
                    hid_t propertyListId, void *buffer)
   : rDataId (dataId), rFileSpaceId (fileSpaceId), rMemSpaceId (memSpaceId),
     rPropertyListId (propertyListId)
   {
      rBuffer = buffer;
   }

   virtual ~ReadWriteObject () {}

   // function object
   virtual bool operator () (void) = 0;
 
   // public methods
   bool checkIdsOkay (std::ostream& os);
   void cleanUp      (void);

protected:
   hid_t openType  (hid_t datasetId); 
   void  closeType (hid_t typeId) { H5Tclose (typeId); }

   hid_t rDataId, rFileSpaceId, rMemSpaceId, rPropertyListId;
   void  *rBuffer;
};

class WriteObject : public ReadWriteObject
{
public:
   WriteObject (hid_t dataId, hid_t fileSpaceId, hid_t memSpaceId,
                hid_t propertyListId, void *buffer)
   : ReadWriteObject (dataId, fileSpaceId, memSpaceId, propertyListId, buffer)
   {}

   bool operator() (void);
};

class ReadObject : public ReadWriteObject
{
public:
   ReadObject (hid_t dataId, hid_t fileSpaceId, hid_t memSpaceId,
                   hid_t propertyListId, void *buffer)
   : ReadWriteObject (dataId, fileSpaceId, memSpaceId, propertyListId, buffer)
   {}
 
   bool operator() (void);
};

#endif
