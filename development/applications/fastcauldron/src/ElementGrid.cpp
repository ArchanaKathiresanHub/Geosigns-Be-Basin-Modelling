#include "ElementGrid.h"
#include "NumericFunctions.h"

#include "DistributedGrid.h"


//------------------------------------------------------------//

ElementGrid::ElementGrid () {
   m_xPartitioning = 0;
   m_numberOfXProcessors = 0;
   m_yPartitioning = 0;
   m_numberOfYProcessors = 0;
}

//------------------------------------------------------------//

ElementGrid::~ElementGrid () {

   if ( m_xPartitioning != 0 ) {
      DMDestroy ( &m_localInfo.da );

      delete [] m_xPartitioning;
      m_xPartitioning = 0;

      delete [] m_yPartitioning;
      m_yPartitioning = 0;
   }

}

//------------------------------------------------------------//

void ElementGrid::construct ( const DataAccess::Interface::Grid* nodeGrid,
                              const int                          rank ) {

   const DMDALocalInfo& info = dynamic_cast<const DataAccess::Interface::DistributedGrid*>(nodeGrid)->getLocalInfo ();

   // Used temporarily to create the DA, ownership of contents 
   // is then passed to the m_localInfo.da component.
   DM elementDa;

   int i;

   m_numberOfXProcessors = nodeGrid->numProcsI ();
   m_numberOfYProcessors = nodeGrid->numProcsJ ();

   int totalNumberOfXElements = nodeGrid->numIGlobal () - 1;
   int totalNumberOfYElements = nodeGrid->numJGlobal () - 1;

   int* buffer = new int [ NumericFunctions::Maximum<int>( m_numberOfXProcessors, m_numberOfYProcessors )];

   if ( m_xPartitioning != 0 ) {
      delete [] m_xPartitioning;
      delete [] m_yPartitioning;
      DMDestroy ( &m_localInfo.da );
   }

   m_xPartitioning = new int [ m_numberOfXProcessors ];
   m_yPartitioning = new int [ m_numberOfYProcessors ];

   int myXProc = rank % nodeGrid->numProcsI ();
   int myYProc = rank / nodeGrid->numProcsI ();

   int numberOfXElements;
   int numberOfYElements;

   if (( info.xs + info.xm ) == info.mx ) {
      numberOfXElements = info.xm - 1;
   } else {
      numberOfXElements = info.xm;
   }

   if (( info.ys + info.ym ) == info.my ) {
      numberOfYElements = info.ym - 1;
   } else {
      numberOfYElements = info.ym;
   }

   for ( i = 0; i < m_numberOfXProcessors; ++i ) {
      buffer [ i ] = 0;
      m_xPartitioning [ i ] = 0;
   }

   buffer [ myXProc ] = numberOfXElements;

   for ( i = 0; i < m_numberOfXProcessors; ++i ) {
      MPI_Allreduce ( &buffer [ i ], &m_xPartitioning [ i ], 1, MPI_INT, MPI_MAX, PETSC_COMM_WORLD );
   }


   // Fill xElementPartitioning array.
   // MPI_Allreduce ( buffer, m_xPartitioning, m_numberOfXProcessors, MPI_INT, MPI_MAX, PETSC_COMM_WORLD );

   for ( i = 0; i < m_numberOfYProcessors; ++i ) {
      buffer [ i ] = 0;
      m_yPartitioning [ i ] = 0;
   }

   buffer [ myYProc ] = numberOfYElements;

   for ( i = 0; i < m_numberOfYProcessors; ++i ) {
      MPI_Allreduce ( &buffer [ i ], &m_yPartitioning [ i ], 1, MPI_INT, MPI_MAX, PETSC_COMM_WORLD );
   }
   /* Create a DMDA and an associated vector */
   // // Fill yElementPartitioning array.
   // MPI_Allreduce ( buffer, m_yPartitioning, m_numberOfYProcessors, MPI_INT, MPI_MAX, PETSC_COMM_WORLD );
   PetscErrorCode err = DMDACreate2d(PETSC_COMM_WORLD, DM_BOUNDARY_NONE, DM_BOUNDARY_NONE, DMDA_STENCIL_STAR,
                  totalNumberOfXElements,
                  totalNumberOfYElements,
                  m_numberOfXProcessors,
                  m_numberOfYProcessors,
                  1, 1,
                  m_xPartitioning,
                  m_yPartitioning,
                  &elementDa );
   
   err = DMSetFromOptions(elementDa);
   err = DMSetUp(elementDa);
   err = DMDAGetLocalInfo(elementDa, &m_localInfo);
   delete [] buffer;
}

//------------------------------------------------------------//

bool ElementGrid::isPartOfStencil ( const int i,
                                    const int j ) const 
{

   bool isIinRange = NumericFunctions::inRange(i, firstI(), lastI());
   bool isJinRange = NumericFunctions::inRange(j, firstJ(), lastJ());
   bool isIonBoundary = (i == firstI(true) or i == lastI(true));
   bool isJonBoundary = (j == firstJ(true) or j == lastJ(true));

   return  (isIinRange and isJinRange) or (isIinRange and isJonBoundary) or (isJinRange and isIonBoundary);
}

//------------------------------------------------------------//
