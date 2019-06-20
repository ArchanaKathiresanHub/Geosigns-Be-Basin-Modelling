//                                                                      
// Copyright (C) 2015-2018 Shell International Exploration & Production.
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

// Access DataAccess library.
#include "Local3DArray.h"

// Access to fastcauldron application code.
#include "CompositeElementActivityPredicate.h"
#include "FormationElementGrid.h"
#include "FormationMapping.h"
#include "GeneralElement.h"
#include "ConstantsFastcauldron.h"
#include "layer.h"
#include "NodalVolumeGrid.h"
#include "PetscBlockVector.h"
#include "StratigraphicColumn.h"
#include "StratigraphicGrids.h"

/// \brief The domain on which the coupled, pressure or temperature equations will be solved.
///
/// For the age that has been assigned a computational domain object will contain:
///    1. The number of active nodes for this MPI rank;
///    2. The number of active nodes for all MPI ranks;
///    3. The number of active elements for this MPI rank;
///    4. A list containing the active elements for this MPI rank;
///    5. An array containing the depth index values on this MPI rank which includes the ghost nodes.
///
/// Each active node will have a unique global dof number.
///
/// Depth indicies are the values of the k-index value for the node in the depth direction.
/// If there are zero thickness segments then the nodes connecting these nodes will
/// have the same depth index value.
///
/// For PETSc DM objects the counting starts with 0 at the bottom of each needle and counts upwards.
class ComputationalDomain {

public :

   /// \brief Indicator for the ordering of the dofs.
   ///
   /// IJK -> numbers in depth are consecutive.
   /// KIJ -> numbers in depth are not consecutive.
   /// KJI -> numbers in depth are not consecutive.
   enum DofOrdering { IJKOrder, KIJOrder, KJIOrder };

   /// \typedef FormationGeneralElementGrid
   typedef FormationElementGrid<GeneralElement> FormationGeneralElementGrid;


   /// \brief The value given to inactive degrees of freedom.
   static const int NullDofNumber;

   /// \brief Constructor with top and bottom layers.
   ///
   /// The domain will consists of all layer in between and including the top and bottom layers.
   /// \param [in] topLayer The top layer of the computational domain.
   /// \param [in] bottomLayer The bottom layer of the computational domain.
   /// \param [in] activityPredicate The predicate used to determine if an element is to be included in the computational domain.
   /// \pre topLayer should be at the same depth or shallower than the bottomLayer.
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

   /// \brief Return the number of active nodes for the process.
   int getLocalNumberOfActiveNodes () const;

   /// \brief Return the number of active nodes for all processes.
   int getGlobalNumberOfActiveNodes () const;

   /// \brief Return the number of active elements for the process.
   int getLocalNumberOfActiveElements () const;

   /// \brief Get a const reference to element at position i.
   ///
   /// Only active elements that are local to the process will be returned here.
   /// \param [in] i The index of the element required.
   /// \pre i should be in the half open interval [0,getLocalNumberOfActiveElements ())
   const GeneralElement& getActiveElement ( const size_t i ) const;

   /// \brief Get a reference to the element activity predicate.
   const CompositeElementActivityPredicate& getActivityPredicate () const;


   /// \brief Get the stratigraphic column for the computational domain.
   const StratigraphicColumn& getStratigraphicColumn () const;

   /// \brief Get access to node grids for this computational domain.
   const StratigraphicGrids& getStratigraphicGrids () const;


   /// \brief Get 3 dimensional array of node activity.
   ///
   /// A node is active if it takes part the calculation for the computational-domain.
   /// If n nodes are at the same location, due to a series of zero thickness segments, then 
   /// only 1 of them will be active.
   /// The active nodes array is valid only for the nodes that are local to the process, it 
   /// does not include the ghost nodes.
   const LocalBooleanArray3D& getActiveNodes () const;

   /// \brief Get the 3 dimensional array containing the depth index values.
   ///
   /// The index values are for both local and ghost nodes of the process.
   const LocalIntegerArray3D& getDepthIndices () const;

   /// \brief Get the PETSc vector that contains the global dof numbers.
   Vec getDofVector () const;

   /// \brief Get the ordering of the dofs.
   DofOrdering getDofOrdering () const;

   /// \brief Get the PETSc IS local to global mapping.
   ISLocalToGlobalMapping getLocalToGlobalMapping () const;


   /// \brief Get the formation-grid associated with the formation.
   const FormationGeneralElementGrid* getFormationGrid ( const LayerProps* layer ) const;

   /// \brief Get the formation-grid associated with the formation.
   FormationGeneralElementGrid* getFormationGrid ( const LayerProps* layer );

private :

   /// \typedef FormationToElementGridMap
   /// \brief Mapping from a formation to formation-subdomain-element-grid.
   typedef FormationMapping< FormationGeneralElementGrid > FormationToElementGridMap;

   /// \brief Array of pointers to GeneralElement.
   ///
   /// The raw pointer is used here because we take a reference of the element from the 
   /// 3d array of elements. It will be used to hold a list of elements that are active.
   typedef std::vector<GeneralElement*> GeneralElementArray;


   // Disallow default construction and copying.
   ComputationalDomain (); // = delete;

   ComputationalDomain ( const ComputationalDomain& copy ); // = delete;

   ComputationalDomain& operator=( const ComputationalDomain& copy ); // = delete;


   /// \brief Add element pointers to neighbouring elements between two adjacent layers across their connecting surface.
   void linkElementsVertically ();

   /// \brief Number the depth values for all the nodes.
   void numberDepthIndices ( const bool verbose );

   /// \brief Number the dofs on the local process with the correct global dof number.
   void numberGlobalDofs ( const bool verbose );

   /// \brief Number the dofs in IJK order.
   void numberGlobalDofsIJK ( int&                      globalDofNumber,
                              PetscBlockVector<double>& dof );

   /// \brief Number the dofs in KIJ order.
   void numberGlobalDofsKIJ ( int&                      globalDofNumber,
                              PetscBlockVector<double>& dof );

   /// \brief Number the dofs in KJI order.
   void numberGlobalDofsKJI ( int&                      globalDofNumber,
                              PetscBlockVector<double>& dof );


   /// \brief Resize all arrays and vectors to the new size.
   void resizeGrids ( const int newNodeCount );

   /// \brief Set the depth values for the nodes of each of the elements.
   void setElementNodeDepthIndices ( const bool verbose );

   /// \brief Determines all active elements that are local to the mpi process.
   ///
   /// This is achieved by looping over all element in the domain and using the activity
   /// predicate to determine if the element is considered active.
   void determineActiveElements ( const bool verbose );

   /// \brief Determines all active nodes that are both local to the mpi process and its ghost nodes.
   ///
   /// Once all the active element has been determined: for each active element loop over all
   /// nodes to find the set of active nodes. The node may be owned by another process.
   void determineActiveNodes ( const bool verbose );

   /// \brief Assign the global dof number to the nodes of every active element.
   void assignElementGobalDofNumbers ();

   /// \brief Number the PETSc IS local to global mapping.
   void numberLocalToGlobalMapping ();

   /// \brief Assign the depth index numbers based on depth values.
   ///
   /// This function updates the member m_depthIndexNumbers.
   /// Currently this is only necessary for the mantle layer.
   void assignDepthIndicesUsingDepth ( const FormationGeneralElementGrid& layerGrid,
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
   void assignDepthIndicesUsingThickness ( const FormationGeneralElementGrid& layerGrid,
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

   /// \brief Used to indicate if an element is considered active or not.
   ///
   /// Only active elements will be used in the calculation of pressure or temperature.
   CompositeElementActivityPredicate m_activityPredicate;

   /// \brief Is any layer active.
   bool                              m_isActive;

   /// \brief All the active elements in the computational domain.
   GeneralElementArray               m_activeElements;

   /// \brief Array containing the depth index numbers for the computational domain.
   LocalIntegerArray3D               m_depthIndexNumbers;

   /// \brief Vector containing the dof numbers for the computational domain.
   Vec                               m_globalDofNumbers;

   /// \brief Local to global mapping.
   ISLocalToGlobalMapping            m_local2global;

   /// \brief Array indicating which node is active 
   LocalBooleanArray3D               m_activeNodes;

   /// \brief The current age for the computational domain.
   double                            m_currentAge;

   /// \brief The maximum number of elements that can be active in the part of the domain that lies on this process.
   int                               m_maximumNumberOfElements;

   /// \brief Array of number of active nodes on al processes.
   IntegerArray                      m_numberOfActiveNodesPerProcess;

   /// \brief The process on which this part of the computational domain lies.
   int                               m_rank;

   /// \brief The number of the first degree of freedom on this process.
   int                               m_localStartDofNumber;

   /// \brief Indicates in which order the dofs are to be numbered.
   DofOrdering                       m_dofOrdering;

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

inline int ComputationalDomain::getLocalNumberOfActiveElements () const {
   return static_cast<int>( m_activeElements.size () );
}

inline int ComputationalDomain::getLocalStartDof () const {
   return m_localStartDofNumber;
}

inline ComputationalDomain::DofOrdering ComputationalDomain::getDofOrdering () const {
   return m_dofOrdering;
}

inline int ComputationalDomain::getLocalNumberOfActiveNodes () const {
   return m_numberOfActiveNodesPerProcess [ m_rank ];
}

inline const LocalIntegerArray3D& ComputationalDomain::getDepthIndices () const {
   return m_depthIndexNumbers;
}

inline const LocalBooleanArray3D& ComputationalDomain::getActiveNodes () const {
   return m_activeNodes;
}

inline const GeneralElement& ComputationalDomain::getActiveElement ( const size_t i ) const {
   return *m_activeElements [ i ];
}

inline const CompositeElementActivityPredicate& ComputationalDomain::getActivityPredicate () const {
   return m_activityPredicate;
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

inline ComputationalDomain::FormationGeneralElementGrid*
ComputationalDomain::getFormationGrid ( const LayerProps* layer ) {

   FormationToElementGridMap::iterator iter = m_layerMap.find ( layer );

   if ( iter != m_layerMap.end ()) {
      return iter->m_formationGrid;
   } else {
      return 0;
   }

}

inline Vec ComputationalDomain::getDofVector () const {
   return m_globalDofNumbers;

}

inline ISLocalToGlobalMapping ComputationalDomain::getLocalToGlobalMapping () const {
   return m_local2global;
}


#endif // FASTCAULDRON__COMPUTATIONAL_DOMAIN__H
