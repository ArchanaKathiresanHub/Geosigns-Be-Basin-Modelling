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
   m_column ( topLayer, bottomLayer ),
   m_isActive ( false ),
   m_sourceRockIsActive ( false )
{

   const AppCtx* cauldron = FastcauldronSimulator::getInstance ().getCauldron ();

   for ( size_t i = 0; i < m_column.getNumberOfLayers (); ++i ) {
      m_layerMap [ m_column.getLayer ( i )] = new FormationSubdomainElementGrid ( *m_column.getLayer ( i ));
   }

   int topIndex    = cauldron->getLayerIndex ( topLayer.getName ());
   int bottomIndex = cauldron->getLayerIndex ( bottomLayer.getName ());
   int i;

   linkSubdomainElementsVertically ();

   int elementCount = numberOfElements ();

   m_isActive = false;
   m_containsSulphurSourceRock = determineContainsSulphur ();

   if ( m_isActive ) {
      m_sourceRockIsActive = determineSourceRockActivity ();
      m_grids.allocateElementGrid ( 1 );
      m_grids.allocateNodeGrid ( 1 );
   }

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
      PetscSynchronizedFlush ( PETSC_COMM_WORLD, PETSC_STDOUT );

   }

   m_id = -1;
}

//------------------------------------------------------------//

Subdomain::Subdomain ( const LayerProps& theLayer ) : 
   m_column ( theLayer, theLayer )
{

   const AppCtx* cauldron = FastcauldronSimulator::getInstance ().getCauldron ();

   for ( size_t i = 0; i < m_column.getNumberOfLayers (); ++i ) {
      m_layerMap [ m_column.getLayer ( i )] = new FormationSubdomainElementGrid ( *m_column.getLayer ( i ));
   }

   int elementCount = numberOfElements ();
   m_isActive = false;
   m_containsSulphurSourceRock = determineContainsSulphur ();

   if ( m_isActive ) {
      m_sourceRockIsActive = determineSourceRockActivity ();
      m_grids.allocateElementGrid ( 1 );
      m_grids.allocateNodeGrid ( 1 );
   } else {
      m_sourceRockIsActive = false;
   }

   numberElements ();
   initialiseGlobalKToLayerMapping ();
   m_id = -1;
}

//------------------------------------------------------------//

Subdomain::~Subdomain () {

   FormationToElementGridMap::iterator it;

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
            above->getElement ( i, j, 0 ).setNeighbour ( VolumeData::DeepFace, &below->getElement ( i, j, belowSize ));
            below->getElement ( i, j, belowSize ).setNeighbour ( VolumeData::ShallowFace, &above->getElement ( i, j, 0 ));
         }

      }

      above = below;
      ++iter;
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
                  iter->getElement ( i, j, k ).setK ( subdomainK );
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

   m_scalarDofNumbers.fill ( 0 );

   int i;
   int j;
   int k;
   int maximumDegenerateSegments = 0;

   // One less because the first index is zero.
   int numberOfNodesInDepth = numberOfNodes () - 1;

   for ( i = fc.firstI ( true ); i <= fc.lastI ( true ); ++i ) {

      for ( j = fc.firstJ ( true ); j <= fc.lastJ ( true ); ++j ) {

         if ( fc.nodeIsDefined ( i, j )) {

            m_scalarDofNumbers ( i, j, numberOfNodesInDepth ) = numberOfNodesInDepth;

            for ( k = numberOfNodesInDepth - 1; k >= 0; --k ) {
               const FormationSubdomainElementGrid& grid = *m_globalKToFormationGridMapping [ k ];
               int formationK = m_globalKToLayerKMapping [ k ];

               if ( grid.getFormation ().getDepositingThickness ( i, j, formationK, age ) > DepositingThicknessTolerance ) {
                  m_scalarDofNumbers ( i, j, k ) = k;
               } else {
                  m_scalarDofNumbers ( i, j, k ) = m_scalarDofNumbers ( i, j, k + 1 );
                  maximumDegenerateSegments = NumericFunctions::Maximum ( maximumDegenerateSegments,
                                                                          static_cast<int>( m_scalarDofNumbers ( i, j, k + 1 ) - k ));
               }

            }

         }

      }

   }

   double globalStencilWidth;
   double localStencilWidth = static_cast<double>( maximumDegenerateSegments );

   // PetscGlobalMax ( &localStencilWidth, &globalStencilWidth, PETSC_COMM_WORLD );
   MPI_Allreduce( &localStencilWidth, &globalStencilWidth, 1, MPIU_REAL, MPI_MAX, PETSC_COMM_WORLD);
  
   m_stencilWidth = static_cast<int>(globalStencilWidth);

   if ( fc.getCauldron ()->debug1 or fc.getCauldron()->verbose ) {
      PetscPrintf ( PETSC_COMM_WORLD, " Maximum number of degenerate segments: %d \n", m_stencilWidth );
   }

}

//------------------------------------------------------------//

void Subdomain::resizeGrids ( const int elementCount,
                              const int nodeCount ) {

   if ( elementCount == 0 or nodeCount <= 1 ) {
      return;
   }

   m_grids.resizeGrids ( elementCount, nodeCount );
   m_scalarDofNumbers.reallocate ( FastcauldronSimulator::getInstance ().getActivityOutputGrid (), nodeCount );
}

//------------------------------------------------------------//

void Subdomain::setActivity ( const double currentTime ) {

   m_currentAge = currentTime;

   const int elementCount = numberOfElements ();
   const int nodeCount = numberOfNodes ();

   m_isActive = m_column.getNumberOfLayers () > 0 and elementCount > 0;

   if ( m_isActive ) {
      m_sourceRockIsActive = determineSourceRockActivity ();
   } else {
      m_sourceRockIsActive = false;
   }

   resizeGrids ( elementCount, nodeCount );
   numberNodeDofs ( currentTime );
   setSubdomainBoundary ();
}

//------------------------------------------------------------//

void Subdomain::setSubdomainBoundary () {

   if ( not m_isActive ) {
      // There is nothing to number.
      return;
   }

   int nodeCount = numberOfNodes ();
   int globalKStart = nodeCount - 1;
   int topDof = nodeCount - 1;

   for ( size_t l = 0; l < m_column.getNumberOfLayers (); ++l ) {
      const LayerProps* layer = m_column.getLayer ( l );

      if ( layer->depositionStartAge > m_currentAge ) {
         FormationSubdomainElementGrid* grid = m_layerMap [ layer ];

         grid->setElementNodeKValues ( m_scalarDofNumbers, topDof, globalKStart );
         globalKStart -= grid->lengthK ();
      }

   }

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

ElementVolumeGrid& Subdomain::getVolumeGrid ( const int numberOfDofs ) {
   return m_grids.getElementGrid ( numberOfDofs );
}

//------------------------------------------------------------//

const ElementVolumeGrid& Subdomain::getVolumeGrid ( const int numberOfDofs ) const {
   return m_grids.getElementGrid ( numberOfDofs );
}

//------------------------------------------------------------//

NodalVolumeGrid& Subdomain::getNodeGrid ( const int numberOfDofs ) {
   return m_grids.getNodeGrid ( numberOfDofs );
}

//------------------------------------------------------------//

const NodalVolumeGrid& Subdomain::getNodeGrid ( const int numberOfDofs ) const {
   return m_grids.getNodeGrid ( numberOfDofs );
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

   if ( m_column.getNumberOfLayers () == 0 ) {
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

      for ( i = 0; i < m_column.getNumberOfLayers (); ++i ) {
         buffer << std::setw ( 30 ) << m_column.getLayer ( i )->getName ()
                << std::setw ( 30 ) << m_column.getLayer ( i )->getTopSurface ()->getName ()
                << std::setw ( 10 ) << m_column.getLayer ( i )->getTopSurface ()->getSnapshot ()->getTime ()
                << std::setw ( 30 ) << m_column.getLayer ( i )->getBottomSurface ()->getName ()
                << std::setw ( 10 ) << m_column.getLayer ( i )->getBottomSurface ()->getSnapshot ()->getTime ()
                << std::setw (  5 ) << (m_column.getLayer ( i )->isSourceRock () ? "SR" : "" )
                << std::setw ( 17 ) << m_column.getLayer ( i )->getMinimumThickness ()
                << std::setw ( 17 ) << m_column.getLayer ( i )->getMaximumThickness ()
                << std::setw ( 15 ) << m_column.getLayer ( i )->getMaximumNumberOfElements ()
                << std::endl;
         segmentCount += m_column.getLayer ( i )->getMaximumNumberOfElements ();
      }

      buffer << std::setw ( 164 ) << "--------------------" << std::endl;
      buffer << std::setw ( 149 ) << "Total" << std::setw ( 15 ) << segmentCount << std::endl;
   }

   return buffer.str ();
}

//------------------------------------------------------------//

void Subdomain::setId ( const int id ) {
   m_id = id;
}

//------------------------------------------------------------//
