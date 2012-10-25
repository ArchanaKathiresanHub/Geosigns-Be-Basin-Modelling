#include "NodalVolumeGrid.h"

//------------------------------------------------------------//

NodalVolumeGrid::NodalVolumeGrid () {
   m_xPartitioning = 0;
   m_numberOfXProcessors = 0;
   m_yPartitioning = 0;
   m_numberOfYProcessors = 0;
}

//------------------------------------------------------------//

NodalVolumeGrid::~NodalVolumeGrid () {

   if ( m_xPartitioning != 0 ) {
      delete [] m_xPartitioning;
      m_xPartitioning = 0;
   }

   if ( m_yPartitioning != 0 ) {
      delete [] m_yPartitioning;
      m_yPartitioning = 0;
   }

}

//------------------------------------------------------------//

void NodalVolumeGrid::construct ( const NodalGrid& grid,
                                  const int        numberOfZNodes,
                                  const int        numberOfDofs ) {

   if ( isInitialised () and numberOfZNodes == getNumberOfZNodes () and numberOfDofs == getNumberOfDofs ()) {
      // the current size is the same as the requested new size, so nothing to do.
      return;
   }

   int i;
   DA volumeDa;

   if ( isInitialised ()) {
      delete [] m_xPartitioning;
      delete [] m_yPartitioning;
      DADestroy ( m_localInfo.da );
   }

   m_numberOfXProcessors = grid.getNumberOfXProcessors ();
   m_xPartitioning = new int [ m_numberOfXProcessors ];

   for ( i = 0; i < m_numberOfXProcessors; ++i ) {
      m_xPartitioning [ i ] = grid.getXPartitioning ()[ i ];
   }

   m_numberOfYProcessors = grid.getNumberOfYProcessors ();
   m_yPartitioning = new int [ m_numberOfYProcessors ];
   
   for ( i = 0; i < m_numberOfYProcessors; ++i ) {
      m_yPartitioning [ i ] = grid.getYPartitioning ()[ i ];
   }

   DACreate3d ( PETSC_COMM_WORLD, DA_NONPERIODIC, DA_STENCIL_BOX,
                grid.getNumberOfXNodes (),
                grid.getNumberOfYNodes (),
                numberOfZNodes,
                grid.getNumberOfXProcessors (),
                grid.getNumberOfYProcessors (),
                1, 
                numberOfDofs,
                1,
                m_xPartitioning,
                m_yPartitioning,
                PETSC_NULL,
                &volumeDa );

   DAGetLocalInfo ( volumeDa, &m_localInfo );   
}

//------------------------------------------------------------//

void NodalVolumeGrid::resizeInZDirection ( const int numberOfZNodes ) {


   if ( not isInitialised ()) {
      // Error.
   } else if ( numberOfZNodes == ( lastK () - firstK () + 1 )) {
      return;
   } else {
      DA volumeDa;

      DADestroy ( m_localInfo.da );

      DACreate3d ( PETSC_COMM_WORLD, DA_NONPERIODIC, DA_STENCIL_BOX,
                   getNumberOfXNodes (),
                   getNumberOfYNodes (),
                   numberOfZNodes,
                   getNumberOfXProcessors (),
                   getNumberOfYProcessors (),
                   1, 
                   getNumberOfDofs (),
                   1,
                   m_xPartitioning,
                   m_yPartitioning,
                   PETSC_NULL,
                   &volumeDa );

      DAGetLocalInfo ( volumeDa, &m_localInfo );   
   }

}

//------------------------------------------------------------//
