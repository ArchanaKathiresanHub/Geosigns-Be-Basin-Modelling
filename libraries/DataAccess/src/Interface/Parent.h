#ifndef _INTERFACE_PARENT_H_
#define _INTERFACE_PARENT_H_

#include "DynArray.h"
namespace DataAccess
{
   namespace Interface
   {
      class Child;

      /// A Parent object can have multiple Child objects for whose destruction it is responsible
      /// when it is destroyed itself.
      /// Its Child objects are individually accessible.
      class Parent
      {
	 public:
	    Parent (void);
	    Parent (Child * child, unsigned index = 0);
	    virtual ~Parent (void);

            /// set the child at the specified array index
	    virtual void setChild (Child * child, unsigned int index = 0) const;
            /// Disconnect yourself from the Child at the specified index.
	    virtual void detachChild (unsigned int index = 0) const;
            /// delete the child at the specified index
	    virtual void releaseChild (unsigned int index = 0) const;
            /// return the child at the specified index
	    virtual Child * getChild (unsigned int index = 0) const;

	 protected:
            /// Storage for its Child objects
	    mutable DynArray<Child *> * m_children;
      };
   }
}

#endif // _INTERFACE_PARENT_H_
