#ifndef _INTERFACE_PALEOSURFACEPROPERTY_H_
#define _INTERFACE_PALEOSURFACEPROPERTY_H_

#include <string>

#include "Interface.h"
#include "PaleoProperty.h"

namespace DataAccess
{
   namespace Interface
   {

      /// Contains a property for a surface in paleo-time.
      class PaleoSurfaceProperty : public PaleoProperty
      {
      public:

         PaleoSurfaceProperty (ProjectHandle& projectHandle, database::Record * record, const Surface* surface );

         virtual ~PaleoSurfaceProperty (void);

         /// Return the Surface of this PaleoSurfaceProperty.
         virtual const Surface * getSurface (void) const;

         /// Return the Surface name of this PaleoSurfaceProperty.
         const std::string & getSurfaceName (void) const;

         void asString (string &) const;

         //void setSurface ( const Surface* surface );

      protected:

         const Surface * m_surface;
         const string surfaceName;

      };

   }
}

#endif // _INTERFACE_PALEOSURFACEPROPERTY_H_
