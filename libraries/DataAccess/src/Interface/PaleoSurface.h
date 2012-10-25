#ifndef _IMPLEMENTATION_PALEO_SURFACE_H_
#define _IMPLEMENTATION_PALEO_SURFACE_H_


#ifndef USEINTERFACE
#define INTERFACE Implementation
#define IMPLEMENTATION Implementation
#else
#define INTERFACE Interface
#define IMPLEMENTATION Implementation
#include "../Interface/PaleoSurface.h"
#endif

#include "../Implementation/DAObject.h"
#include "../Implementation/Implementation.h"
#include "../Implementation/Surface.h"

namespace DataAccess
{
   namespace IMPLEMENTATION
   {
      /// A Surface object forms the upper and/or lower boundary of a Formation.
      /// It also has a geological event time associated to it in the form of a Snapshot
#ifndef USEINTERFACE
      class PaleoSurface : public IMPLEMENTATION::Surface
#else
      class Surface : public INTERFACE::PaleoSurface, public IMPLEMENTATION::Surface
#endif
      {
	 public:
	    PaleoSurface (ProjectHandle * projectHandle, database::Record * record);

	    virtual ~PaleoSurface (void);


            virtual INTERFACE::PaleoPropertyList * getPaleoDepths () const;

         

         const INTERFACE::Snapshot* getSnapsthot () const;


	    /// Return the present day, user-supplied Surface depth GridMap.
	    virtual const INTERFACE::GridMap * getInputDepthMap (void) const;

	    /// Return the Formation found above this Surface if there is one.
	    virtual const INTERFACE::Formation * getTopFormation (void) const;
	    /// Return the Formation found below this Surface if there is one.
	    virtual const INTERFACE::Formation * getBottomFormation (void) const;

	    void setTopFormation (Formation * formation);
	    void setBottomFormation (Formation * formation);

	    GridMap * loadDepthMap (void) const;
	    GridMap * computeDepthMap (void) const;

	    virtual void printOn (ostream &) const;

	    virtual void asString (string &) const;

	 private:

            static const int DepthMap = 0;

      };
   }
}


#endif // _IMPLEMENTATION_PALEO_SURFACE_H_
