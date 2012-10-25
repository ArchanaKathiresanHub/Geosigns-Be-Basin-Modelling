#ifndef _INTERFACE_DAOBJECT_H_
#define _INTERFACE_DAOBJECT_H_

namespace database
{
   class Database;
   class Table;
   class Record;
}

#include "Interface/Parent.h"
#include "Interface/AttributeValue.h"

#include <string>
using namespace std;

namespace DataAccess
{
   namespace Interface
   {

      class ProjectHandle;
      class ObjectFactory;


      /// Classes whose objects need to refer to a TableIO record to get at their data, inherit from this class
      /// as this class contains the code to do so.
      /// An object of this class can also refer back to the ProjectHandle that owns it.
      class DAObject : public Parent
      {
	 public:
	    /// constructor
	    DAObject (ProjectHandle * projectHandle, database::Record * record);
	    /// destructor
	    virtual ~DAObject (void);

	    /// return the record
	    database::Record * getRecord (void) const;
	    /// set the record
	    void setRecord (database::Record * record);

	    /// return the project handle
	    ProjectHandle * getProjectHandle (void) const;

	    /// return the object factory
	    ObjectFactory * getFactory (void) const;

	 protected:
	    ProjectHandle * m_projectHandle;
	    mutable database::Record * m_record;

	    AttributeValue & getAttributeValue (const string & attributeName, unsigned int indexOffset) const;
	    AttributeType getAttributeType (const string & attributeName) const;
	    int getAttributeIndex (const string & attributeName) const;
      };
   }
}


#endif // _INTERFACE_DAOBJECT_H_
