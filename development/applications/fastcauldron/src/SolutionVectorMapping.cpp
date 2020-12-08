//
// Copyright (C) 2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "SolutionVectorMapping.h"

// Access to the utilities library.
#include "NumericFunctions.h"

// Access to fastcauldron application code.
#include "ComputationalDomain.h"
#include "FastcauldronSimulator.h"
#include "ConstantsFastcauldron.h"
#include "NodalVolumeGrid.h"
#include "PetscBlockVector.h"
#include "StratigraphicColumn.h"

// utilities library
#include "ConstantsMathematics.h"
using Utilities::Maths::MegaPaToPa;

//------------------------------------------------------------//

SolutionVectorMapping::SolutionVectorMapping ( ComputationalDomain&                        domain,
                                               const Basin_Modelling::Fundamental_Property property ) :
   m_computationalDomain ( domain ),
   m_property ( property ),
   m_scalingFactor ( property == Basin_Modelling::Overpressure ? MegaPaToPa : 1.0 ),
   m_layerMappingNumbers ( domain.getStratigraphicColumn ().getNumberOfLayers ())
{

   if ( property != Basin_Modelling::Temperature and property != Basin_Modelling::Overpressure ) {
      PetscPrintf ( PETSC_COMM_WORLD,
                    " Incorrect property: %s, should be either Temperature or Overpressure.\n",
                    Basin_Modelling::fundamentalPropertyImage ( property ).c_str ());
      exit ( 1 );
   }

   numberMapping ();
}

//------------------------------------------------------------//

void SolutionVectorMapping::allocateLayerMappings ( const size_t topLayerIndex,
                                                    size_t&      maxDepthDirection ) {

   const StratigraphicColumn& column = m_computationalDomain.getStratigraphicColumn ();
   size_t numberOfLayers = column.getNumberOfLayers ();

   maxDepthDirection = 0;

   // Make sure that the arrays are allocated and with the correct size.
   for ( size_t i = topLayerIndex; i < numberOfLayers; ++i ) {

      if ( m_layerMappingNumbers [ i ].isNull () or
           m_layerMappingNumbers [ i ].lastK () != column.getLayer ( i )->getMaximumNumberOfElements ()) {
         m_layerMappingNumbers [ i ].create ( column.getLayer ( i )->layerDA );
         m_layerMappingNumbers [ i ].fill ( 0 );
      }

      maxDepthDirection = NumericFunctions::Maximum<size_t> ( maxDepthDirection, static_cast<size_t>( m_layerMappingNumbers [ i ].lastK () + 1 ));
   }

}

//------------------------------------------------------------//

void SolutionVectorMapping::numberMapping () {

   if ( m_computationalDomain.getCurrentAge () < 0.0 ) {
      return;
   }

   const FastcauldronSimulator& fc = FastcauldronSimulator::getInstance ();
   const StratigraphicColumn& column = m_computationalDomain.getStratigraphicColumn ();

   size_t maxDepthDirection = 0;
   size_t numberOfLayers = column.getNumberOfLayers ();
   size_t topLayerIndex = column.getTopLayerIndex ( m_computationalDomain.getCurrentAge ());
   int localStartDof = m_computationalDomain.getLocalStartDof ();

   allocateLayerMappings ( topLayerIndex, maxDepthDirection );

   std::vector<int> depthIndices ( maxDepthDirection, 0 );
   int nodeCount = column.getNumberOfLogicalNodesInDepth ( m_computationalDomain.getCurrentAge ()) - 1;
   const NodalVolumeGrid&  scalarNodeGrid = m_computationalDomain.getStratigraphicGrids ().getNodeGrid ();
   const LocalIntegerArray3D& depthIndex = m_computationalDomain.getDepthIndices ();

   PetscBlockVector<double> dof;

   // Need get only dof number for this process.
   dof.setVector ( scalarNodeGrid, m_computationalDomain.getDofVector (), INSERT_VALUES, true );

   for ( size_t l = topLayerIndex; l < numberOfLayers ; ++l ) {
      Integer3DArray& layerMappingNumbers = m_layerMappingNumbers [ l ];

      const ComputationalDomain::FormationGeneralElementGrid* formationGrid =
                     m_computationalDomain.getFormationGrid ( column.getLayer ( l ));

      size_t numberOfLayerNodes = static_cast<size_t>(formationGrid->getFormation ().getMaximumNumberOfElements ()) + 1;

      // Loop over the number of nodes in the depth direction (number of elements + 1)
      for ( size_t j = numberOfLayerNodes; j >= 1; --j ) {
         depthIndices [ j - 1 ] = nodeCount--;
      }

      // The node at the top of the layer below has the same depth-index as the bottom node of this layer.
      ++nodeCount;

      // Loop over the nodes that are local to the rank.
      for ( unsigned int i = scalarNodeGrid.firstI (); i <= scalarNodeGrid.lastI (); ++i ) {

         for ( unsigned int j = scalarNodeGrid.firstJ (); j <= scalarNodeGrid.lastJ (); ++j ) {

            if ( fc.nodeIsDefined ( i, j )) {

               for ( size_t k = 0; k < numberOfLayerNodes; ++k ) {

                  if ( dof ( depthIndex ( i, j, depthIndices [ k ] ), j, i ) != ComputationalDomain::NullDofNumber ) {
                     layerMappingNumbers ( i, j, k ) = dof ( depthIndex ( i, j, depthIndices [ k ] ), j, i ) - localStartDof;
                  } else {
                     layerMappingNumbers ( i, j, k ) = ComputationalDomain::NullDofNumber;
                  }

               }

            }

         }

      }

   }

   dof.restoreVector ( NO_UPDATE );
}

//------------------------------------------------------------//

void SolutionVectorMapping::putSolution ( const Vec vector ) const {

   const FastcauldronSimulator& fc = FastcauldronSimulator::getInstance ();
   const StratigraphicColumn& column = m_computationalDomain.getStratigraphicColumn ();

   size_t topLayerIndex = column.getTopLayerIndex ( m_computationalDomain.getCurrentAge ());
   size_t numberOfLayers = column.getNumberOfLayers ();
   const NodalVolumeGrid&  scalarNodeGrid = m_computationalDomain.getStratigraphicGrids ().getNodeGrid ();

   double* localArray;

   VecGetArray ( vector, &localArray );

   for ( size_t l = topLayerIndex; l < numberOfLayers ; ++l ) {
      ComputationalDomain::FormationGeneralElementGrid* formationGrid = m_computationalDomain.getFormationGrid ( column.getLayer ( l ));
      const Integer3DArray& layerMappingNumbers = m_layerMappingNumbers [ l ];

      PetscBlockVector<double> layerProperty;
      layerProperty.setVector ( formationGrid->getFormation ().getNodalVolumeGrid ( 1 ),
                                formationGrid->getFormation ().Current_Properties ( m_property ),
                                INSERT_VALUES );


      // Loop over the nodes that are local to the rank.
      for ( unsigned int i = scalarNodeGrid.firstI (); i <= scalarNodeGrid.lastI (); ++i ) {

         for ( unsigned int j = scalarNodeGrid.firstJ (); j <= scalarNodeGrid.lastJ (); ++j ) {

            if ( fc.nodeIsDefined ( i, j )) {

               for ( unsigned int k = 0; k <= formationGrid->getFormation ().getMaximumNumberOfElements (); ++k ) {

                  if ( layerMappingNumbers ( i, j, k ) != ComputationalDomain::NullDofNumber ) {
                     layerProperty ( k, j, i ) = localArray [ layerMappingNumbers ( i, j, k )] / m_scalingFactor;
                  }

               }

            }

         }

      }

      layerProperty.restoreVector ( UPDATE_EXCLUDING_GHOSTS );
   }

   VecRestoreArray ( vector, &localArray );
}

//------------------------------------------------------------//

void SolutionVectorMapping::getSolution ( Vec vector ) const {

   const FastcauldronSimulator& fc = FastcauldronSimulator::getInstance ();
   const StratigraphicColumn& column = m_computationalDomain.getStratigraphicColumn ();

   size_t topLayerIndex = column.getTopLayerIndex ( m_computationalDomain.getCurrentAge ());
   size_t numberOfLayers = column.getNumberOfLayers ();
   const NodalVolumeGrid&  scalarNodeGrid = m_computationalDomain.getStratigraphicGrids ().getNodeGrid ();

   double* localArray;

   VecGetArray ( vector, &localArray );

   for ( size_t l = topLayerIndex; l < numberOfLayers ; ++l ) {
      ComputationalDomain::FormationGeneralElementGrid* formationGrid = m_computationalDomain.getFormationGrid ( column.getLayer ( l ));
      const Integer3DArray& layerMappingNumbers = m_layerMappingNumbers [ l ];

      PetscBlockVector<double> layerProperty;
      layerProperty.setVector ( formationGrid->getFormation ().getNodalVolumeGrid ( 1 ),
                                formationGrid->getFormation ().Current_Properties ( m_property ),
                                INSERT_VALUES );

      // Loop over the nodes that are local to the rank.
      for ( unsigned int i = scalarNodeGrid.firstI (); i <= scalarNodeGrid.lastI (); ++i ) {

         for ( unsigned int j = scalarNodeGrid.firstJ (); j <= scalarNodeGrid.lastJ (); ++j ) {

            if ( fc.nodeIsDefined ( i, j )) {

               for ( unsigned int k = 0; k <= formationGrid->getFormation ().getMaximumNumberOfElements (); ++k ) {

                  if ( layerMappingNumbers ( i, j, k ) != ComputationalDomain::NullDofNumber ) {
                     localArray [ layerMappingNumbers ( i, j, k )] = layerProperty ( k, j, i ) * m_scalingFactor;
                  }

               }

            }

         }

      }

      layerProperty.restoreVector ( NO_UPDATE );
   }

   VecRestoreArray ( vector, &localArray );
}


std::vector<int> SolutionVectorMapping::getLocationMaxValue ( Vec vector, double& maxResidual ) const {

   const FastcauldronSimulator& fc = FastcauldronSimulator::getInstance ();
   const StratigraphicColumn& column = m_computationalDomain.getStratigraphicColumn ();

   size_t topLayerIndex = column.getTopLayerIndex ( m_computationalDomain.getCurrentAge ());
   size_t numberOfLayers = column.getNumberOfLayers ();
   const NodalVolumeGrid&  scalarNodeGrid = m_computationalDomain.getStratigraphicGrids ().getNodeGrid ();

   double* localArray;

   VecGetArray ( vector, &localArray );

   int maxI = -1;
   int maxJ = -1;
   int maxK = -1;
   int maxL = -1;

   for ( size_t l = topLayerIndex; l < numberOfLayers ; ++l )
   {
     ComputationalDomain::FormationGeneralElementGrid* formationGrid = m_computationalDomain.getFormationGrid ( column.getLayer ( l ));
     const Integer3DArray& layerMappingNumbers = m_layerMappingNumbers [ l ];

     // Loop over the nodes that are local to the rank.
     for ( unsigned int i = scalarNodeGrid.firstI (); i <= scalarNodeGrid.lastI (); ++i )
     {
       for ( unsigned int j = scalarNodeGrid.firstJ (); j <= scalarNodeGrid.lastJ (); ++j )
       {
         if ( fc.nodeIsDefined ( i, j ))
         {
           for ( unsigned int k = 0; k <= formationGrid->getFormation ().getMaximumNumberOfElements (); ++k )
           {
             if ( layerMappingNumbers ( i, j, k ) != ComputationalDomain::NullDofNumber )
             {
               const double absVal = std::fabs(localArray [ layerMappingNumbers ( i, j, k )]);
               if (absVal > maxResidual)
               {
                 maxI = i;
                 maxJ = j;
                 maxK = k;
                 maxL = l;
                 maxResidual = absVal;
               }
             }
           }
         }
       }
     }
   }

   VecRestoreArray ( vector, &localArray );

   return {maxI, maxJ, maxK, maxL};
}


//------------------------------------------------------------//
