#ifndef _FASTCAULDRON__ELEMENT_VOLUME_GRID__H_
#define _FASTCAULDRON__ELEMENT_VOLUME_GRID__H_

#include <vector>

#include "petsc.h"
#include "petscdmda.h"

#include "ElementGrid.h"

/// \brief Class describing processor-local regular 3d-element grids.
class ElementVolumeGrid {

public :

   ElementVolumeGrid ();

   ~ElementVolumeGrid ();


   /// Construct the element volume grid.
   void construct ( const ElementGrid& grid,
                    const int          numberOfZElements,
                    const int          numberOfDofs );

   /// Resize the element grid in the z-direction with the new number of z-elements.
   ///
   /// The element-grid must have been constructed already.
   void resizeInZDirection ( const int numberOfZElements );

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

   /// Get the total number of elements in the z-direction.
   int getNumberOfZElements () const;


   /// \brief Indicate whether or not the element is part of the valid set of elements.
   ///
   /// The valid set of elements consists of "normal" elements and ghost elements.
   bool isPartOfStencil ( const int i,
                          const int j,
                          const int k ) const;


   /// Get the local x-start position. 
   int firstI ( const bool includeGhosts = false ) const;

   /// Get the local x-end position. 
   ///
   /// This is a closed interval.
   int lastI ( const bool includeGhosts = false ) const;

   /// \brief The number of items in the I-dimension.
   int lengthI ( const bool includeGhosts = false ) const;

   /// Get the local y-start position. 
   int firstJ ( const bool includeGhosts = false ) const;

   /// Get the local y-end position. 
   ///
   /// This is a closed interval.
   int lastJ ( const bool includeGhosts = false ) const;

   /// \brief The number of items in the J-dimension.
   int lengthJ ( const bool includeGhosts = false ) const;

   /// Get the local z-start position. 
   ///
   /// There is no partitioning in the z-direciton, so there can be no ghost nodes.
   int firstK () const;

   /// Get the local z-end position. 
   ///
   /// This is a closed interval.
   /// There is no partitioning in the z-direciton, so there can be no ghost nodes.
   int lastK () const;

   /// \brief The number of items in the K-dimension.
   int lengthK () const;

   /// Get the number of dofs per node.
   int getNumberOfDofs () const;

   /// The DA object on which the element-grid is based.
   DM getDa ();

   /// The DA object on which the element-grid is based.
   DM getDa () const;

   /// Return whether or not the object has been initialised.
   bool isInitialised () const;

private :

   DMDALocalInfo m_localInfo;

   // Should this really point to the arrays in the element-map-grid?
   int* m_xPartitioning;
   int  m_numberOfXProcessors;

   // Should this really point to the arrays in the element-map-grid?
   int* m_yPartitioning;
   int  m_numberOfYProcessors;

   int m_first [ 3 ];
   int m_last  [ 3 ];
   int m_ghostFirst [ 3 ];
   int m_ghostLast  [ 3 ];

}; 

/// \typedef ElementVolumeGridArray
/// \brief Array of pointers to ElementVolumeGrid.
typedef std::vector<ElementVolumeGrid*> ElementVolumeGridArray;


//------------------------------------------------------------//
// Inline functions
//------------------------------------------------------------//

inline DM ElementVolumeGrid::getDa () {
   return m_localInfo.da;
}

inline DM ElementVolumeGrid::getDa () const {
   return m_localInfo.da;
}

inline int ElementVolumeGrid::getNumberOfXElements () const {
   return m_localInfo.mx;
}

inline int ElementVolumeGrid::getNumberOfXProcessors () const {
   return m_numberOfXProcessors;
} 

inline int* ElementVolumeGrid::getXPartitioning () const {
   return m_xPartitioning;
}

inline int ElementVolumeGrid::getNumberOfYElements () const {
   return m_localInfo.my;
}

inline int ElementVolumeGrid::getNumberOfYProcessors () const {
   return m_numberOfYProcessors;
} 

inline int* ElementVolumeGrid::getYPartitioning () const {
   return m_yPartitioning;
}

inline int ElementVolumeGrid::getNumberOfZElements () const {
   return m_localInfo.mz;
}

inline int ElementVolumeGrid::firstI ( const bool includeGhosts ) const {
   return ( includeGhosts ? m_ghostFirst [ 0 ] : m_first [ 0 ]);
}

inline int ElementVolumeGrid::lastI ( const bool includeGhosts ) const {
   return ( includeGhosts ? m_ghostLast [ 0 ] : m_last [ 0 ]);
}

inline int ElementVolumeGrid::lengthI ( const bool includeGhosts ) const {
   return lastI ( includeGhosts ) - firstI ( includeGhosts ) + 1;
}

inline int ElementVolumeGrid::firstJ ( const bool includeGhosts ) const {
   return ( includeGhosts ? m_ghostFirst [ 1 ] : m_first [ 1 ]);
}

inline int ElementVolumeGrid::lastJ ( const bool includeGhosts ) const {
   return ( includeGhosts ? m_ghostLast [ 1 ] : m_last [ 1 ]);
}

inline int ElementVolumeGrid::lengthJ ( const bool includeGhosts ) const {
   return lastJ ( includeGhosts ) - firstJ ( includeGhosts ) + 1;
}

inline int ElementVolumeGrid::firstK () const {
   return m_first [ 2 ];
   return m_localInfo.zs;
}

inline int ElementVolumeGrid::lastK () const {
   return m_last [ 2 ];
}

inline int ElementVolumeGrid::lengthK () const {
   return lastK () - firstK () + 1;
}

inline int ElementVolumeGrid::getNumberOfDofs () const {
   return m_localInfo.dof;
}

inline bool ElementVolumeGrid::isInitialised () const {
   // This test could be applied to the y-partitioning array or the DA.
   return m_xPartitioning != 0;
}

#endif // _FASTCAULDRON__ELEMENT_VOLUME_GRID__H_
