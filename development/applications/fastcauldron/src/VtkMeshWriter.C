//
// Copyright (C) 2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "VtkMeshWriter.h"

#include <sstream>
#include <algorithm>
#include <numeric>

#include "FastcauldronSimulator.h"
#include "ConstantsFastcauldron.h"

VtkMeshWriter::VtkMeshWriter () :
   m_size ( FastcauldronSimulator::getInstance ().getSize ()),
   m_rank ( FastcauldronSimulator::getInstance ().getRank ())
{
}


void VtkMeshWriter::save ( const ComputationalDomain& domain,
                           const std::string&         fileName,
                           const double               zScale,
                           const bool                 useProjectOrigin ) const {

   IntegerArray numberOfActiveNodes;
   IntegerArray numberOfActiveElements;

   getNumberOfActiveNodesAndElements ( domain, numberOfActiveNodes, numberOfActiveElements );

   int totalNumberOfNodes = std::accumulate ( numberOfActiveNodes.begin (), numberOfActiveNodes.end (), 0 );
   int totalNumberOfElements = std::accumulate ( numberOfActiveElements.begin (), numberOfActiveElements.end (), 0 );

   DoubleArray  localNodes ( ValuesPerNode * numberOfActiveNodes [ m_rank ]);
   IntegerArray localElementDofs ( NumberOfElementNodes * numberOfActiveElements [ m_rank ]);
   IntegerArray localElementLayerIds ( numberOfActiveElements [ m_rank ]);

   DoubleArray  allNodes;
   IntegerArray allElementDofs;
   IntegerArray allElementLayerIds;

   if ( m_rank == 0 ) {
      allNodes.resize ( ValuesPerNode * totalNumberOfNodes );
      allElementDofs.resize ( NumberOfElementNodes * totalNumberOfElements );
      allElementLayerIds.resize ( totalNumberOfElements );
   }

   getLocalNodes ( domain, localNodes, zScale, useProjectOrigin );
   gatherAllNodes ( localNodes, numberOfActiveNodes, allNodes );

   getLocalElementDofs ( domain, localElementDofs );
   gatherAllElementDofs ( localElementDofs, numberOfActiveElements, allElementDofs );

   getLocalElementLayerIds ( domain, localElementLayerIds );
   gatherAllElementLayerIds ( localElementLayerIds, numberOfActiveElements, allElementLayerIds );

   writeFile ( fileName, totalNumberOfNodes, allNodes, totalNumberOfElements, allElementDofs, allElementLayerIds );
}

void VtkMeshWriter::writeFile ( const std::string&  fileName,
                                const int           numberOfNodes,
                                const DoubleArray&  allNodes,
                                const int           numberOfElements,
                                const IntegerArray& allElementDofs,
                                const IntegerArray& allElementLayerIds ) const {

   if ( m_rank == 0 ) {
      std::ofstream file ( fileName.c_str ());
      int count;

      // Print header
      file << "# vtk DataFile Version 1.0"  << std::endl;
      file << std::endl;
      file << "ASCII" << std::endl;
      file << std::endl;
      file << "DATASET UNSTRUCTURED_GRID" << std::endl;

      // First add vertices.
      file << "POINTS " << numberOfNodes << " float" << std::endl;

      for ( int i = 0; i < numberOfNodes * ValuesPerNode; i += ValuesPerNode ) {
         file << allNodes [ i ] << "  " << allNodes [ i + 1 ] << "  " << allNodes [ i + 2 ] << endl;
      }

      file << std::endl;

      // now element data
      file << "CELLS " << numberOfElements << "  " << numberOfElements * ( NumberOfElementNodes + 1 ) << std::endl;

      count = 0;

      for ( int i = 0; i < numberOfElements; ++i ) {
         file << " " << NumberOfElementNodes;

         for ( int j = 0; j < NumberOfElementNodes; ++j, ++count ) {
            file  << "  " << allElementDofs [ count ];
         }

         file << endl;
      }

      file << std::endl;

      // Now add element type data.
      file << "CELL_TYPES " << numberOfElements << std::endl;

      for ( int i = 0; i < numberOfElements; ++i ) {
         // 12 is the general hexahedral element shape.
         file << " 12" << endl;
      }

      file << std::endl;

      // Now add element type data.
      file << "CELL_DATA " << numberOfElements << std::endl;
      file << "SCALARS layer_id int 1 " << std::endl;
      file << "LOOKUP_TABLE default " << std::endl;

      for ( int i = 0; i < numberOfElements; ++i ) {
         file << ' ' << allElementLayerIds [ i ] << endl;
      }

      file << std::endl;
      file.close ();
   }

}


void VtkMeshWriter::getNumberOfActiveNodesAndElements ( const ComputationalDomain& domain,
                                                        IntegerArray&              numberOfActiveNodes,
                                                        IntegerArray&              numberOfActiveElements ) const {

   numberOfActiveNodes.clear ();
   numberOfActiveNodes.resize ( m_size, 0 );

   numberOfActiveElements.clear ();
   numberOfActiveElements.resize ( m_size, 0 );

   int value;

   value = domain.getLocalNumberOfActiveNodes ();
   MPI_Allgather ( &value, 1, MPI_INT, numberOfActiveNodes.data (), 1, MPI_INT, PETSC_COMM_WORLD );

   value = domain.getLocalNumberOfActiveElements ();
   MPI_Allgather ( &value, 1, MPI_INT, numberOfActiveElements.data (), 1, MPI_INT, PETSC_COMM_WORLD );

}

void VtkMeshWriter::getLocalNodes ( const ComputationalDomain& domain,
                                    DoubleArray&               activeNodes,
                                    const double               zScale,
                                    const bool                 useProjectOrigin ) const {

   switch ( domain.getDofOrdering ()) {

      case ComputationalDomain::IJKOrder :
         getLocalNodesIJK ( domain, activeNodes, zScale, useProjectOrigin );
         break;

      case ComputationalDomain::KIJOrder :
         getLocalNodesKIJ ( domain, activeNodes, zScale, useProjectOrigin );
         break;

      case ComputationalDomain::KJIOrder :
         getLocalNodesKJI ( domain, activeNodes, zScale, useProjectOrigin );
         break;

      default :
         PetscPrintf ( PETSC_COMM_WORLD, " Basin_Error: dof ordering incorrectly defined.\n" );
         exit ( 1 );
   }

}


void VtkMeshWriter::getLocalNodesIJK ( const ComputationalDomain& domain,
                                       DoubleArray&               activeNodes,
                                       const double               zScale,
                                       const bool                 useProjectOrigin ) const {

   const FastcauldronSimulator& fc = FastcauldronSimulator::getInstance ();
   const StratigraphicColumn& stratigraphicColumn = domain.getStratigraphicColumn ();

   double deltaX = fc.getCauldronGridDescription ().deltaI;
   double deltaY = fc.getCauldronGridDescription ().deltaJ;

   double originX = fc.firstI () * deltaX + ( useProjectOrigin ? fc.getCauldronGridDescription ().originI : 0.0 );
   double originY = fc.firstJ () * deltaY + ( useProjectOrigin ? fc.getCauldronGridDescription ().originJ : 0.0 );

   double x;
   double y;
   double z;
   int count = 0;
   int numberOfNodesInDepth = stratigraphicColumn.getNumberOfLogicalNodesInDepth ( domain.getCurrentAge ());
   size_t firstLayerIndex = stratigraphicColumn.getTopLayerIndex ( domain.getCurrentAge ());
   int globalK;

   vector<PETSC_3D_Array> layerDepths ( stratigraphicColumn.getNumberOfLayers ());

   for ( size_t l = firstLayerIndex; l < stratigraphicColumn.getNumberOfLayers (); ++l ) {
      layerDepths [ l ].Set_Global_Array ( stratigraphicColumn.getLayer ( l )->layerDA,
                                           stratigraphicColumn.getLayer ( l )->Current_Properties ( Basin_Modelling::Depth ));
   }

   x = originX;

   for ( size_t i = fc.firstI (); i <= fc.lastI (); ++i ) {
      y = originY;

      for ( size_t j = fc.firstJ (); j <= fc.lastJ (); ++j ) {

         if ( fc.nodeIsDefined ( i, j )) {

            globalK = 0;

            ///1. Loop over the elements, so top nodes are excluded
            // size_t must not be 0 so we use stratigraphicColumn.getNumberOfLayers () instead of stratigraphicColumn.getNumberOfLayers ()-1
            // and firstLayerIndex+1 instead of firstLayerIndex, so we access l-1
            for ( size_t l = stratigraphicColumn.getNumberOfLayers (); l >= firstLayerIndex+1; --l ) {
               const FormationElementGrid<GeneralElement>& grid = *domain.getFormationGrid ( stratigraphicColumn.getLayer ( l-1 ));
               const PETSC_3D_Array& layerDepth = layerDepths [ l-1 ];

               for ( int k = grid.firstK (); k <= grid.lastK (); ++k, ++globalK ) {
                  z = layerDepth ( k, j, i );

                  if ( domain.getActiveNodes ()( i, j, globalK )) {
                     activeNodes [ count++ ] = x;
                     activeNodes [ count++ ] = y;
                     activeNodes [ count++ ] = zScale * z;
                  }

               }

            }

            ///2. Top nodes numbering
            z = fc.getSeaBottomDepth ( i, j, domain.getCurrentAge ());

            if ( domain.getActiveNodes ()( i, j, numberOfNodesInDepth - 1 )) {
               activeNodes [ count++ ] = x;
               activeNodes [ count++ ] = y;
               activeNodes [ count++ ] = zScale * z;
            }

         }

         y += deltaY;
      }

      x += deltaX;
   }

   ///3. Restore array
   for ( size_t l = firstLayerIndex; l < stratigraphicColumn.getNumberOfLayers (); ++l ) {
      layerDepths [ l ].Restore_Global_Array ( No_Update );
   }

} // end getLocalNodesIJK


void VtkMeshWriter::getLocalNodesKIJ ( const ComputationalDomain& domain,
                                       DoubleArray&               activeNodes,
                                       const double               zScale,
                                       const bool                 useProjectOrigin ) const {

   const FastcauldronSimulator& fc = FastcauldronSimulator::getInstance ();
   const StratigraphicColumn& stratigraphicColumn = domain.getStratigraphicColumn ();

   double deltaX = fc.getCauldronGridDescription ().deltaI;
   double deltaY = fc.getCauldronGridDescription ().deltaJ;

   double originX = fc.firstI () * deltaX + ( useProjectOrigin ? fc.getCauldronGridDescription ().originI : 0.0 );
   double originY = fc.firstJ () * deltaY + ( useProjectOrigin ? fc.getCauldronGridDescription ().originJ : 0.0 );

   double x;
   double y;
   double z;
   int count = 0;
   int numberOfNodesInDepth = stratigraphicColumn.getNumberOfLogicalNodesInDepth ( domain.getCurrentAge ());
   size_t firstLayerIndex = stratigraphicColumn.getTopLayerIndex ( domain.getCurrentAge ());
   int globalK;

   vector<PETSC_3D_Array> layerDepths ( stratigraphicColumn.getNumberOfLayers ());

   for ( size_t l = firstLayerIndex; l < stratigraphicColumn.getNumberOfLayers (); ++l ) {
      layerDepths [ l ].Set_Global_Array ( stratigraphicColumn.getLayer ( l )->layerDA,
                                           stratigraphicColumn.getLayer ( l )->Current_Properties ( Basin_Modelling::Depth ));
   }

   ///1. Loop over the elements, so top nodes are excluded
   // size_t must not be 0 so we use stratigraphicColumn.getNumberOfLayers () instead of stratigraphicColumn.getNumberOfLayers ()-1
   // and firstLayerIndex+1 instead of firstLayerIndex, so we access l-1
   globalK = 0;
   for ( size_t l = stratigraphicColumn.getNumberOfLayers (); l >= firstLayerIndex+1; --l ) {
      const FormationElementGrid<GeneralElement>& grid = *domain.getFormationGrid ( stratigraphicColumn.getLayer ( l-1 ));
      const PETSC_3D_Array& layerDepth = layerDepths [ l-1 ];

      for ( int k = grid.firstK (); k <= grid.lastK (); ++k, ++globalK ) {
         x = originX;

         for ( size_t i = fc.firstI (); i <= fc.lastI (); ++i ) {
            y = originY;

            for ( size_t j = fc.firstJ (); j <= fc.lastJ (); ++j ) {

               if ( fc.nodeIsDefined ( i, j )) {
                  z = layerDepth ( k, j, i );

                  if ( domain.getActiveNodes ()( i, j, globalK )) {
                     activeNodes [ count++ ] = x;
                     activeNodes [ count++ ] = y;
                     activeNodes [ count++ ] = zScale * z;
                  }

               }

            }

            y += deltaY;
         }

         x += deltaX;
      }

   }

   ///2. Top nodes numbering
   x = originX;

   for ( size_t i = fc.firstI (); i <= fc.lastI (); ++i ) {
      y = originY;

      for ( size_t j = fc.firstJ (); j <= fc.lastJ (); ++j ) {

         if ( fc.nodeIsDefined ( i, j )) {
            z = fc.getSeaBottomDepth ( i, j, domain.getCurrentAge ());

            if ( domain.getActiveNodes ()( i, j, numberOfNodesInDepth - 1 )) {
               activeNodes [ count++ ] = x;
               activeNodes [ count++ ] = y;
               activeNodes [ count++ ] = zScale * z;
            }

         }

         y += deltaY;
      }

      x += deltaX;
   }

   ///3. Restore array
   for ( size_t l = firstLayerIndex; l < stratigraphicColumn.getNumberOfLayers (); ++l ) {
      layerDepths [ l ].Restore_Global_Array ( No_Update );
   }

} // end getLocalNodesKIJ

void VtkMeshWriter::getLocalNodesKJI ( const ComputationalDomain& domain,
                                       DoubleArray&               activeNodes,
                                       const double               zScale,
                                       const bool                 useProjectOrigin ) const {

   const FastcauldronSimulator& fc = FastcauldronSimulator::getInstance ();
   const StratigraphicColumn& stratigraphicColumn = domain.getStratigraphicColumn ();

   double deltaX = fc.getCauldronGridDescription ().deltaI;
   double deltaY = fc.getCauldronGridDescription ().deltaJ;

   double originX = fc.firstI () * deltaX + ( useProjectOrigin ? fc.getCauldronGridDescription ().originI : 0.0 );
   double originY = fc.firstJ () * deltaY + ( useProjectOrigin ? fc.getCauldronGridDescription ().originJ : 0.0 );

   double x;
   double y;
   double z;
   int count = 0;
   int numberOfNodesInDepth = stratigraphicColumn.getNumberOfLogicalNodesInDepth ( domain.getCurrentAge ());
   size_t firstLayerIndex = stratigraphicColumn.getTopLayerIndex ( domain.getCurrentAge ());
   int globalK;

   vector<PETSC_3D_Array> layerDepths ( stratigraphicColumn.getNumberOfLayers ());

   for ( size_t l = firstLayerIndex; l < stratigraphicColumn.getNumberOfLayers (); ++l ) {
      layerDepths [ l ].Set_Global_Array ( stratigraphicColumn.getLayer ( l )->layerDA,
                                           stratigraphicColumn.getLayer ( l )->Current_Properties ( Basin_Modelling::Depth ));
   }

   globalK = 0;

   ///1. Loop over the elements, so top nodes are excluded
   // size_t must not be 0 so we use stratigraphicColumn.getNumberOfLayers () instead of stratigraphicColumn.getNumberOfLayers ()-1
   // and firstLayerIndex+1 instead of firstLayerIndex, so we access l-1
   for ( size_t l = stratigraphicColumn.getNumberOfLayers (); l >= firstLayerIndex+1; --l ) {
      const FormationElementGrid<GeneralElement>& grid = *domain.getFormationGrid ( stratigraphicColumn.getLayer ( l-1 ));
      const PETSC_3D_Array& layerDepth = layerDepths [ l-1 ];

      for ( int k = grid.firstK (); k <= grid.lastK (); ++k, ++globalK ) {
         y = originY;

         for ( size_t j = fc.firstJ (); j <= fc.lastJ (); ++j ) {
            x = originX;

            for ( size_t i = fc.firstI (); i <= fc.lastI (); ++i ) {

               if ( fc.nodeIsDefined ( i, j )) {
                  z = layerDepth ( k, j, i );

                  if ( domain.getActiveNodes ()( i, j, globalK )) {
                     activeNodes [ count++ ] = x;
                     activeNodes [ count++ ] = y;
                     activeNodes [ count++ ] = zScale * z;
                  }

               }

               x += deltaX;
            }

            y += deltaY;
         }

      }

   }

   ///2. Top nodes numbering
   y = originY;

   for ( size_t j = fc.firstJ (); j <= fc.lastJ (); ++j ) {
      x = originX;

      for ( size_t i = fc.firstI (); i <= fc.lastI (); ++i ) {

         if ( fc.nodeIsDefined ( i, j )) {
            z = fc.getSeaBottomDepth ( i, j, domain.getCurrentAge ());

            if ( domain.getActiveNodes ()( i, j, numberOfNodesInDepth - 1 )) {
               activeNodes [ count++ ] = x;
               activeNodes [ count++ ] = y;
               activeNodes [ count++ ] = zScale * z;
            }

         }

         x += deltaX;
      }

      y += deltaY;
   }

   ///3. Restore array
   for ( size_t l = firstLayerIndex; l < stratigraphicColumn.getNumberOfLayers (); ++l ) {
      layerDepths [ l ].Restore_Global_Array ( No_Update );
   }

} // end getLocalNodesKJI


void VtkMeshWriter::getLocalElementDofs ( const ComputationalDomain& domain,
                                          IntegerArray&              elementDofs ) const {

   int count = 0;

   for ( int i = 0; i < domain.getLocalNumberOfActiveElements (); ++i ) {
      const GeneralElement& element = domain.getActiveElement ( i );

      for ( int j = 0; j < NumberOfElementNodes; ++j, ++count ) {
         elementDofs [ count ] = element.getDof ( j );
      }

   }

}

void VtkMeshWriter::getLocalElementLayerIds ( const ComputationalDomain& domain,
                                              IntegerArray&              localElementLayerIds ) const {

   const StratigraphicColumn& stratigraphicColumn = domain.getStratigraphicColumn ();
   size_t nullValue = StratigraphicColumn::NullIndexValue;

   for ( int i = 0; i < domain.getLocalNumberOfActiveElements (); ++i ) {
      const GeneralElement& element = domain.getActiveElement ( i );
      size_t value = stratigraphicColumn.getLayerIndex ( element.getLayerElement ().getFormation ());

      if ( value != nullValue ) {
         localElementLayerIds [ i ] = static_cast<int>(value);
      } else {
         localElementLayerIds [ i ] = -1;
      }

   }


}


void VtkMeshWriter::gatherAllNodes ( const DoubleArray&    localNodes,
                                     const IntegerArray&   numberOfActiveNodes,
                                     DoubleArray&          globalNodes ) const {

   IntegerArray numberOfActiveNodeValues ( numberOfActiveNodes.size ());
   IntegerArray nodeOffset ( numberOfActiveNodes.size ());
   int offset = 0;

   for ( size_t i = 0; i < numberOfActiveNodes.size (); ++i ) {
      numberOfActiveNodeValues [ i ] = ValuesPerNode * numberOfActiveNodes [ i ];
      nodeOffset [ i ] = offset;
      offset += numberOfActiveNodeValues [ i ];
   }

   // The const_cast is okay here because MPI_Gatherv does not modify the array.
   MPI_Gatherv ( const_cast<double*>(localNodes.data ()), ValuesPerNode * numberOfActiveNodes [ m_rank ], MPI_DOUBLE,
                 globalNodes.data (), numberOfActiveNodeValues.data (), nodeOffset.data (), MPI_DOUBLE, 0, PETSC_COMM_WORLD );

}

void VtkMeshWriter::gatherAllElementDofs ( const IntegerArray& localElementDofs,
                                           const IntegerArray& numberOfActiveElements,
                                           IntegerArray&       globalElementDofs ) const {

   IntegerArray numberOfActiveElementDofValues ( numberOfActiveElements.size ());
   IntegerArray elementDofOffset ( numberOfActiveElements.size ());
   int offset = 0;

   for ( size_t i = 0; i < numberOfActiveElements.size (); ++i ) {
      numberOfActiveElementDofValues [ i ] = NumberOfElementNodes * numberOfActiveElements [ i ];
      elementDofOffset [ i ] = offset;
      offset += numberOfActiveElementDofValues [ i ];
   }

   // The const_cast is okay here because MPI_Gatherv does not modify the array.
   MPI_Gatherv ( const_cast<int*>(localElementDofs.data ()), NumberOfElementNodes * numberOfActiveElements [ m_rank ], MPI_INT,
                 globalElementDofs.data (), numberOfActiveElementDofValues.data (), elementDofOffset.data (), MPI_INT, 0, PETSC_COMM_WORLD );

}

void VtkMeshWriter::gatherAllElementLayerIds ( const IntegerArray& localElementLayerIds,
                                               const IntegerArray& numberOfActiveElements,
                                               IntegerArray&       globalElementLayerIds ) const {

   IntegerArray elementIdOffset ( numberOfActiveElements.size ());
   int offset = 0;

   for ( size_t i = 0; i < numberOfActiveElements.size (); ++i ) {
      elementIdOffset [ i ] = offset;
      offset += numberOfActiveElements [ i ];
   }

   // The const_cast is okay here because MPI_Gatherv does not modify the array.
   MPI_Gatherv ( const_cast<int*>(localElementLayerIds.data ()), numberOfActiveElements [ m_rank ], MPI_INT,
                 globalElementLayerIds.data (), const_cast<int*>(numberOfActiveElements.data ()), elementIdOffset.data (), MPI_INT, 0, PETSC_COMM_WORLD );

}
