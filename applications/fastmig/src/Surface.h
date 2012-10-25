#ifndef _MIGRATION_SURFACE_H_
#define _MIGRATION_SURFACE_H_

#include <iostream>

#include <string>
#include <map>
#include <vector>

using namespace std;

namespace database
{
   class Record;
   class Table;
}

#include "Interface/Surface.h"

namespace DataAccess
{
   namespace Interface
   {
   }
}

using namespace DataAccess;

#include "RequestDefs.h"

/// Surface Class
namespace migration
{

   /// This class implements the Surface-specific migration functionality.
   /// It is constructed on top of the DataAccess::formation class.
   class Surface : public Interface::Surface
   {
      public:
	 /// This constructor is called by the object factory
	 Surface (Interface::ProjectHandle * projectHandle, database::Record * record);

	 /// Destructor
	 virtual ~Surface (void);

	 const Surface * getTopSurface () const;
	 const Surface * getBottomSurface () const;
   };
}

#endif // _MIGRATION_SURFACE_H_

