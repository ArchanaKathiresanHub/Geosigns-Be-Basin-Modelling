#ifndef _INTERFACE_SURFACE_H_
#define _INTERFACE_SURFACE_H_


#include "Interface/Surface.h"

#include "Interface/DAObject.h"
#include "Interface.h"

namespace DataAccess
{
   namespace Interface
   {
      /// A Surface object forms the upper and/or lower boundary of a Formation.
      /// It also has a geological event time associated to it in the form of a Snapshot
      class Surface : public DAObject
      {
	 public:
	    Surface (ProjectHandle * projectHandle, database::Record * record);
	    virtual ~Surface (void);

	    /// Return the name of this Surface
	    virtual const string & getName (void) const;

	    /// Return the mangled name of this Surface
	    virtual const string & getMangledName (void) const;

	    /// Return the name of the Formation found above this Surface if there is one.
	    virtual const string & getTopFormationName (void);
	    /// Return the name of the Formation found below this Surface if there is one.
	    virtual const string & getBottomFormationName (void);

	    /// Return the Snapshot containing the deposition age of this Surface
	    virtual const Snapshot * getSnapshot (void) const;

	    /// Return the present day, user-supplied Surface depth GridMap.
	    virtual const GridMap * getInputDepthMap (void) const;

	    /// Return the Formation found above this Surface if there is one.
	    virtual const Formation * getTopFormation (void) const;
	    /// Return the Formation found below this Surface if there is one.
	    virtual const Formation * getBottomFormation (void) const;

	    void setTopFormation (Formation * formation);
	    void setBottomFormation (Formation * formation);

	    virtual GridMap * loadDepthMap (void) const;
	    virtual GridMap * computeDepthMap (void) const;

            virtual SurfaceKind kind () const;

	    virtual void printOn (ostream &) const;

	    virtual void asString (string &) const;

            /// \brief Get the deposition sequence of the formation directly BELOW the surface.
            /// 
            /// This function is required because it may be called before the formations have been set.
            /// If the formation below is part of the basement a scalar null value will be returned.
            int getFormationDepoSequenceNumber () const;

	 protected :

            /// The record is null.
            Surface (ProjectHandle * projectHandle);

	    string m_mangledName;

            static const int DepthMap = 0;

	    const Formation * m_top;
	    const Formation * m_bottom;

	    const Snapshot * m_snapshot;

            SurfaceKind m_kind;
            int m_formationDepositionSequenceNumber;

      };

      /// \brief Functor class for ordering surfaces.
      class SurfaceLessThan {

      public :

         bool operator ()( const Surface* s1,
                           const Surface* s2 ) const;

      };


   }
}


#endif // _INTERFACE_SURFACE_H_
