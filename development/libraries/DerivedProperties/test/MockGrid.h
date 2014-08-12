#ifndef _DERIVED_PROPERTIES__MOCK_GRID_H_
#define _DERIVED_PROPERTIES__MOCK_GRID_H_

#include "AbstractGrid.h"

namespace DataModel {

   /// \brief Describes a regular grid structure.
   ///
   /// This global grid structure may be defined across multiple processors
   /// an object of this type defines a smaller sub-structre.
   class MockGrid : public AbstractGrid {

   public :

      MockGrid ( const int startI, const int ghostStartI,
                 const int startJ, const int ghostStartJ,
                 const int endI,   const int ghostEndI,
                 const int endJ,   const int ghostEndJ );

      /// return the first local horizontal index
      virtual int firstI ( const bool includeGhosts ) const;

      /// return the first local vertical index
      virtual int firstJ ( const bool includeGhosts ) const;

      /// return the last local horizontal index
      virtual int lastI ( const bool includeGhosts ) const;

      /// return the last local vertical index
      virtual int lastJ ( const bool includeGhosts ) const;

   private :

      /// \brief The non-ghost index is in position 0 and the ghost index is in position 1.
      int m_firstI[2];
      int m_lastI[2];

      int m_firstJ[2];
      int m_lastJ[2];

   };

} // namespace DataModel

inline DataModel::MockGrid::MockGrid ( const int startI, const int ghostStartI,
                                       const int startJ, const int ghostStartJ,
                                       const int endI,   const int ghostEndI,
                                       const int endJ,   const int ghostEndJ ) {

   m_firstI [ 0 ] = startI;
   m_firstI [ 1 ] = ghostStartI;

   m_firstJ [ 0 ] = startJ;
   m_firstJ [ 1 ] = ghostStartJ;

   m_lastI [ 0 ] = endI;
   m_lastI [ 1 ] = ghostEndI;

   m_lastJ [ 0 ] = endJ;
   m_lastJ [ 1 ] = ghostEndJ;
}

inline int DataModel::MockGrid::firstI ( const bool includeGhosts ) const {
   return m_firstI [(includeGhosts ? 1 : 0 )];
}

inline int DataModel::MockGrid::firstJ ( const bool includeGhosts ) const {
   return m_firstJ [(includeGhosts ? 1 : 0 )];
}

inline int DataModel::MockGrid::lastI ( const bool includeGhosts ) const {
   return m_lastI [(includeGhosts ? 1 : 0 )];
}

inline int DataModel::MockGrid::lastJ ( const bool includeGhosts ) const {
   return m_lastJ [(includeGhosts ? 1 : 0 )];
}



#endif // _DERIVED_PROPERTIES__MOCK_GRID_H_
