#include "Subdomain.h"

#include <sstream>

#include "Interface/Interface.h"
#include "Interface/Formation.h"
#include "Interface/Snapshot.h"
#include "Interface/Surface.h"

#include "SourceRock.h"

#include "NumericFunctions.h"

#include "FastcauldronSimulator.h"
#include "MultiComponentFlowHandler.h"
#include "propinterface.h"

#include "globaldefs.h"

Subdomain::Subdomain ( const LayerProps& topLayer,
                       const LayerProps& bottomLayer ) :
   m_isActive ( false ),
   m_sourceRockIsActive ( false )
{

   const AppCtx* cauldron = FastcauldronSimulator::getInstance ().getCauldron ();

   int topIndex    = cauldron->getLayerIndex ( topLayer.getName ());
   int bottomIndex = cauldron->getLayerIndex ( bottomLayer.getName ());
   int i;

   for ( i = topIndex; i <= bottomIndex; ++i ) {
      m_layers.push_back ( cauldron->layers [ i ]);
      m_layerMap [ cauldron->layers [ i ]] = new FormationSubdomainElementGrid ( *cauldron->layers [ i ]);
   }

   linkSubdomainElementsVertically ();

   int elementCount = numberOfElements ();

   m_isActive = m_layers.size () > 0 and elementCount > 0;

   m_containsSulphurSourceRock = determineContainsSulphur ();

   if ( m_isActive ) {
      m_sourceRockIsActive = determineSourceRockActivity ();
      createVolumeGrid ( 1 );
      createNodeGrid ( 1 );
      initialise ( elementCount );
   }

   m_scalarDofNumbers = PETSC_NULL;
   numberElements ();
   initialiseGlobalKToLayerMapping ();

   if ( FastcauldronSimulator::getInstance ().getMcfHandler ().getDebugLevel () > 0 ) { // and FastcauldronSimulator::getInstance ().getRank () == 0 ) {

      const ElementGrid& elementGrid = FastcauldronSimulator::getInstance ().getElementGrid ();

      std::stringstream buffer;

      buffer << " Subdomain:" << endl
             << "     layer range: {" << topLayer.layername << ", " << bottomLayer.layername << "}" << endl;
      buffer << "     elements: " << setw ( 4 ) << maximumNumberOfElements () << std::endl;
      buffer << "     delta-x, -y: " 
             << setw ( 12 ) << FastcauldronSimulator::getInstance ().getCauldronGridDescription ().deltaI << "  " 
             << setw ( 12 ) << FastcauldronSimulator::getInstance ().getCauldronGridDescription ().deltaJ << "  " 
             << std::endl;
      

      PetscPrintf ( PETSC_COMM_WORLD, buffer.str ().c_str ());

      buffer.str ( "" );

      buffer << " Subdomain range: " 
             << " rank "  << setw ( 2 ) << FastcauldronSimulator::getInstance ().getRank () << ", "
             << "non-ghost elements => " 
             << "{{ " << setw ( 4 ) << elementGrid.firstI () << ", " << setw ( 4 ) << elementGrid.lastI () << " }, "
             <<  "{ " << setw ( 4 ) << elementGrid.firstJ () << ", " << setw ( 4 ) << elementGrid.lastJ () << " }}, "
             << "ghost elements => " 
             << "{{ " << setw ( 4 ) << elementGrid.firstI ( true ) << ", " << setw ( 4 ) << elementGrid.lastI ( true ) << " }, "
             <<  "{ " << setw ( 4 ) << elementGrid.firstJ ( true ) << ", " << setw ( 4 ) << elementGrid.lastJ ( true ) << " }}"
             << std::endl;

      PetscSynchronizedPrintf ( PETSC_COMM_WORLD, buffer.str ().c_str ());
      PetscSynchronizedFlush ( PETSC_COMM_WORLD );

   }

   m_id = -1;
}

//------------------------------------------------------------//

Subdomain::Subdomain ( const LayerProps& theLayer ) {

   const AppCtx* cauldron = FastcauldronSimulator::getInstance ().getCauldron ();

   int layerIndex = cauldron->getLayerIndex ( theLayer.getName ());


   m_layers.push_back ( cauldron->layers [ layerIndex ]);
   // m_layers.push_back ( cauldron->layers [ cauldron->Find_Layer_Index ( theLayer.depoage )]);
   m_layerMap [ m_layers [ 0 ]] = new FormationSubdomainElementGrid ( *m_layers [ 0 ]);

   int elementCount = numberOfElements ();
   m_isActive = m_layers.size () > 0 and elementCount > 0;

   if ( m_isActive ) {
      m_sourceRockIsActive = determineSourceRockActivity ();
      createVolumeGrid ( 1 );
      createNodeGrid ( 1 );
      initialise ( elementCount );
   } else {
      m_sourceRockIsActive = false;
   }

   m_scalarDofNumbers = PETSC_NULL;
   numberElements ();
   initialiseGlobalKToLayerMapping ();
   m_id = -1;
}

//------------------------------------------------------------//

Subdomain::~Subdomain () {

   FormationToElementGridMap::iterator it;
   size_t i;

   for ( i = 0; i < m_elementVolumeGrids.size (); ++i ) {

      if ( m_elementVolumeGrids [ i ] != 0 ) {
         delete m_elementVolumeGrids [ i ];
      }

   }

   for ( i = 0; i < m_nodalGrids.size (); ++i ) {

      if ( m_nodalGrids [ i ] != 0 ) {
         delete m_nodalGrids [ i ];
      }

   }

   for ( it = m_layerMap.begin (); it != m_layerMap.end (); ++it ) {
      delete it->m_formationGrid;
   }

}

//------------------------------------------------------------//

void Subdomain::linkSubdomainElementsVertically () {

   const ElementGrid& elementGrid = FastcauldronSimulator::getInstance ().getElementGrid ();

   LayerIterator                  iter;
   FormationSubdomainElementGrid* above;
   FormationSubdomainElementGrid* below;
   int i;
   int j;
   int belowSize;

   initialiseLayerIterator ( iter );
   above = &(*iter);
   ++iter;

   while ( not iter.isDone ()) {
      below = &(*iter);
      belowSize = below->lastK ();

      for ( i = elementGrid.firstI (); i <= elementGrid.lastI (); ++i ) {

         for ( j = elementGrid.firstJ (); j <= elementGrid.lastJ (); ++j ) {
            (*above)( i, j, 0 ).setNeighbour ( VolumeData::DeepFace, &(*below)( i, j, belowSize ));
            (*below)( i, j, belowSize ).setNeighbour ( VolumeData::ShallowFace, &(*above)( i, j, 0 ));
         }

      }

      above = below;
      ++iter;
   }

}

//------------------------------------------------------------//

void Subdomain::initialise ( const int numberOfElements ) {

   const ElementGrid& elementGrid = FastcauldronSimulator::getInstance ().getElementGrid ();

   unsigned int i;
   unsigned int j;
   unsigned int k;
   int subdomainK;
   int l;
   int subdomainStartK;

   subdomainStartK = 0;

   // Set subdomain-element with the layer-element.
   for ( l = 0; l < m_layers.size (); ++l ) {

      for ( k = 0, subdomainK = subdomainStartK; k < m_layers [ l ]->getMaximumNumberOfElements (); ++k, ++subdomainK ) {

         for ( i = elementGrid.firstI (); i <= elementGrid.lastI (); ++i ) {

            for ( j = elementGrid.firstJ (); j <= elementGrid.lastJ (); ++j ) {
               // m_subdomainElements ( i, j, subdomainK ).setLayerElement ( m_layers [ l ]->getLayerElement ( i, j, k ));
               // m_subdomainElements ( i, j, subdomainK ).setK ( subdomainK );
            }

         }

      }

      subdomainStartK += m_layers [ l ]->getMaximumNumberOfElements ();
   }

}

//------------------------------------------------------------//

void Subdomain::initialiseGlobalKToLayerMapping () {

   ReverseLayerIterator iter;
   unsigned int globalK;
   unsigned int k;

   m_globalKToLayerKMapping.resize ( maximumNumberOfElements ());
   m_globalKToFormationGridMapping.resize ( maximumNumberOfElements ());

   initialiseLayerIterator ( iter );
   globalK = 0;

   while ( not iter.isDone ()) {

      for ( k = 0; k < iter->getFormation ().getMaximumNumberOfElements (); ++k, ++globalK ) {
         m_globalKToLayerKMapping [ globalK ] = k;
         m_globalKToFormationGridMapping [ globalK ] = &(*iter);
      }

      ++iter;
   }
}

//------------------------------------------------------------//

void Subdomain::numberElements () {

   const ElementGrid& elementGrid = FastcauldronSimulator::getInstance ().getElementGrid ();

   ReverseLayerIterator iter;

   unsigned int i;
   unsigned int j;
   int k;
   int subdomainK;
   int subdomainStartK;

   subdomainStartK = 0;

   initialiseLayerIterator ( iter );

   while ( not iter.isDone ()) {

      for ( k = 0, subdomainK = subdomainStartK; k < iter->getFormation().getMaximumNumberOfElements (); ++k, ++subdomainK ) {

         for ( i = elementGrid.firstI ( true ); i <= elementGrid.lastI ( true ); ++i ) {

            for ( j = elementGrid.firstJ ( true ); j <= elementGrid.lastJ ( true ); ++j ) {

               if ( elementGrid.isPartOfStencil ( i, j )) {
                  (*iter)( i, j, k ).setK ( subdomainK );
               }

            }

         }

      }

      subdomainStartK += iter->getFormation ().getMaximumNumberOfElements ();
      ++iter;
   }

}

//------------------------------------------------------------//

void Subdomain::numberNodeDofs ( const double age ) {

   if ( not m_isActive ) {
      // There is nothing to number.
      return;
   }

   const FastcauldronSimulator& fc = FastcauldronSimulator::getInstance ();

   NodalVolumeGrid& scalarNodeGrid = getNodeGrid ( 1 );

   VecZeroEntries ( m_scalarDofNumbers );

   PETSC_3D_Array dof ( scalarNodeGrid.getDa (), m_scalarDofNumbers, INSERT_VALUES );

   int i;
   int j;
   int k;
   int maximumDegenerateSegments = 0;

   // One less because the first index is zero.
   int numberOfNodesInDepth = numberOfNodes () - 1;

   for ( i = fc.firstI (); i <= fc.lastI (); ++i ) {

      for ( j = fc.firstJ (); j <= fc.lastJ (); ++j ) {

         if ( fc.nodeIsDefined ( i, j )) {

            dof ( numberOfNodesInDepth, j, i ) = numberOfNodesInDepth;

            for ( k = numberOfNodesInDepth - 1; k >= 0; --k ) {
               const FormationSubdomainElementGrid& grid = *m_globalKToFormationGridMapping [ k ];
               int formationK = m_globalKToLayerKMapping [ k ];

               if ( grid.getFormation ().getDepositingThickness ( i, j, formationK, age ) > DepositingThicknessTolerance ) {
                  dof ( k, j, i ) = k;
               } else {
                  dof ( k, j, i ) = dof ( k + 1, j, i );
                  maximumDegenerateSegments = NumericFunctions::Maximum ( maximumDegenerateSegments,
                                                                          static_cast<int>( dof ( k + 1, j, i ) - k ));
               }

            }

         }

      }

   }

   double globalStencilWidth;
   double localStencilWidth = static_cast<double>( maximumDegenerateSegments );

   PetscGlobalMax ( &localStencilWidth, &globalStencilWidth, PETSC_COMM_WORLD );
   m_stencilWidth = static_cast<int>(globalStencilWidth);

   if (( fc.getCauldron ()->debug1 ) and ( fc.getRank () == 0 )) {
      PetscPrintf ( PETSC_COMM_WORLD, " Maximum number of degenerate segments: %d \n", m_stencilWidth );
   }

#if 0
   PetscViewer viewer;
   PetscViewerSetFormat(PETSC_VIEWER_STDOUT_WORLD, PETSC_VIEWER_ASCII_MATLAB );
   PetscViewerCreate ( PETSC_COMM_WORLD, &viewer);
   PetscViewerSetType(viewer, PETSC_VIEWER_ASCII );
   PetscViewerSetFormat(viewer, PETSC_VIEWER_ASCII_MATLAB );
   VecView ( m_scalarDofNumbers, viewer );
#endif

}

//------------------------------------------------------------//

void Subdomain::resizeGrids ( const int elementCount,
                              const int nodeCount ) {

   if ( elementCount == 0 or nodeCount <= 1 ) {
      return;
   }

   unsigned int i;
   bool resizeDofVector = getNodeGrid ( 1 ).getNumberOfZNodes () != nodeCount;
   PetscBool isValid;

   for ( i = 0; i < m_elementVolumeGrids.size (); ++i ) {

      if ( m_elementVolumeGrids [ i ] != 0 ) {
         m_elementVolumeGrids [ i ]->resizeInZDirection ( elementCount );
      }

   }

   for ( i = 0; i < m_nodalGrids.size (); ++i ) {

      if ( m_nodalGrids [ i ] != 0 ) {
         m_nodalGrids [ i ]->resizeInZDirection ( nodeCount );
      }

   }

   VecValid ( m_scalarDofNumbers, &isValid );

   // This is used for the node dof vector.
   if ( nodeCount > 1 and ( resizeDofVector or not isValid )) {

      if ( isValid ) {
         VecDestroy ( m_scalarDofNumbers );
      }

      DACreateGlobalVector ( m_nodalGrids [ 0 ]->getDa (), &m_scalarDofNumbers );

   }

}

//------------------------------------------------------------//

void Subdomain::setActivity ( const double currentTime ) {

   const int elementCount = numberOfElements ();
   const int nodeCount = numberOfNodes ();

   m_isActive = m_layers.size () > 0 and elementCount > 0;

   if ( m_isActive ) {
      m_sourceRockIsActive = determineSourceRockActivity ();
   } else {
      m_sourceRockIsActive = false;
   }

   resizeGrids ( elementCount, nodeCount );
   numberNodeDofs ( currentTime );
   setSubdomainBoundary ();

   // if ( currentTime < m_layers [ 0 ]->depoage ) {
   //    m_isActive = true;
   // } else {
   //    m_isActive = false;
   // }

}

//------------------------------------------------------------//

void Subdomain::setSubdomainBoundary () {

   if ( not m_isActive ) {
      // There is nothing to number.
      return;
   }

   NodalVolumeGrid& scalarNodeGrid = getNodeGrid ( 1 );

   PETSC_3D_Array dofs ( scalarNodeGrid.getDa (), m_scalarDofNumbers, INSERT_VALUES, true );

   double ***dof2 = 0;
   Vec LocalDOF;

   DAGetLocalVector ( scalarNodeGrid.getDa (), &LocalDOF );
   DAGlobalToLocalBegin ( scalarNodeGrid.getDa (), m_scalarDofNumbers, INSERT_VALUES, LocalDOF );
   DAGlobalToLocalEnd ( scalarNodeGrid.getDa (), m_scalarDofNumbers, INSERT_VALUES, LocalDOF );
   DAVecGetArray ( scalarNodeGrid.getDa (), LocalDOF,  &dof2 );


   ActiveLayerIterator iter;

   int nodeCount = numberOfNodes ();
   int globalKStart = nodeCount - 2;
   int topDof = nodeCount - 1;
   int globalK;

   int i;
   int j;
   int k;

   int n1;
   int n2;
   int n3;
   int n4;
   int n5;
   int n6;
   int n7;
   int n8;

   initialiseLayerIterator ( iter );

   while ( not iter.isDone ()) {

      FormationSubdomainElementGrid& formation = *iter;

      for ( i = formation.firstI (); i <= formation.lastI (); ++i ) {

         for ( j = formation.firstJ (); j <= formation.lastJ (); ++j ) {
            globalK = globalKStart;

            bool print = false; //FastcauldronSimulator::getInstance ().getRank () == 0 and (( i == 10 ) and ( j == 10 ));

            if ( print ) {
               cout << " ranges: " 
                    << FastcauldronSimulator::getInstance ().getRank () << "  "
                    << scalarNodeGrid.firstI () << "  "
                    << scalarNodeGrid.lastI  () << "  "
                    << scalarNodeGrid.firstJ () << "  "
                    << scalarNodeGrid.lastJ  () << "  "
                    << scalarNodeGrid.firstK () << "  "
                    << scalarNodeGrid.lastK  () << "  "
                  
                    << formation.firstI () << "  "
                    << formation.lastI  () << "  "
                    << formation.firstJ () << "  "
                    << formation.lastJ  () << "  "
                    << formation.firstK () << "  "
                    << formation.lastK  () << "  "

                    << endl;
            }

            for ( k = formation.lastK (); k >= formation.firstK (); --k, --globalK ) {
               SubdomainElement& element = formation ( i, j, k );

               if ( element.getLayerElement ().isActive ()) {
                  n1 = int ( dof2 [ globalK][ j][     i ]);
                  n2 = int ( dof2 [ globalK][ j][     i + 1 ]);
                  n3 = int ( dof2 [ globalK][ j + 1][ i + 1 ]);
                  n4 = int ( dof2 [ globalK][ j + 1][ i ]);
                  n5 = int ( dof2 [ globalK + 1][ j][     i ]);
                  n6 = int ( dof2 [ globalK + 1][ j][     i + 1 ]);
                  n7 = int ( dof2 [ globalK + 1][ j + 1][ i + 1 ]);
                  n8 = int ( dof2 [ globalK + 1][ j + 1][ i ]);

                  // n1 = int ( dofs ( globalK, j,     i ));
                  // n2 = int ( dofs ( globalK, j,     i + 1 ));
                  // n3 = int ( dofs ( globalK, j + 1, i + 1 ));
                  // n4 = int ( dofs ( globalK, j + 1, i ));

                  // n5 = int ( dofs ( globalK + 1, j,     i ));
                  // n6 = int ( dofs ( globalK + 1, j,     i + 1 ));
                  // n7 = int ( dofs ( globalK + 1, j + 1, i + 1 ));
                  // n8 = int ( dofs ( globalK + 1, j + 1, i ));


                  // Set node numbers.
                  element.setNodeK ( 0, n1 );
                  element.setNodeK ( 1, n2 );
                  element.setNodeK ( 2, n3 );
                  element.setNodeK ( 3, n4 );

                  element.setNodeK ( 4, n5 );
                  element.setNodeK ( 5, n6 );
                  element.setNodeK ( 6, n7 );
                  element.setNodeK ( 7, n8 );

                  if ( print ) {
                     cout << " element nums: " << i << "  " << j << "  " << k << "  "
                          << n1 << "  "
                          << n2 << "  "
                          << n3 << "  "
                          << n4 << "  "
                          << n5 << "  "
                          << n6 << "  "
                          << n7 << "  "
                          << n8 << "  "
                          << endl;

                     cout << " element nums: " << globalK << "  "
                          << dofs ( globalK, j,     i ) << "  "
                          << dofs ( globalK, j,     i + 1 ) << "  "
                          << dofs ( globalK, j + 1, i + 1 ) << "  "
                          << dofs ( globalK, j + 1, i ) << "  "
                          << endl;


                  }

                  element.setShallowIsOnDomainBoundary ( n1 == topDof and n2 == topDof and n3 == topDof and n4 == topDof );
                  element.setDeepIsOnDomainBoundary ( n5 == 0 and n6 == 0 and n7 == 0 and n8 == 0 );
               }

            }

         }

      }

      // Decrement the global-start for the next layer down.
      globalKStart -= formation.lastK () - formation.firstK () + 1;

      ++iter;
   }

   DAVecRestoreArray ( scalarNodeGrid.getDa (), LocalDOF,  &dof2 );
   DARestoreLocalVector ( scalarNodeGrid.getDa (), &LocalDOF );

}

//------------------------------------------------------------//

FormationSubdomainElementGrid* Subdomain::getGrid ( const LayerProps* formation ) {

   MapIterator iter = m_layerMap.find ( formation );

   if ( iter == m_layerMap.end ()) {
      return 0;
   } else {
      return iter->m_formationGrid;
   }

   // Just to stop the compiler from complaining.
   // The complaint (reaching end of function) is unjustified since 
   // all cases are covered in the if-then-else statement above.
   return 0;
}

//------------------------------------------------------------//

const FormationSubdomainElementGrid* Subdomain::getGrid ( const LayerProps* formation ) const {

   ConstMapIterator iter = m_layerMap.find ( formation );

   if ( iter == m_layerMap.end ()) {
      return 0;
   } else {
      return iter->m_formationGrid;
   }

   // Just to stop the compiler from complaining.
   // The complaint (reaching end of function) is unjustified since 
   // all cases are covered in the if-then-else statement above.
   return 0;
}

//------------------------------------------------------------//

bool Subdomain::determineContainsSulphur () const {

   ConstLayerIterator iter;

   initialiseLayerIterator ( iter );

   while ( not iter.isDone ()) {

      if ( iter->getFormation ().isSourceRock () and dynamic_cast<const Genex6::SourceRock* >(iter->getFormation ().getSourceRock1 ())->isSulphur ()) {
         return true;
      } 

      ++iter;
   }

   return false;
}

//------------------------------------------------------------//

bool Subdomain::determineSourceRockActivity () const {

   ConstActiveLayerIterator iter;

   initialiseLayerIterator ( iter );

   while ( not iter.isDone ()) {

      if ( iter->getFormation ().isSourceRock ()) {
         return true;
      } 

      ++iter;
   }

   return false;
}

//------------------------------------------------------------//

int Subdomain::numberOfElements () const {

   int elementCount = 0;

   ConstActiveLayerIterator iter;

   initialiseLayerIterator ( iter );

   while ( not iter.isDone ()) {
      elementCount += iter->getFormation ().getMaximumNumberOfElements ();
      ++iter;
   }

   return elementCount;
}

//------------------------------------------------------------//

int Subdomain::maximumNumberOfElements () const {

   int elementCount = 0;

   ConstLayerIterator iter;

   initialiseLayerIterator ( iter );

   while ( not iter.isDone ()) {
      elementCount += iter->getFormation ().getMaximumNumberOfElements ();
      ++iter;
   }

   return elementCount;
}

//------------------------------------------------------------//

int Subdomain::numberOfNodes () const {

   int nodeCount = 0;

   ConstActiveLayerIterator iter;

   initialiseLayerIterator ( iter );

   while ( not iter.isDone ()) {
      nodeCount += iter->getFormation ().getMaximumNumberOfElements ();
      ++iter;
   }

   return nodeCount + 1;
}

//------------------------------------------------------------//

int Subdomain::maximumNumberOfNodes () const {

   int nodeCount = 0;

   ConstLayerIterator iter;

   initialiseLayerIterator ( iter );

   while ( not iter.isDone ()) {
      nodeCount += iter->getFormation ().getMaximumNumberOfElements ();
      ++iter;
   }

   return nodeCount + 1;
}

//------------------------------------------------------------//

bool Subdomain::hasLayer ( const LayerProps* formation ) const {

   ConstMapIterator iter = m_layerMap.find ( formation );

#if 1
   if ( iter == m_layerMap.end ()) {
      return false;
   } else {
      return true;
   }
#else
   return iter != m_layerMap.end ();
#endif

}

//------------------------------------------------------------//

void Subdomain::allocateElementGrid ( const int numberOfDofs ) const {

   // Resize the array if array is too small, filling extra values with the null value.
   if ( m_elementVolumeGrids.size () < numberOfDofs ) {
      m_elementVolumeGrids.resize ( numberOfDofs, 0 );
   }

   // If the element-grid does not exist then create one.
   if ( m_elementVolumeGrids [ numberOfDofs - 1 ] == 0 ) {
      m_elementVolumeGrids [ numberOfDofs - 1 ] = new ElementVolumeGrid;
      m_elementVolumeGrids [ numberOfDofs - 1 ]->construct ( FastcauldronSimulator::getInstance ().getElementGrid (),
                                                             numberOfElements (),
                                                             numberOfDofs );
   }

}

//------------------------------------------------------------//

void Subdomain::createVolumeGrid ( const int numberOfDofs ) {
   allocateElementGrid ( numberOfDofs );
}

//------------------------------------------------------------//

ElementVolumeGrid& Subdomain::getVolumeGrid ( const int numberOfDofs ) {

   allocateElementGrid ( numberOfDofs );

   return *m_elementVolumeGrids [ numberOfDofs - 1 ];
}

//------------------------------------------------------------//

const ElementVolumeGrid& Subdomain::getVolumeGrid ( const int numberOfDofs ) const {

   allocateElementGrid ( numberOfDofs );

   return *m_elementVolumeGrids [ numberOfDofs - 1 ];
}

//------------------------------------------------------------//

void Subdomain::allocateNodeGrid ( const int numberOfDofs ) const {

   // Resize the array if array is too small, filling extra values with the null value.
   if ( m_nodalGrids.size () < numberOfDofs ) {
      m_nodalGrids.resize ( numberOfDofs, 0 );
   }

   // If the element-grid does not exist then create one.
   if ( m_nodalGrids [ numberOfDofs - 1 ] == 0 ) {
      m_nodalGrids [ numberOfDofs - 1 ] = new NodalVolumeGrid;
      m_nodalGrids [ numberOfDofs - 1 ]->construct ( FastcauldronSimulator::getInstance ().getNodalGrid (),
                                                     numberOfNodes (),
                                                     numberOfDofs );
   }

}

//------------------------------------------------------------//

void Subdomain::createNodeGrid ( const int numberOfDofs ) {
   allocateNodeGrid ( numberOfDofs );
}

//------------------------------------------------------------//

NodalVolumeGrid& Subdomain::getNodeGrid ( const int numberOfDofs ) {

   allocateNodeGrid ( numberOfDofs );

   return *m_nodalGrids [ numberOfDofs - 1 ];
}

//------------------------------------------------------------//

const NodalVolumeGrid& Subdomain::getNodeGrid ( const int numberOfDofs ) const {

   allocateNodeGrid ( numberOfDofs );

   return *m_nodalGrids [ numberOfDofs - 1 ];
}

//------------------------------------------------------------//

bool Subdomain::overlaps ( const Subdomain& subdomain ) const {

   ConstLayerIterator iter;

   initialiseLayerIterator ( iter );

   while ( not iter.isDone ()) {

      if ( subdomain.hasLayer ( &iter->getFormation ())) {
         return true;
      }

      ++iter;
   }

   return false;
}

//------------------------------------------------------------//

std::string Subdomain::image () const {

   std::stringstream buffer;

   if ( m_layers.size () == 0 ) {
      buffer << " The subdomain has no associated layers." << std::endl;
   } else {
      size_t i;

      buffer << std::setw ( 30 ) << "Layer name" 
             << std::setw ( 30 ) << "Top surface name" 
             << std::setw ( 10 ) << "Depo age"
             << std::setw ( 30 ) << "Bottom surface name" 
             << std::setw ( 10 ) << "Depo age"
             << std::setw (  5 ) << " "
             << std::setw ( 17 ) << "Min thickness"
             << std::setw ( 17 ) << "Max thickness"
             << std::setw ( 15 ) << "Segments"
             << std::endl;

      int segmentCount = 0;

      for ( i = 0; i < m_layers.size (); ++i ) {
         buffer << std::setw ( 30 ) << m_layers [ i ]->getName ()
                << std::setw ( 30 ) << m_layers [ i ]->getTopSurface ()->getName ()
                << std::setw ( 10 ) << m_layers [ i ]->getTopSurface ()->getSnapshot ()->getTime ()
                << std::setw ( 30 ) << m_layers [ i ]->getBottomSurface ()->getName ()
                << std::setw ( 10 ) << m_layers [ i ]->getBottomSurface ()->getSnapshot ()->getTime ()
                << std::setw (  5 ) << (m_layers [ i ]->isSourceRock () ? "SR" : "" )
                << std::setw ( 17 ) << m_layers [ i ]->getMinimumThickness ()
                << std::setw ( 17 ) << m_layers [ i ]->getMaximumThickness ()
                << std::setw ( 15 ) << m_layers [ i ]->getMaximumNumberOfElements ()
                << std::endl;
         segmentCount += m_layers [ i ]->getMaximumNumberOfElements ();
      }

      buffer << std::setw ( 164 ) << "--------------------" << std::endl;
      buffer << std::setw ( 149 ) << "Total" << std::setw ( 15 ) << segmentCount << std::endl;
   }

   // if ( m_layers.size () == 1 ) {
   //    buffer << "The subdomain is a single layer: " << m_layers [ 0 ]->layername << std::endl;
   // } else if ( m_layers.size () > 0 ) {
   //    size_t i;

   //    buffer << " The subdomain has " << m_layers.size () << " layers: " << std::endl;

   //    for ( i = 0; i < m_layers.size (); ++i ) {
   //       buffer << " layer " << std::setw ( 3 ) << i << "  " << std::setw ( 30 ) << m_layers [ i ]->layername << std::endl;
   //    }

   // } else {
   //    buffer << " The subdomain has no associated layers." << std::endl;
   // }

   return buffer.str ();
}

//------------------------------------------------------------//

void Subdomain::setId ( const int id ) {
   m_id = id;
}

//------------------------------------------------------------//
