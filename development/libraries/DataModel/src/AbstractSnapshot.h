#ifndef _DATA_MODEL__ABSTRACT_SNAPSHOT_H_
#define _DATA_MODEL__ABSTRACT_SNAPSHOT_H_

namespace DataModel {

   class AbstractSnapshot {

   public :

      virtual ~AbstractSnapshot () {}

      virtual double getTime () const = 0;

   };

} // namespace DataModel

#endif // _DATA_MODEL__ABSTRACT_SNAPSHOT_H_
