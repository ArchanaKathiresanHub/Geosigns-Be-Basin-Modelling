#ifndef _FASTCAULDRON__NODAL_VOLUME_GRID__H_
#define _FASTCAULDRON__NODAL_VOLUME_GRID__H_

#include "NodalGrid.h"

/// Class describing processor-local regular 3d-node grids.
class NodalVolumeGrid {

public :

   NodalVolumeGrid ();

   /// Construct the nodal-volume grid.
   NodalVolumeGrid ( const NodalGrid& grid,
                     const int        numberOfZNodes,
                     const int        numberOfDofs );


   ~NodalVolumeGrid ();


   /// Construct the nodal-volume grid.
   void construct ( const NodalGrid& grid,
                    const int        numberOfZNodes,
                    const int        numberOfDofs );

   /// Resize the nodal-grid in the z-direction with the new number of z-nodes.
   ///
   /// The nodal-grid must have been constructed already.
   void resizeInZDirection ( const int numberOfZNodes );

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

   /// Get the total number of nodes in the z-direction.
   int getNumberOfZNodes () const;


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

   /// Get the local z-start position. 
   ///
   /// There is no partitioning in the z-direciton, so there can be no ghost nodes.
   int firstK () const;

   /// Get the local z-end position. 
   ///
   /// This is a closed interval.
   /// There is no partitioning in the z-direciton, so there can be no ghost nodes.
   int lastK () const;

   /// Get the number of dofs per node.
   int getNumberOfDofs () const;

   /// The DA object on which the nodal-grid is based.
   DM getDa ();

   /// The DA object on which the nodal-grid is based.
   DM getDa () const;

   /// Return whether or not the object has been initialised.
   bool isInitialised () const;

private :

   // make class non-copyable
   NodalVolumeGrid ( const NodalVolumeGrid& grid );

   void  operator=( const NodalVolumeGrid& grid );

   DMDALocalInfo m_localInfo;

   // Should this really point to the arrays in the nodal-map-grid?
   int* m_xPartitioning;
   int  m_numberOfXProcessors;

   // Should this really point to the arrays in the nodal-map-grid?
   int* m_yPartitioning;
   int  m_numberOfYProcessors;


}; 

/// \typedef NodalVolumeGridArray
/// \brief Array of pointers to NodalVolumeGrid.
typedef std::vector<NodalVolumeGrid*> NodalVolumeGridArray;


//------------------------------------------------------------//
// Inline functions
//------------------------------------------------------------//

inline DM NodalVolumeGrid::getDa () {
   return m_localInfo.da;
}

inline DM NodalVolumeGrid::getDa () const {
   return m_localInfo.da;
}

inline int NodalVolumeGrid::getNumberOfXNodes () const {
   return m_localInfo.mx;
}

inline int NodalVolumeGrid::getNumberOfXProcessors () const {
   return m_numberOfXProcessors;
} 

inline int* NodalVolumeGrid::getXPartitioning () const {
   return m_xPartitioning;
}

inline int NodalVolumeGrid::getNumberOfYNodes () const {
   return m_localInfo.my;
}

inline int NodalVolumeGrid::getNumberOfYProcessors () const {
   return m_numberOfYProcessors;
} 

inline int* NodalVolumeGrid::getYPartitioning () const {
   return m_yPartitioning;
}

inline int NodalVolumeGrid::getNumberOfZNodes () const {
   return m_localInfo.mz;
}

inline int NodalVolumeGrid::firstI ( const bool includeGhosts ) const {
   return ( includeGhosts ? m_localInfo.gxs : m_localInfo.xs );
}

inline int NodalVolumeGrid::lastI ( const bool includeGhosts ) const {
   return ( includeGhosts ? m_localInfo.gxs + m_localInfo.gxm - 1 : m_localInfo.xs + m_localInfo.xm - 1 );
}

inline int NodalVolumeGrid::firstJ ( const bool includeGhosts ) const {
   return ( includeGhosts ? m_localInfo.gys : m_localInfo.ys );
}

inline int NodalVolumeGrid::lastJ ( const bool includeGhosts ) const {
   return ( includeGhosts ? m_localInfo.gys + m_localInfo.gym - 1 : m_localInfo.ys + m_localInfo.ym - 1 );
}

inline int NodalVolumeGrid::firstK () const {
   return m_localInfo.zs;
}

inline int NodalVolumeGrid::lastK () const {
   return m_localInfo.zs + m_localInfo.zm - 1;
}

inline int NodalVolumeGrid::getNumberOfDofs () const {
   return m_localInfo.dof;
}

inline bool NodalVolumeGrid::isInitialised () const {
   // This test could be applied to the y-partitioning array or the DA.
   return m_xPartitioning != 0;
}

#endif // _FASTCAULDRON__NODAL_VOLUME_GRID__H_
