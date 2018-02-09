//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef DATA_MODEL__ABSTRACT_SNAPSHOT_H
#define DATA_MODEL__ABSTRACT_SNAPSHOT_H

#include <set>

namespace DataModel {

   /// \brief The abstract snapshot
   class AbstractSnapshot {

   public :

      AbstractSnapshot () = default;

      virtual ~AbstractSnapshot () = default;

      /// \brief Get the age at which the snapshot is defined
      virtual double getTime () const = 0;

      /// \brief Utility to compare snapshot pointers
      /// \details Pointers can be raw or smart
      template <typename T>
      struct ComparePointers {
         bool operator() (const T lhs, const T rhs) const {
            if(lhs==nullptr or rhs==nullptr) return false;
            return lhs->getTime()<rhs->getTime();
         }
      };

   };

   /// \brief A set of snapshot pointers
   typedef std::set<const AbstractSnapshot*> AbstractSnapshotSet;

} // namespace DataModel

#endif // DATA_MODEL__ABSTRACT_SNAPSHOT_H
