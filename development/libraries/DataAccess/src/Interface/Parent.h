// 
// Copyright (C) 2015-2017 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef INTERFACE_PARENT_H
#define INTERFACE_PARENT_H

#include "DynArray.h"
namespace DataAccess
{
   namespace Interface
   {
      class Child;

      /// @class Parent A Parent object can have multiple Child objects for whose destruction it is responsible
      /// when it is destroyed itself.
      /// Its Child objects are individually accessible.
      class Parent
      {
      public:
         Parent (void);
         Parent (Child * child, unsigned index = 0);
         virtual ~Parent (void);
      
         /// @brief Set the child at the specified array index
         virtual void setChild (Child * child, unsigned int index = 0) const;
         /// @brief Disconnect yourself from the Child at the specified index.
         virtual void detachChild (unsigned int index = 0) const;
         /// @brief Delete the child at the specified index
         virtual void releaseChild (unsigned int index = 0) const;
         /// @brief Return the child at the specified index
         virtual Child * getChild (unsigned int index = 0) const;
      
      protected:
         mutable DynArray<Child *> * m_children; ///< Storage for its Child objects
      };
   }
}

#endif // INTERFACE_PARENT_H
