#include "NodalGrid.h"

#include "Interface/DistributedGrid.h"

#include "NumericFunctions.h"

NodalGrid::NodalGrid () {
   m_xPartitioning = 0;
   m_numberOfXProcessors = 0;
   m_yPartitioning = 0;
   m_numberOfYProcessors = 0;
}


NodalGrid::~NodalGrid () {

   if ( m_xPartitioning != 0 ) {
      DMDestroy ( &m_localInfo.da );

      delete [] m_xPartitioning;
      m_xPartitioning = 0;

      delete [] m_yPartitioning;
      m_yPartitioning = 0;
   }

}

void NodalGrid::construct ( const DataAccess::Interface::Grid* nodeGrid,
                            const int                          rank ) {

   // Used temporarily to create the DA, ownership of contents 
   // is then passed to the m_localInfo.da component.
   DM nodalDa;

   int i;

   m_numberOfXProcessors = nodeGrid->numProcsI ();
   m_numberOfYProcessors = nodeGrid->numProcsJ ();

   int totalNumberOfXNodes = nodeGrid->numIGlobal ();
   int totalNumberOfYNodes = nodeGrid->numJGlobal ();
   int numberOfXNodes;
   int numberOfYNodes;

   if ( m_xPartitioning != 0 ) {
      delete [] m_xPartitioning;
      delete [] m_yPartitioning;
      DMDestroy ( &m_localInfo.da );
   }

   m_xPartitioning = new int [ m_numberOfXProcessors ];
   m_yPartitioning = new int [ m_numberOfYProcessors ];

   for ( i = 0; i < m_numberOfXProcessors; ++i ) {
      m_xPartitioning [ i ] = static_cast<int> ( nodeGrid->numsI ()[ i ]);
   }

   for ( i = 0; i < m_numberOfYProcessors; ++i ) {
      m_yPartitioning [ i ] = static_cast<int> ( nodeGrid->numsJ ()[ i ]);
   }

   DMDACreate2d ( PETSC_COMM_WORLD, DMDA_BOUNDARY_NONE, DMDA_BOUNDARY_NONE, DMDA_STENCIL_BOX,
                  totalNumberOfXNodes,
                  totalNumberOfYNodes,
                  m_numberOfXProcessors,
                  m_numberOfYProcessors,
                  1, 1,
                  m_xPartitioning,
                  m_yPartitioning,
                  &nodalDa );
   
   DMDAGetLocalInfo ( nodalDa, &m_localInfo );
}

