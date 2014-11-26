#ifndef _DATA_MODEL__ABSTRACT_SNAPSHOT_H_
#define _DATA_MODEL__ABSTRACT_SNAPSHOT_H_

#include <set>

namespace DataModel {

   /// \brief The age at which.
   class AbstractSnapshot {

   public :

      virtual ~AbstractSnapshot () {}

      /// \brief Get the age at which the snapshot is defined.
      virtual double getTime () const = 0;

   };

   /// \brief A set of snapshot pointers.
   typedef std::set<const AbstractSnapshot*> AbstractSnapshotSet;

} // namespace DataModel

#endif // _DATA_MODEL__ABSTRACT_SNAPSHOT_H_
