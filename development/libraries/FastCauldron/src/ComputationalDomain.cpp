//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "ComputationalDomain.h"

// Access STL library.
#include <algorithm>
#include <numeric>
#include <iostream>
#include <iomanip>
#include <sstream>

// Access utilities library.
#include "NumericFunctions.h"

// Access to fastcauldron application code.
#include "FastcauldronSimulator.h"
#include "LayerElement.h"
#include "Lithology.h"
#include "propinterface.h"
using namespace std;
//------------------------------------------------------------//

// This number should be either negative or larger than the maximum
// number of nodes we expect to have for even the largest grid.
const int ComputationalDomain::NullDofNumber = -9999;

//------------------------------------------------------------//

ComputationalDomain::ComputationalDomain ( const LayerProps& topLayer,
                                           const LayerProps& bottomLayer,
                                           const CompositeElementActivityPredicate& activityPredicate ) :
   m_column ( topLayer, bottomLayer ),
   m_activityPredicate ( activityPredicate ),
   m_isActive ( false ),
   m_globalDofNumbers ( PETSC_IGNORE ),
   m_local2global ( PETSC_IGNORE ),
   m_currentAge ( -1.0 ),
   m_rank ( FastcauldronSimulator::getInstance ().getRank ()),
   m_localStartDofNumber ( 0 ),
   m_dofOrdering ( KJIOrder )
{

   const ElementGrid& elementMap = FastcauldronSimulator::getInstance ().getElementGrid ();

   for ( size_t i = 0; i < m_column.getNumberOfLayers (); ++i ) {
      m_layerMap [ m_column.getLayer ( i )] = new FormationGeneralElementGrid ( *m_column.getLayer ( i ));
   }

   // The number of elements will never be greater than this, it may well be less.
   m_maximumNumberOfElements = m_column.getMaximumNumberOfElementsInDepth () * elementMap.lengthI () * elementMap.lengthJ ();
   linkElementsVertically ();

   // Resize the array of number of active nodes to the size of the number of processors.
   m_numberOfActiveNodesPerProcess.resize ( FastcauldronSimulator::getInstance ().getSize (), 0 );
}

//------------------------------------------------------------//

ComputationalDomain::~ComputationalDomain () {

   FormationToElementGridMap::iterator it;

   for ( it = m_layerMap.begin (); it != m_layerMap.end (); ++it ) {
      delete it->m_formationGrid;
   }

   PetscBool isValid;

   VecValid ( m_globalDofNumbers, &isValid );

   if ( isValid ) {
      VecDestroy ( &m_globalDofNumbers );
   }

   if ( m_local2global != PETSC_IGNORE ) {
      ISLocalToGlobalMappingDestroy ( &m_local2global );
   }

}

//------------------------------------------------------------//

int ComputationalDomain::getGlobalNumberOfActiveNodes () const {
   return std::accumulate ( m_numberOfActiveNodesPerProcess.begin (), m_numberOfActiveNodesPerProcess.end (), 0 );
}

//------------------------------------------------------------//

void ComputationalDomain::linkElementsVertically () {

   const ElementGrid& elementGrid = FastcauldronSimulator::getInstance ().getElementGrid ();

   FormationGeneralElementGrid* above;
   FormationGeneralElementGrid* below;
   int i;
   int j;
   int belowSize;

   above = m_layerMap [ m_column.getLayer ( 0 )];

   for ( size_t l = 1; l < m_column.getNumberOfLayers (); ++l ) {
      below = m_layerMap [ m_column.getLayer ( l )];
      belowSize = below->lastK ();

      for ( i = elementGrid.firstI (); i <= elementGrid.lastI (); ++i ) {

         for ( j = elementGrid.firstJ (); j <= elementGrid.lastJ (); ++j ) {
            above->getElement ( i, j,         0 ).setNeighbour ( VolumeData::DeepFace,    &below->getElement ( i, j, belowSize ));
            below->getElement ( i, j, belowSize ).setNeighbour ( VolumeData::ShallowFace, &above->getElement ( i, j, 0 ));
         }

      }

      above = below;
   }

}

//------------------------------------------------------------//

void ComputationalDomain::assignDepthIndicesUsingDepth ( const FormationGeneralElementGrid& layerGrid,
                                                         const PETSC_3D_Array&              layerDepth,
                                                         const int                          i,
                                                         const int                          j,
                                                         int&                               globalK,
                                                         int&                               activeSegments,
                                                         int&                               inactiveSegments,
                                                         int&                               maximumDegenerateSegments ) {

   for ( int k = layerGrid.lastK (); k >= layerGrid.firstK (); --k, --globalK ) {
      double segmentThickness = layerDepth ( k, j, i ) - layerDepth ( k + 1, j, i );

      if ( segmentThickness > DepositingThicknessTolerance ) {
         m_depthIndexNumbers ( i, j, globalK ) = globalK;
         ++activeSegments;
      } else {
         m_depthIndexNumbers ( i, j, globalK ) = m_depthIndexNumbers ( i, j, globalK + 1 );
         ++inactiveSegments;
         maximumDegenerateSegments = NumericFunctions::Maximum ( maximumDegenerateSegments,
                                                                 static_cast<int>( m_depthIndexNumbers ( i, j, globalK + 1 ) - globalK ));
      }

   }

}

//------------------------------------------------------------//

void ComputationalDomain::assignDepthIndicesUsingThickness ( const FormationGeneralElementGrid& layerGrid,
                                                             const int                          i,
                                                             const int                          j,
                                                             int&                               globalK,
                                                             int&                               activeSegments,
                                                             int&                               inactiveSegments,
                                                             int&                               maximumDegenerateSegments ) {

   for ( int k = layerGrid.lastK (); k >= layerGrid.firstK (); --k, --globalK ) {

      if ( layerGrid.getFormation ().getDepositingThickness ( i, j, k, m_currentAge ) > DepositingThicknessTolerance ) {
         m_depthIndexNumbers ( i, j, globalK ) = globalK;
         ++activeSegments;
      } else {
         m_depthIndexNumbers ( i, j, globalK ) = m_depthIndexNumbers ( i, j, globalK + 1 );
         ++inactiveSegments;
         maximumDegenerateSegments = NumericFunctions::Maximum ( maximumDegenerateSegments,
                                                                 static_cast<int>( m_depthIndexNumbers ( i, j, globalK + 1 ) - globalK ));
      }

   }

}

//------------------------------------------------------------//

void ComputationalDomain::numberDepthIndices ( const bool verbose ) {

   if ( not m_isActive ) {
      // There is nothing to number.
      return;
   }

   const FastcauldronSimulator& fc = FastcauldronSimulator::getInstance ();

   m_depthIndexNumbers.fill ( NullDofNumber );

   int activeSegments = 0;
   int inactiveSegments = 0;

   int maximumDegenerateSegments = 0;

   // One less because the first index is zero.
   int numberOfNodesInDepth = m_column.getNumberOfLogicalNodesInDepth ( m_currentAge ) - 1;
   size_t firstLayerIndex = m_column.getTopLayerIndex ( m_currentAge );
   int globalK;

   LayerProps* mantleLayer = 0;
   bool mantleDepthRetrieved = true;
   PETSC_3D_Array mantleDepth;

   if ( m_column.getLayer ( m_column.getNumberOfLayers () - 1 )->isMantle ()) {
      // If the stratigraphic column contains the mantle then it will the bottommost layer.
      // The mantle has neither solid nor real thickness history functions for the segments.
      // So the segment thickness is determined from the node depth.
      mantleLayer = m_column.getLayer ( m_column.getNumberOfLayers () - 1 );

      if ( not mantleLayer->Current_Properties.propertyIsActivated ( Basin_Modelling::Depth )) {
         mantleDepthRetrieved = false;
         mantleDepth.Set_Global_Array ( mantleLayer->layerDA, mantleLayer->Current_Properties ( Basin_Modelling::Depth ), INSERT_VALUES, true );
      }

   }

   for ( size_t i = fc.firstI ( true ); i <= fc.lastI ( true ); ++i ) {

      for ( size_t j = fc.firstJ ( true ); j <= fc.lastJ ( true ); ++j ) {

         if ( fc.nodeIsDefined ( i, j )) {

            // Number nodes, top down but in reverse numbering.
            // So the nodes are numbered from the bottom up, i.e. 0 at the bottom.
            m_depthIndexNumbers ( i, j, numberOfNodesInDepth ) = numberOfNodesInDepth;
            globalK = numberOfNodesInDepth - 1;

            for ( size_t l = firstLayerIndex; l < m_column.getNumberOfLayers (); ++l ) {
               const FormationElementGrid<GeneralElement>& grid = *getFormationGrid ( m_column.getLayer ( l ));

               if ( grid.getFormation ().isMantle ()) {
                  assignDepthIndicesUsingDepth ( grid, mantleDepth, i, j, globalK, activeSegments, inactiveSegments, maximumDegenerateSegments );
               } else {
                  assignDepthIndicesUsingThickness ( grid, i, j, globalK, activeSegments, inactiveSegments, maximumDegenerateSegments );
               }

            }

         }

      }

   }

   if ( mantleLayer != 0 and not mantleDepthRetrieved ) {
      mantleDepth.Restore_Global_Array ( No_Update );
   }

   if ( verbose ) {
      std::stringstream buffer;

      buffer << " numberDepthIndices " << activeSegments << "  " << inactiveSegments << std::endl;
      PetscSynchronizedPrintf ( PETSC_COMM_WORLD, buffer.str ().c_str ());
      PetscSynchronizedFlush ( PETSC_COMM_WORLD, PETSC_STDOUT );

      if ( fc.getCauldron ()->debug1 ) {
         double globalStencilWidth;
         double localStencilWidth = static_cast<double>( maximumDegenerateSegments );
         int stencilWidth;

         MPI_Allreduce( &localStencilWidth, &globalStencilWidth, 1, MPI_DOUBLE, MPI_MAX, PETSC_COMM_WORLD);
         stencilWidth = static_cast<int>(globalStencilWidth);
         PetscPrintf ( PETSC_COMM_WORLD, " Maximum number of degenerate segments: %d \n", stencilWidth );
      }

   }

}

//------------------------------------------------------------//

void ComputationalDomain::numberGlobalDofs ( const bool verbose ) {

   if ( not m_isActive ) {
      // There is nothing to number.
      return;
   }

   const NodalVolumeGrid& scalarNodeGrid = m_grids.getNodeGrid ( 1 );
   const double NullDofNumberReal = static_cast<double>( NullDofNumber );

   VecSet ( m_globalDofNumbers, NullDofNumberReal );

   PetscBlockVector<double> dof;

   int globalDofNumber = m_localStartDofNumber;

   dof.setVector ( scalarNodeGrid, m_globalDofNumbers, INSERT_VALUES );

   switch ( m_dofOrdering ) {

      case IJKOrder :
         numberGlobalDofsIJK ( globalDofNumber, dof );
         break;

      case KIJOrder :
         numberGlobalDofsKIJ ( globalDofNumber, dof );
         break;

      case KJIOrder :
         numberGlobalDofsKJI ( globalDofNumber, dof );
         break;

      default :
         PetscPrintf ( PETSC_COMM_WORLD, " Basin_Error: dof ordering incorrectly defined.\n" );
         exit ( 1 );
   }

   if ( verbose and m_rank + 1 == FastcauldronSimulator::getInstance ().getSize ()) {
      std::cout << " Total number of dofs : " << globalDofNumber << std::endl;
   }

   assert (( "Incorrect counting of dofs.", ( globalDofNumber - m_localStartDofNumber ) == getLocalNumberOfActiveNodes ()));

   dof.restoreVector ( UPDATE_EXCLUDING_GHOSTS );

#if PETSC_VIEWER_CREATE
   PetscViewer viewer;
   PetscViewerCreate ( PETSC_COMM_WORLD, &viewer);
   PetscViewerSetType(viewer, PETSCVIEWERASCII );
#if PETSC_VIEWER_FORMAT_MATLAB
   PetscViewerPushFormat( viewer, PETSC_VIEWER_ASCII_MATLAB );
#endif
   VecView ( m_globalDofNumbers, viewer );
#if PETSC_VIEWER_FORMAT_MATLAB
   PetscViewerPopFormat(viewer);
#endif
   PetscViewerDestroy ( &viewer );
#endif

}

//------------------------------------------------------------//

void ComputationalDomain::numberGlobalDofsIJK ( int&                      globalDofNumber,
                                                PetscBlockVector<double>& dof ) {

   const FastcauldronSimulator& fc = FastcauldronSimulator::getInstance ();

   const double NullDofNumberReal = static_cast<double>( NullDofNumber );

   // One less because the first index is zero.
   int numberOfNodesInDepth = m_column.getNumberOfLogicalNodesInDepth ( m_currentAge ) - 1;
   globalDofNumber = m_localStartDofNumber;

   for ( size_t i = fc.firstI (); i <= fc.lastI (); ++i ) {

      for ( size_t j = fc.firstJ (); j <= fc.lastJ (); ++j ) {

         if ( fc.nodeIsDefined ( i, j )) {

            for (unsigned int k = 0; k <= numberOfNodesInDepth; ++k) {

               if ( m_activeNodes ( i, j, k )) {
                  dof ( k, j, i ) = globalDofNumber++;
               } else {
                  // Could the number of the dof that lies directly above this in-active dof be
                  // used to number this one. This may make it easier when extracting the vector
                  // of values and copying them back to the layer 3d array of values.
                  dof ( k, j, i ) = NullDofNumberReal; // Is this the best value?
               }

            }

         }

      }

   }

}

//------------------------------------------------------------//

void ComputationalDomain::numberGlobalDofsKIJ ( int&                      globalDofNumber,
                                                PetscBlockVector<double>& dof ) {

   const FastcauldronSimulator& fc = FastcauldronSimulator::getInstance ();

   const double NullDofNumberReal = static_cast<double>( NullDofNumber );

   // One less because the first index is zero.
   int numberOfNodesInDepth = m_column.getNumberOfLogicalNodesInDepth ( m_currentAge ) - 1;
   globalDofNumber = m_localStartDofNumber;

   for (unsigned int k = 0; k <= numberOfNodesInDepth; ++k) {

      for ( size_t i = fc.firstI (); i <= fc.lastI (); ++i ) {

         for ( size_t j = fc.firstJ (); j <= fc.lastJ (); ++j ) {

            if ( fc.nodeIsDefined ( i, j )) {

               if ( m_activeNodes ( i, j, k )) {
                  dof ( k, j, i ) = globalDofNumber++;
               } else {
                  // Could the number of the dof that lies directly above this in-active dof be
                  // used to number this one. This may make it easier when extracting the vector
                  // of values and copying them back to the layer 3d array of values.
                  dof ( k, j, i ) = NullDofNumberReal; // Is this the best value?
               }

            }

         }

      }

   }

}

//------------------------------------------------------------//

void ComputationalDomain::numberGlobalDofsKJI ( int&                      globalDofNumber,
                                                PetscBlockVector<double>& dof ) {

   const FastcauldronSimulator& fc = FastcauldronSimulator::getInstance ();

   const double NullDofNumberReal = static_cast<double>( NullDofNumber );

   // One less because the first index is zero.
   int numberOfNodesInDepth = m_column.getNumberOfLogicalNodesInDepth ( m_currentAge ) - 1;
   globalDofNumber = m_localStartDofNumber;

   for (unsigned int k = 0; k <= numberOfNodesInDepth; ++k) {

      for ( size_t j = fc.firstJ (); j <= fc.lastJ (); ++j ) {

         for ( size_t i = fc.firstI (); i <= fc.lastI (); ++i ) {

            if ( fc.nodeIsDefined ( i, j )) {

               if ( m_activeNodes ( i, j, k )) {
                  dof ( k, j, i ) = globalDofNumber++;
               } else {
                  // Could the number of the dof that lies directly above this in-active dof be
                  // used to number this one. This may make it easier when extracting the vector
                  // of values and copying them back to the layer 3d array of values.
                  dof ( k, j, i ) = NullDofNumberReal; // Is this the best value?
               }

            }

         }

      }

   }

}

//------------------------------------------------------------//

void ComputationalDomain::assignElementGobalDofNumbers () {

   const NodalVolumeGrid& scalarNodeGrid = m_grids.getNodeGrid ( 1 );

   PetscBlockVector<double> dof;
   dof.setVector ( scalarNodeGrid, m_globalDofNumbers, INSERT_VALUES, true );

   for ( size_t i = 0; i < m_activeElements.size (); ++i ) {
      GeneralElement* element = m_activeElements [ i ];

      // Loop over the nodes of each element collecting the global dof number.
      for ( int n = 0; n < 8; ++n ) {
         element->setDof ( n, static_cast<int>(dof ( element->getNodeK ( n ), element->getNodeJ ( n ), element->getNodeI ( n ))));

#ifdef VERBOSE_ELEMENT_DOF_ASSIGNMENT
         // What are the performance implications of the following assertion?
         assert (( "Dof number should not be the null value.", element->getDof ( n ) != NullDofNumber ));
#else
         assert ( element->getDof ( n ) != NullDofNumber );
#endif

      }

   }

   dof.restoreVector ( NO_UPDATE );
}

//------------------------------------------------------------//

void ComputationalDomain::resetAge ( const double age,
                                     const bool   verbose ) {

   const int newNodeCount = m_column.getNumberOfLogicalNodesInDepth ( age );
   const int elementCount = m_column.getNumberOfLogicalElementsInDepth ( age );

   m_currentAge = age;
   m_isActive = m_column.getNumberOfLayers () > 0 and newNodeCount > 1;

   if ( verbose ) {
      PetscPrintf ( PETSC_COMM_WORLD, " Resetting computational domain: age - %f, nodes - %d \n", age, newNodeCount );
   }

   m_grids.resizeGrids ( elementCount, newNodeCount );
   resizeGrids ( newNodeCount );
   numberDepthIndices ( verbose );
   setElementNodeDepthIndices ( verbose );
   determineActiveElements ( verbose );
   determineActiveNodes ( verbose );
   // Now that the active/inactive nodes have been determined and counted the active nodes can be numbered.
   numberGlobalDofs ( verbose );
   // Now for each active element assign the global dof numbers.
   assignElementGobalDofNumbers ();
   numberLocalToGlobalMapping ();
}

//------------------------------------------------------------//

void ComputationalDomain::setElementNodeDepthIndices ( const bool verbose ) {

   int nodeCount = m_column.getNumberOfLogicalNodesInDepth ( m_currentAge );
   int globalKValue = nodeCount - 1;
   int topValue = nodeCount - 1;

   for ( size_t l = 0; l < m_column.getNumberOfLayers (); ++l ) {

      if ( m_column.getLayer ( l )->depositionStartAge > m_currentAge ) {
         FormationGeneralElementGrid* grid = m_layerMap [ m_column.getLayer ( l )];

         if ( verbose ) {
            PetscPrintf ( PETSC_COMM_WORLD,
                          " Renumber general elements: layer top depth-index value - %d, layer name - %s\n",
                          globalKValue,
                          m_column.getLayer ( l )->getName ().c_str ());
         }

         grid->setElementNodeKValues ( m_depthIndexNumbers, topValue, globalKValue );
         globalKValue -= grid->lengthK ();

         if ( verbose ) {
            const IntegerArray& kIndices = grid->getSubdomainNodeKIndices ();

            PetscPrintf ( PETSC_COMM_WORLD, " depth-indices for layer: " );

            for ( size_t k = 0; k < kIndices.size (); ++k ) {
               PetscPrintf ( PETSC_COMM_WORLD, "%d  ", kIndices [ k ]);
            }

            PetscPrintf ( PETSC_COMM_WORLD, "\n" );
         }
      }

   }

}

//------------------------------------------------------------//

void ComputationalDomain::determineActiveElements ( const bool verbose ) {

   m_activeElements.clear ();
   // Allocate sufficient memory to contain all active elements.
   // Is it worth counting the number of elements that are active to allocate the exact size of the list required?
   m_activeElements.reserve ( m_maximumNumberOfElements );
   int activeElementCount;

   for ( size_t l = 0; l < m_column.getNumberOfLayers (); ++l ) {

      if ( m_column.getLayer ( l )->depositionStartAge > m_currentAge ) {
         FormationGeneralElementGrid* grid = m_layerMap [ m_column.getLayer ( l )];
         activeElementCount = 0;

         for ( int i = grid->firstI (); i <= grid->lastI (); ++i ) {

            for ( int j = grid->firstJ (); j <= grid->lastJ (); ++j ) {

               for ( int k = grid->firstK (); k <= grid->lastK (); ++k ) {
                  const LayerElement& element = grid->getElement ( i, j, k ).getLayerElement ();

                  if ( m_activityPredicate.isActive ( element )) {
                     m_activeElements.push_back ( &(grid->getElement ( i, j, k )));
                     ++activeElementCount;
                  }

               }

            }

         }

         if ( verbose ) {
            // Print line for each MPI process.
            PetscSynchronizedPrintf ( PETSC_COMM_WORLD,
                                      " number active elements for layer %s is %d\n",
                                      m_column.getLayer ( l )->getName ().c_str (),
                                      activeElementCount );
         }

      }

   }

   if ( verbose ) {
      // Print line for each MPI process.
      PetscSynchronizedPrintf ( PETSC_COMM_WORLD,
                                " There are %d active elements.\n",
                                m_activeElements.size ());
   }

}

//------------------------------------------------------------//

void ComputationalDomain::determineActiveNodes ( const bool verbose ) {

   const NodalGrid& nodeGrid = FastcauldronSimulator::getInstance ().getNodalGrid ();

   Vec activeNodesVec;

   DMCreateGlobalVector ( m_grids.getNodeGrid ( 1 ).getDa (), &activeNodesVec );
   VecZeroEntries ( activeNodesVec );

   PetscBlockVector<double> activeNodes;
   activeNodes.setVector ( m_grids.getNodeGrid ( 1 ), activeNodesVec, ADD_VALUES, true );

   m_activeNodes.fill ( false );

   // For all active elements indicate that the nodes are active.
   for ( size_t i = 0; i < m_activeElements.size (); ++i ) {
      const GeneralElement& element = *m_activeElements [ i ];

      for ( int n = 0; n < 8; ++n ) {
         activeNodes ( element.getNodeK ( n ), element.getNodeJ ( n ), element.getNodeI ( n )) = 1.0;
      }

   }

   // Gather all necessary information about active nodes from all processors.
   activeNodes.restoreVector ( UPDATE_INCLUDING_GHOSTS );
   activeNodes.setVector ( m_grids.getNodeGrid ( 1 ), activeNodesVec, INSERT_VALUES, true );

   int activeNodeCount = 0;
   // Only necessary in verbose mode.
   int inactiveNodes = 0;

   // Now that all the necessary information about active nodes has been gathered
   // it is now possible to set the node activity for the local nodes.
   for ( unsigned int i = nodeGrid.firstI (); i <= nodeGrid.lastI (); ++i ) {

      for ( unsigned int j = nodeGrid.firstJ (); j <= nodeGrid.lastJ (); ++j ) {
         bool localNode = NumericFunctions::inRange<int> ( static_cast<int>(i), nodeGrid.firstI (), nodeGrid.lastI ()) and
                          NumericFunctions::inRange<int> ( static_cast<int>(j), nodeGrid.firstJ (), nodeGrid.lastJ ());

         for ( unsigned int k = m_activeNodes.first ( 2 ); k <= m_activeNodes.last ( 2 ); ++k ) {

            if ( activeNodes ( k, j, i ) > 0.0 ) {
               m_activeNodes ( i, j, k ) = true;

               if ( localNode ) {
                  ++activeNodeCount;
               }

            } else if ( localNode ) {
               ++inactiveNodes;
            }

         }

      }

   }

   if ( verbose ) {
      std::stringstream buffer;

      buffer << " There are " << m_rank << ":  " << activeNodeCount << " active nodes, "  << inactiveNodes << " inactive nodes." << endl;
      PetscSynchronizedPrintf ( PETSC_COMM_WORLD, buffer.str ().c_str ());
      PetscSynchronizedFlush ( PETSC_COMM_WORLD, PETSC_STDOUT );
   }

   // Collect the number of active nodes from all processors.
   std::fill ( m_numberOfActiveNodesPerProcess.begin (), m_numberOfActiveNodesPerProcess.end (), 0 );
   m_numberOfActiveNodesPerProcess [ m_rank ] = activeNodeCount;

   // Check this, if it works then remove the two MPI function below.
   // MPI_Allgather ( &activeNodeCount, 1, MPI_INT, m_numberOfActiveNodesPerProcess.data (), 1, MPI_INT, PETSC_COMM_WORLD );

   MPI_Gather ( &activeNodeCount, 1, MPI_INT, m_numberOfActiveNodesPerProcess.data (), 1, MPI_INT, 0, PETSC_COMM_WORLD );
   MPI_Bcast ( m_numberOfActiveNodesPerProcess.data (), FastcauldronSimulator::getInstance ().getSize (), MPI_INT, 0, PETSC_COMM_WORLD );

   m_localStartDofNumber = 0;

   // Determine the first global dof number for local dofs by summing the number
   // of active number of nodes in all ranks less than the current rank.
   for ( int i = 0; i < m_rank; ++i ) {
      m_localStartDofNumber += m_numberOfActiveNodesPerProcess [ i ];
   }

   if ( verbose ) {
      std::stringstream buffer;

      buffer << " local start dof " << m_rank
             << ":  " << activeNodeCount << "  " << m_localStartDofNumber << "  "
             << std::accumulate( m_numberOfActiveNodesPerProcess.begin (), m_numberOfActiveNodesPerProcess.end (), 0 ) << "  "
             << ( nodeGrid.lastI () - nodeGrid.firstI () + 1 ) * ( nodeGrid.lastJ () - nodeGrid.firstJ () + 1 ) * m_activeNodes.length ( 2 ) << "  "
             << endl << flush;

      PetscSynchronizedPrintf ( PETSC_COMM_WORLD, buffer.str ().c_str ());
      PetscSynchronizedFlush ( PETSC_COMM_WORLD, PETSC_STDOUT );
   }

   activeNodes.restoreVector ( NO_UPDATE );
   VecDestroy ( &activeNodesVec );
}

//------------------------------------------------------------//

void ComputationalDomain::numberLocalToGlobalMapping () {

   if ( m_local2global != PETSC_IGNORE ) {
      ISLocalToGlobalMappingDestroy ( &m_local2global );
   }

   IntegerArray localInds ( getLocalNumberOfActiveNodes ());

   for ( size_t i = 0; i < localInds.size (); ++i ) {
      localInds [ i ] = m_localStartDofNumber + i;
   }

   ISLocalToGlobalMappingCreate ( PETSC_COMM_WORLD, 1, getLocalNumberOfActiveNodes (), localInds.data (), PETSC_COPY_VALUES, &m_local2global );
}

//------------------------------------------------------------//

void ComputationalDomain::resizeGrids ( const int newNodeCount ) {

   if ( newNodeCount <= 1 ) {
      return;
   }

   PetscBool isValid;

   VecValid ( m_globalDofNumbers, &isValid );

   if ( isValid ) {
      VecDestroy ( &m_globalDofNumbers );
   }

   DMCreateGlobalVector ( m_grids.getNodeGrid ( 1 ).getDa (), &m_globalDofNumbers );
   m_depthIndexNumbers.reallocate ( FastcauldronSimulator::getInstance ().getActivityOutputGrid (), newNodeCount );
   m_activeNodes.reallocate ( FastcauldronSimulator::getInstance ().getActivityOutputGrid (), newNodeCount );
}

//------------------------------------------------------------//
