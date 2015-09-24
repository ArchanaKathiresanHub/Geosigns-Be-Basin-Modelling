//                                                                      
// Copyright (C) 2015 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#ifndef FASTCAULDRON__COMPUTATIONAL_DOMAIN__H
#define FASTCAULDRON__COMPUTATIONAL_DOMAIN__H

// STL library.
#include <vector>
#include <string>

// Access to PETSc library
#include "petsc.h"

// Access Geophysics and DataAccess library.
#include "Local3DArray.h"

// Access to fastcauldron application code.
#include "CompositeElementActivityPredicate.h"
#include "FormationElementGrid.h"
#include "FormationMapping.h"
#include "GeneralElement.h"
#include "NodalVolumeGrid.h"
#include "StratigraphicColumn.h"
#include "StratigraphicGrids.h"
#include "globaldefs.h"
#include "layer.h"

/// \brief The domain on which the coupled, pressure or temperature equations will be solved.
///
/// For the age that has been assigned a computational domain object will contain:
///    1. The number of active nodes for this rank;
///    2. The number of active nodes for all ranks;
///    3. The number of active elements for this rank;
///    4. A list containing the active elements for this rank;
///    5. An array containing the k-index values on this rank which includes the ghost nodes.
///
/// Each active node will have a unique global dof number.
///
/// What is a k-index? A k-index is ...
///
/// For PETSc DM objects the counting starts with 0 at the bottom of each needle and counts upwards.
class ComputationalDomain {

   /// \typedef FormationGeneralElementGrid
   typedef FormationElementGrid<GeneralElement> FormationGeneralElementGrid;

   /// \typedef FormationToElementGridMap
   /// \brief Mapping from a formation to formation-subdomain-element-grid.
   // Should re-do the subdomain class to use the vector directly.
   typedef FormationMapping< FormationGeneralElementGrid > FormationToElementGridMap;

   /// \brief Array of pointers to GeneralElement.
   ///
   /// The raw pointer is used here because we take a reference of the element from the 
   /// 3d array of elements. It will be used to hold a list of elements that are active.
   typedef std::vector<GeneralElement*> GeneralElementArray;

public :


   /// \brief The value given to inactive degrees of freedom.
   static const int NullDofNumber;

   /// \brief Constructor with top and bottom layers.
   ///
   /// The top and bottom layers or the computational domain.
   /// The domain will consists of all layer in between and including these layers.
   /// \param [in] topLayer The top layer of the computational domain.
   /// \param [in] bottomLayer The bottom layer of the computational domain.
   /// \pre topLayer should be at the same depth or shallower that bottomLayer.
   ComputationalDomain ( const LayerProps& topLayer,
                         const LayerProps& bottomLayer,
                         const CompositeElementActivityPredicate& activityPredicate );

   /// \brief Destructor.
   ~ComputationalDomain ();

   /// \brief Reset the ComputationalDomain to the new age.
   ///
   /// This will update the DOF numbers, active elements, ...
   /// \param [in] age     The age to which the domain should be set.
   /// \param [in] verbose Control how much output to be printed.
   void resetAge ( const double age,
                   const bool   verbose = false );

   /// \brief Return the current age of the computational domain.
   double getCurrentAge () const;


   /// \brief Return the first dof number for this process.
   int getLocalStartDof () const;

   /// \brief Return the number of active nodes for the rank.
   int getLocalNumberOfActiveNodes () const;

   /// \brief Determine if the dof number is from the dofs that are local to the process or not.
   bool isLocalDof ( const int dof ) const;

   /// \brief Return the number of active nodes for all ranks.
   int getGlobalNumberOfActiveNodes () const;

   /// \brief Get the maximum number of zero thickness segments for the rank.
   ///
   /// This number will include zero thickness segments that are local to
   /// the rank and for the ghost nodes.
   int getMaximumNumberDegenerateSegments () const;

   /// \brief Return the number of active elements for the rank.
   int getLocalNumberOfActiveElements () const;

   /// \brief Get a const reference to element at position i.
   ///
   /// Only active elements that are local to the rank will be returned here.
   /// \param [in] i The index of the element required.
   /// \pre i should be in the half open interval [0,getLocalNumberOfActiveElements ())
   const GeneralElement& getElement ( const size_t i ) const;

   /// \brief Get a reference to element at position i.
   ///
   /// Only active elements that are local to the rank will be returned here.
   /// \param [in] i The index of the element required.
   /// \pre i should be in the half open interval [0,getLocalNumberOfActiveElements ())
   GeneralElement& getElement ( const size_t i );


   /// \brief Get the stratigraphic column for the computational domain.
   const StratigraphicColumn& getStratigraphicColumn () const;

   /// \brief Get access to node grids for this computational domain.
   const StratigraphicGrids& getStratigraphicGrids () const;


   /// \brief Get 3 dimensional array of node activity.
   ///
   /// A node is active it it takes active part the computation for the computational-domain.
   /// If n nodes are at the same location, due to a series of zero thickness segments, then 
   /// only 1 of them will be active.
   /// The active nodes array is valid only for the nodes that are local to the rank, it 
   /// does not include the ghost nodes.
   const LocalBooleanArray3D& getActiveNodes () const;

   /// \brief Get the 3 dimensional array containing the k-index values.
   ///
   /// The index values are for both local and ghost nodes of the rank.
   const LocalIntegerArray3D& getDepthIndices () const;

   /// \brief Get the PETSc vector that contains the global dof numbers.
   Vec getDofVector () const;

   //
   // New Dof2DMDAMapping (or something like that)
   // is able to map from new dofs numbering to layer solution vectors and vice-versa.
   // Could be 3d array, like getDepthIndices, with dof numbers instead of depth-k-indices.
   // 
   //
   //
   //
   //
   //


   /// \brief Get the formation-grid associated with the formation.
   const FormationGeneralElementGrid* getFormationGrid ( const LayerProps* layer ) const;


private :

   // Disallow default construction and copying.
   ComputationalDomain (); // = delete;

   ComputationalDomain ( const ComputationalDomain& copy ); // = delete;

   ComputationalDomain& operator=( const ComputationalDomain& copy ); // = delete;


   /// \brief Add element pointers to neighbouring elements across a formation surface.
   void linkElementsVertically ();

   /// \brief Number the k-values for all the nodes.
   ///
   /// Describe k-values (perhaps change name)
   void numberNodeDofs ( const bool verbose );

   /// \brief Number the dofs on the local process with the correct global dof number.
   void numberGlobalDofs ( const bool verbose );

   /// \brief Resize all arrays and vectors to the new size.
   void resizeGrids ( const int previousNodeCount,
                      const int newNodeCount );

   /// \brief Set the k-values for the nodes of each of the elements.
   void setElementNodeKValues ( const bool verbose );

   /// \brief Determines all active elements that are local to the mpi rank.
   ///
   /// This is achieved by looping over all element in the domain and using the activity
   /// predicate to determine if the element is considered active.
   void determineActiveElements ( const bool verbose );

   /// \brief Determines all active nodes that are both local to the mpi rank and its ghost nodes.
   ///
   /// Once all the active element has been determined: for each active element loop over all
   /// nodes to find the set of active nodes. The node may be owned by another process.
   void determineActiveNodes ( const bool verbose );

   /// \brief Assign the global dof number to the nodes of every active element.
   void getElementGobalDofNumbers ();

   /// \brief Assign the depth index numbers based on depth values.
   ///
   /// This function updates the member m_depthIndexNumbers.
   void assignDofNumbersUsingDepth ( const FormationGeneralElementGrid& layerGrid,
                                     const PETSC_3D_Array&              layerDepth,
                                     const int                          i,
                                     const int                          j,
                                     int&                               globalK,
                                     int&                               activeSegments,
                                     int&                               inactiveSegments,
                                     int&                               maximumDegenerateSegments );

   /// \brief Assign the depth index numbers based on segments thicknesses.
   ///
   /// This function updates the member m_depthIndexNumbers.
   void assignDofNumbersUsingThickness ( const FormationGeneralElementGrid& layerGrid,
                                         const int                          i,
                                         const int                          j,
                                         int&                               globalK,
                                         int&                               activeSegments,
                                         int&                               inactiveSegments,
                                         int&                               maximumDegenerateSegments );

   /// \brief The stratigraphic column for this computational domain.
   StratigraphicColumn               m_column;

   /// \brief Contains the node and elements grids for the current size domain.
   StratigraphicGrids                m_grids;

   /// \brief Mapping from layer to formation-element-grid.
   FormationToElementGridMap         m_layerMap;

   CompositeElementActivityPredicate m_activityPredicate;

   /// \brief Is any layer active.
   bool                              m_isActive;

   /// \brief All the active elements in the computational domain.
   GeneralElementArray               m_activeElements;

   /// \brief Array containing the k-index numbers for the computational domain.
   LocalIntegerArray3D               m_depthIndexNumbers;

   /// \brief Vector containing the dof numbers for the computational domain.
   Vec                               m_globalDofNumbers;

   /// \brief Array indicating which node is active 
   LocalBooleanArray3D               m_activeNodes;

   /// \brief The current age for the computational domain.
   double                            m_currentAge;

   /// \brief The maximum number of elements that can be active in the part of the domain that lies on this rank.
   int                               m_maximumNumberOfElements;

   /// \brief Array of number of active nodes on al processes.
   IntegerArray                      m_numberOfActiveNodesPerProcess;

   /// \brief The maximum number of zero thickness segments for this process
   ///
   /// This will include local and ghost segments.
   int                               m_localMaximumNumberDegenerateSegments;

   /// \brief The rank on which this part of the computational domain lies.
   int                               m_rank;

   /// \brief The number of the first degree of freedom on this process.
   int                               m_localStartDofNumber;

};

//------------------------------------------------------------//
//                     Inline functions                       //
//------------------------------------------------------------//

inline const StratigraphicColumn& ComputationalDomain::getStratigraphicColumn () const {
   return m_column;
}

inline const StratigraphicGrids& ComputationalDomain::getStratigraphicGrids () const {
   return m_grids;
}

inline double ComputationalDomain::getCurrentAge () const {
   return m_currentAge;
}

inline int ComputationalDomain::getMaximumNumberDegenerateSegments () const {
   return m_localMaximumNumberDegenerateSegments;
}

inline int ComputationalDomain::getLocalNumberOfActiveElements () const {
   return m_activeElements.size ();
}

inline int ComputationalDomain::getLocalStartDof () const {
   return m_localStartDofNumber;
}

inline int ComputationalDomain::getLocalNumberOfActiveNodes () const {
   return m_numberOfActiveNodesPerProcess [ m_rank ];
}

inline bool ComputationalDomain::isLocalDof ( const int dof ) const {
   return m_localStartDofNumber <= dof and dof < m_localStartDofNumber + m_numberOfActiveNodesPerProcess [ m_rank ];
}

inline const LocalIntegerArray3D& ComputationalDomain::getDepthIndices () const {
   return m_depthIndexNumbers;
}

inline const LocalBooleanArray3D& ComputationalDomain::getActiveNodes () const {
   return m_activeNodes;
}

inline const GeneralElement& ComputationalDomain::getElement ( const size_t i ) const {
   return *m_activeElements [ i ];
}

inline GeneralElement& ComputationalDomain::getElement ( const size_t i ) {
   return *m_activeElements [ i ];
}

inline const ComputationalDomain::FormationGeneralElementGrid*
ComputationalDomain::getFormationGrid ( const LayerProps* layer ) const {

   FormationToElementGridMap::const_iterator iter = m_layerMap.find ( layer );

   if ( iter != m_layerMap.end ()) {
      return iter->m_formationGrid;
   } else {
      return 0;
   }

}

inline Vec ComputationalDomain::getDofVector () const {
   return m_globalDofNumbers;
}


#endif // FASTCAULDRON__COMPUTATIONAL_DOMAIN__H
