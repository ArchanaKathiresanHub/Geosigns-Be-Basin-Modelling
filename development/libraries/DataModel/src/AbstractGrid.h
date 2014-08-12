#ifndef _DATA_MODEL__ABSTRACT_GRID_H_
#define _DATA_MODEL__ABSTRACT_GRID_H_

namespace DataModel {

   /// \brief Describes a regular grid structure.
   ///
   /// This global grid structure may be defined across multiple processors
   /// an object of this type defines a smaller sub-structre.
   class AbstractGrid {

   public :

      virtual ~AbstractGrid () {}

      /// return the first local horizontal index
      virtual int firstI (bool includeGhosts) const = 0;

      /// return the first local vertical index
      virtual int firstJ (bool includeGhosts) const = 0;

      /// return the last local horizontal index
      virtual int lastI (bool includeGhosts) const = 0;

      /// return the last local vertical index
      virtual int lastJ (bool includeGhosts) const = 0;

   };

} // namespace DataModel

#endif // _DATA_MODEL__ABSTRACT_GRID_H_
