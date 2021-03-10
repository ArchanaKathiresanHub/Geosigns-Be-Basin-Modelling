#ifndef _FASTCAULDRON__ELEMENT_GRID__H_
#define _FASTCAULDRON__ELEMENT_GRID__H_

#include "petsc.h"
#include "petscdmda.h"

#include "Grid.h"


/// Class describing processor-local regular 2d-element grids.
class ElementGrid {

public :

   ElementGrid ();

   ~ElementGrid ();

   /// Initialise the element-grid.
   ///
   /// Parameters are the map node-grid and the process rank.
   void construct ( const DataAccess::Interface::Grid* nodeGrid,
                    const int                          rank );


   /// Get the total number of elements in the x-direction.
   int getNumberOfXElements () const;

   /// Get the number of processors in the x-direction.
   int getNumberOfXProcessors () const;

   /// Get the partitioning information in the x-direction.
   int* getXPartitioning () const;

   /// Get the total number of elements in the y-direction.
   int getNumberOfYElements () const;

   /// Get the number of processors in the y-direction.
   int getNumberOfYProcessors () const;

   /// Get the partitioning information in the y-direction.
   int* getYPartitioning () const;

   /// Get the local x-start position. 
   int firstI ( const bool includeGhosts = false ) const;

   /// Get the local x-end position. 
   ///
   /// This is a closed interval.
   int lastI ( const bool includeGhosts = false ) const;

   /// \brief Get the number of elements in the x-dimension.
   int lengthI ( const bool includeGhosts = false ) const;

   /// Get the local y-start position. 
   int firstJ ( const bool includeGhosts = false ) const;

   /// Get the local y-end position. 
   ///
   /// This is a closed interval.
   int lastJ ( const bool includeGhosts = false ) const;

   /// \brief Get the number of elements in the y-dimension.
   int lengthJ ( const bool includeGhosts = false ) const;

   /// \brief Indicate whether or not the position (i, j) is part of the valid set of indices in the element-grid.
   ///
   /// The element grid is based on the petsc star-stencil, so the corners of the grid may not be valid indices.
   bool isPartOfStencil ( const int i,
                          const int j ) const;

   /// The DA object on which the element-grid is based.
   DM getDa ();

   /// Return whether or not the object has been initialised.
   bool isInitialised () const;

protected :

   // make class non-copyable
   ElementGrid ( const ElementGrid& grid );

   void operator=( const ElementGrid& grid );


   DMDALocalInfo m_localInfo;

   int* m_xPartitioning;
   int  m_numberOfXProcessors;

   int* m_yPartitioning;
   int  m_numberOfYProcessors;


};

//------------------------------------------------------------//
// Inline functions
//------------------------------------------------------------//

inline DM ElementGrid::getDa () {
   return m_localInfo.da;
}

inline int ElementGrid::getNumberOfXElements () const {
   return m_localInfo.mx;
}

inline int ElementGrid::getNumberOfXProcessors () const {
   return m_numberOfXProcessors;
} 

inline int* ElementGrid::getXPartitioning () const {
   return m_xPartitioning;
}

inline int ElementGrid::getNumberOfYElements () const {
   return m_localInfo.my;
}

inline int ElementGrid::getNumberOfYProcessors () const {
   return m_numberOfYProcessors;
} 

inline int* ElementGrid::getYPartitioning () const {
   return m_yPartitioning;
}

inline int ElementGrid::firstI ( const bool includeGhosts ) const {
   return ( includeGhosts ? m_localInfo.gxs : m_localInfo.xs );
}

inline int ElementGrid::lastI ( const bool includeGhosts ) const {
   return ( includeGhosts ? m_localInfo.gxs + m_localInfo.gxm - 1 : m_localInfo.xs + m_localInfo.xm - 1 );
}

inline int ElementGrid::lengthI ( const bool includeGhosts ) const {
   return lastI ( includeGhosts ) - firstI ( includeGhosts ) + 1;
}

inline int ElementGrid::firstJ ( const bool includeGhosts ) const {
   return ( includeGhosts ? m_localInfo.gys : m_localInfo.ys );
}

inline int ElementGrid::lastJ ( const bool includeGhosts ) const {
   return ( includeGhosts ? m_localInfo.gys + m_localInfo.gym - 1 : m_localInfo.ys + m_localInfo.ym - 1 );
}

inline int ElementGrid::lengthJ ( const bool includeGhosts ) const {
   return lastJ ( includeGhosts ) - firstJ ( includeGhosts ) + 1;
}

inline bool ElementGrid::isInitialised () const {
   // This test could be applied to the y-partitioning array.
   return m_xPartitioning != 0;
}


#endif // _FASTCAULDRON__ELEMENT_GRID__H_
