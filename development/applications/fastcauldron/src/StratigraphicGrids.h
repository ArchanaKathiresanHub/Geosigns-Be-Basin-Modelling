//                                                                      
// Copyright (C) 2015 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#ifndef FASTCAULDRON__STRATIGRAPHIC_GRIDS__H
#define FASTCAULDRON__STRATIGRAPHIC_GRIDS__H

// STL library.
#include <vector>

// PETSc library
#include "petsc.h"

// Access to fastcauldron application code.
#include "layer.h"
#include "NodalVolumeGrid.h"
#include "ElementVolumeGrid.h"

/// \brief A helper class containing a set of node grids with varying number of dofs.
class StratigraphicGrids {

public:

   /// \brief Default constructor.
   StratigraphicGrids ();

   /// \brief Destructor.
   ~StratigraphicGrids ();


   // Are both parameters needed, node = element + 1
   /// \brief Resize all of the element and node grids that have been allocated.
   ///
   /// \param [in] elementCount The number of element in depth for all the allocated element grids.
   /// \param [in] nodeCount    The number of nodes in depth for all the allocated node grids.
   void resizeGrids ( const int elementCount,
                      const int nodeCount );

   /// \name Node grid creation and access.
   //@{

   /// \brief Create a node-grid with the number of dofs indicated and add it to the array of node-grids.
   ///
   /// If node-grid exists already then no action will be taken.
   /// \param [in] numberOfDofs The number of degrees of freedom for each node in the grid.
   /// \pre numberOfDofs > 0
   void allocateNodeGrid ( const int numberOfDofs ) const;

   /// \brief Return reference to NodalGrid with corresponding number of dofs.
   ///
   /// If the corresponding node-grid does not exist then one will be created.
   /// \param [in] numberOfDofs The number of degrees of freedom for each node in the grid.
   /// \pre numberOfDofs > 0
   NodalVolumeGrid& getNodeGrid ( const int numberOfDofs = 1 );

   /// \brief Return const reference to NodalGrid with corresponding number of dofs.
   ///
   /// If the corresponding node-grid does not exist then one will be created.
   /// \param [in] numberOfDofs The number of degrees of freedom for each node in the grid.
   /// \pre numberOfDofs > 0
   const NodalVolumeGrid& getNodeGrid ( const int numberOfDofs = 1 ) const;

   /// \brief Allocate a petsc vector.
   Vec createNodeVector ( const int numberOfDofs = 1 ) const;

   /// \brief Get the number of nodes.
   int getNumberOfNodes () const;

   //@}

   /// \name Element grid creation and access.
   //@{

   /// \brief Create an element-grid with the number of dofs indicated and add it to the array of element-grids.
   ///
   /// If element-grid exists already then no action will be taken.
   /// \param [in] numberOfDofs The number of degrees of freedom for each element in the grid.
   /// \pre numberOfDofs > 0
   void allocateElementGrid ( const int numberOfDofs ) const;

   /// \brief Return reference to ElementVolumeGrid with corresponding number of dofs.
   ///
   /// If the corresponding element-grid does not exist then one will be created.
   /// \param [in] numberOfDofs The number of degrees of freedom for each element in the grid.
   /// \pre numberOfDofs > 0
   ElementVolumeGrid& getElementGrid ( const int numberOfDofs = 1 );

   /// \brief Return const reference to ElementVolumeGrid with corresponding number of dofs.
   ///
   /// If the corresponding element-grid does not exist then one will be created.
   /// \param [in] numberOfDofs The number of degrees of freedom for each element in the grid.
   /// \pre numberOfDofs > 0
   const ElementVolumeGrid& getElementGrid ( const int numberOfDofs = 1 ) const;

   /// \brief Allocate a petsc vector.
   Vec createElementVector ( const int numberOfDofs = 1 ) const;

   /// \brief Get the number of elements.
   int getNumberOfElements () const;

   //@}

private :

   // Disallow copying.
   StratigraphicGrids ( const StratigraphicGrids& copy ); // = delete;

   StratigraphicGrids& operator=( const StratigraphicGrids& copy ); // = delete;



   /// Array of element-volume-grids.
   mutable ElementVolumeGridArray m_elementVolumeGrids;

   /// Array of nodal-volume-grids.
   mutable NodalVolumeGridArray m_nodalGrids;

   /// \brief The number of element in the z-direction for element grids.
   int m_numberOfElements;

   /// \brief The number of nodes in the z-direction for node grids.
   int m_numberOfNodes;

};

inline int StratigraphicGrids::getNumberOfNodes () const {
   return m_numberOfNodes;
}

inline int StratigraphicGrids::getNumberOfElements () const {
   return m_numberOfElements;
}

#endif // FASTCAULDRON__STRATIGRAPHIC_GRIDS__H
