#ifndef _DATA_MODEL__ABSTRACT_GRID_H_
#define _DATA_MODEL__ABSTRACT_GRID_H_

namespace DataModel {

   class AbstractGrid {

   public :

      virtual ~AbstractGrid () {}

      /// return the first local horizontal index
      virtual int firstI (void) const = 0;
      virtual int firstI (bool withGhosts) const = 0;

      /// return the first local vertical index
      virtual int firstJ (void) const = 0;
      virtual int firstJ (bool withGhosts) const = 0;

      /// return the last local horizontal index
      virtual int lastI (void) const = 0;
      virtual int lastI (bool withGhosts) const = 0;

      /// return the last local vertical index
      virtual int lastJ (void) const = 0;
      virtual int lastJ (bool withGhosts) const = 0;

   };

} // namespace DataModel

#endif // _DATA_MODEL__ABSTRACT_GRID_H_
