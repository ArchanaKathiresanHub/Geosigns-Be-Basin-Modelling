#ifndef _FASTCAULDRON__NODAL_GRID__H_
#define _FASTCAULDRON__NODAL_GRID__H_

#include <vector>

#include "petsc.h"
#include "petscdmda.h"

#include "Grid.h"

#include "NumericFunctions.h"


/// Class describing processor-local regular 2d-nodal-grids.
class NodalGrid {

public :

   NodalGrid ();

   ~NodalGrid ();

   /// Initialise the nodal-grid.
   ///
   /// Parameters are the map node-grid and the process rank.
   void construct ( const DataAccess::Interface::Grid* nodeGrid,
                    const int                          rank );


   /// Get the total number of nodes in the x-direction.
   int getNumberOfXNodes () const;

   /// Get the number of processors in the x-direction.
   int getNumberOfXProcessors () const;

   /// Get the partitioning information in the x-direction.
   int* getXPartitioning () const;

   /// Get the total number of nodes in the y-direction.
   int getNumberOfYNodes () const;

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

   /// Get the local y-start position. 
   int firstJ ( const bool includeGhosts = false ) const;

   /// Get the local y-end position. 
   ///
   /// This is a closed interval.
   int lastJ ( const bool includeGhosts = false ) const;

   /// \brief Indicate whether or not the position (i, j) is part of the valid set of indices in the node-grid.
   ///
   /// The node grid is based on the petsc box-stencil.
   bool isPartOfStencil ( const int i,
                          const int j,
                          const bool includeGhosts = false ) const;

   /// The DA object on which the nodal-grid is based.
   DM getDa ();

   /// Return whether or not the object has been initialised.
   bool isInitialised () const;

protected :

   // make class non-copyable
   NodalGrid ( const NodalGrid& );

   void operator= ( const NodalGrid& );

   DMDALocalInfo m_localInfo;

   int* m_xPartitioning;
   int  m_numberOfXProcessors;

   int* m_yPartitioning;
   int  m_numberOfYProcessors;


};

/// \typedef NodalGridArray
/// \brief Array of pointers to NodalGrid.
typedef std::vector<NodalGrid*> NodalGridArray;


//------------------------------------------------------------//
// Inline functions
//------------------------------------------------------------//

inline DM NodalGrid::getDa () {
   return m_localInfo.da;
}

inline int NodalGrid::getNumberOfXNodes () const {
   return m_localInfo.mx;
}

inline int NodalGrid::getNumberOfXProcessors () const {
   return m_numberOfXProcessors;
} 

inline int* NodalGrid::getXPartitioning () const {
   return m_xPartitioning;
}

inline int NodalGrid::getNumberOfYNodes () const {
   return m_localInfo.my;
}

inline int NodalGrid::getNumberOfYProcessors () const {
   return m_numberOfYProcessors;
} 

inline int* NodalGrid::getYPartitioning () const {
   return m_yPartitioning;
}

inline int NodalGrid::firstI ( const bool includeGhosts ) const {
   return ( includeGhosts ? m_localInfo.gxs : m_localInfo.xs );
}

inline int NodalGrid::lastI ( const bool includeGhosts ) const {
   return ( includeGhosts ? m_localInfo.gxs + m_localInfo.gxm - 1 : m_localInfo.xs + m_localInfo.xm - 1 );
}

inline int NodalGrid::firstJ ( const bool includeGhosts ) const {
   return ( includeGhosts ? m_localInfo.gys : m_localInfo.ys );
}

inline int NodalGrid::lastJ ( const bool includeGhosts ) const {
   return ( includeGhosts ? m_localInfo.gys + m_localInfo.gym - 1 : m_localInfo.ys + m_localInfo.ym - 1 );
}

inline bool NodalGrid::isPartOfStencil ( const int i,
                                         const int j,
                                         const bool includeGhosts ) const {

   return NumericFunctions::inRange<int>( i, firstI ( includeGhosts ), lastI ( includeGhosts )) and
          NumericFunctions::inRange<int>( j, firstJ ( includeGhosts ), lastJ ( includeGhosts ));


}


inline bool NodalGrid::isInitialised () const {
   // This test could be applied to the y-partitioning array.
   return m_xPartitioning != 0;
}

#endif // _FASTCAULDRON__NODAL_GRID__H_

