#ifndef _INTERFACE_BASEMENT_SURFACE_H_
#define _INTERFACE_BASEMENT_SURFACE_H_

#include "DAObject.h"
#include "Interface.h"
#include "Surface.h"

namespace DataAccess
{
   namespace Interface
   {
      /// A BasementSurface object extends a normal surface ...
      class BasementSurface : public Surface
      {
      public:
         BasementSurface (ProjectHandle * projectHandle, const std::string& surfaceName);
         ~BasementSurface (void);

         /// Return the name of this Surface
         const string & getName (void) const;

         /// Returns null, since this surface is not associated with any project file record, hence no depth map.
         GridMap * loadDepthMap (void) const;

         /// The depth of surface.
         GridMap * computeDepthMap ( const GridMap* operand, UnaryFunctor& op ) const;

         GridMap * computeDepthMap () const;

         virtual void asString (string & str) const;

      protected :

         string m_surfaceName;

      };
   }
}


#endif // _INTERFACE_BASEMENT_SURFACE_H_
