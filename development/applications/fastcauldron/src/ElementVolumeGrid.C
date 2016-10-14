#include "ElementVolumeGrid.h"
#include "NumericFunctions.h"

//------------------------------------------------------------//

ElementVolumeGrid::ElementVolumeGrid () {
   m_xPartitioning = 0;
   m_numberOfXProcessors = 0;
   m_yPartitioning = 0;
   m_numberOfYProcessors = 0;
}

//------------------------------------------------------------//

ElementVolumeGrid::ElementVolumeGrid ( const ElementGrid& grid,
                                       const int          numberOfZElements,
                                       const int          numberOfDofs ) {
   m_xPartitioning = 0;
   m_numberOfXProcessors = 0;
   m_yPartitioning = 0;
   m_numberOfYProcessors = 0;
   construct ( grid, numberOfZElements, numberOfDofs );
}

//------------------------------------------------------------//

ElementVolumeGrid::~ElementVolumeGrid () {

   if ( isInitialised ()) {
      DMDestroy ( &m_localInfo.da );

      delete [] m_xPartitioning;
      m_xPartitioning = 0;

      delete [] m_yPartitioning;
      m_yPartitioning = 0;
   }

}

//------------------------------------------------------------//

void ElementVolumeGrid::construct ( const ElementGrid& grid,
                                    const int          numberOfZElements,
                                    const int          numberOfDofs ) {

   if ( isInitialised () and numberOfZElements == getNumberOfZElements () and numberOfDofs == getNumberOfDofs ()) {
      // the current size is the same as the requested new size, so nothing to do.
      return;
   }

   int i;
   DM volumeDa;

   if ( isInitialised ()) {
      delete [] m_xPartitioning;
      delete [] m_yPartitioning;
      DMDestroy ( &m_localInfo.da );
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

   DMDACreate3d ( PETSC_COMM_WORLD, DM_BOUNDARY_NONE, DM_BOUNDARY_NONE, DM_BOUNDARY_NONE, DMDA_STENCIL_STAR,
                  grid.getNumberOfXElements (),
                  grid.getNumberOfYElements (),
                  numberOfZElements,
                  grid.getNumberOfXProcessors (),
                  grid.getNumberOfYProcessors (),
                  1, 
                  numberOfDofs,
                  1,
                  m_xPartitioning,
                  m_yPartitioning,
                  PETSC_NULL,
                  &volumeDa );
   
   DMDAGetLocalInfo ( volumeDa, &m_localInfo );   

   m_first [ 0 ] = m_localInfo.xs;
   m_first [ 1 ] = m_localInfo.ys;
   m_first [ 2 ] = m_localInfo.zs;

   m_last  [ 0 ] = m_localInfo.xs + m_localInfo.xm - 1;
   m_last  [ 1 ] = m_localInfo.ys + m_localInfo.ym - 1;
   m_last  [ 2 ] = m_localInfo.zs + m_localInfo.zm - 1;

   m_ghostFirst [ 0 ] = m_localInfo.gxs;
   m_ghostFirst [ 1 ] = m_localInfo.gys;
   m_ghostFirst [ 2 ] = m_localInfo.gzs;

   m_ghostLast  [ 0 ] = m_localInfo.gxs + m_localInfo.gxm - 1;
   m_ghostLast  [ 1 ] = m_localInfo.gys + m_localInfo.gym - 1;
   m_ghostLast  [ 2 ] = m_localInfo.gzs + m_localInfo.gzm - 1;


}

//------------------------------------------------------------//

void ElementVolumeGrid::resizeInZDirection ( const int numberOfZElements ) {


   if ( not isInitialised ()) {
      // Error.
   } else {
      DM volumeDa;

      DMDestroy ( &m_localInfo.da );

      DMDACreate3d ( PETSC_COMM_WORLD, DM_BOUNDARY_NONE, DM_BOUNDARY_NONE, DM_BOUNDARY_NONE, DMDA_STENCIL_STAR,
                     getNumberOfXElements (),
                     getNumberOfYElements (),
                     numberOfZElements,
                     getNumberOfXProcessors (),
                     getNumberOfYProcessors (),
                     1, 
                     getNumberOfDofs (),
                     1,
                     m_xPartitioning,
                     m_yPartitioning,
                     PETSC_NULL,
                     &volumeDa );

      DMDAGetLocalInfo ( volumeDa, &m_localInfo );   

      m_first [ 0 ] = m_localInfo.xs;
      m_first [ 1 ] = m_localInfo.ys;
      m_first [ 2 ] = m_localInfo.zs;

      m_last  [ 0 ] = m_localInfo.xs + m_localInfo.xm - 1;
      m_last  [ 1 ] = m_localInfo.ys + m_localInfo.ym - 1;
      m_last  [ 2 ] = m_localInfo.zs + m_localInfo.zm - 1;

      m_ghostFirst [ 0 ] = m_localInfo.gxs;
      m_ghostFirst [ 1 ] = m_localInfo.gys;
      m_ghostFirst [ 2 ] = m_localInfo.gzs;

      m_ghostLast  [ 0 ] = m_localInfo.gxs + m_localInfo.gxm - 1;
      m_ghostLast  [ 1 ] = m_localInfo.gys + m_localInfo.gym - 1;
      m_ghostLast  [ 2 ] = m_localInfo.gzs + m_localInfo.gzm - 1;

   }

}

//------------------------------------------------------------//

bool ElementVolumeGrid::isPartOfStencil ( const int i,
                                          const int j,
                                          const int k ) const {

   bool isIinRange = NumericFunctions::inRange(i, firstI(), lastI());
   bool isJinRange = NumericFunctions::inRange(j, firstJ(), lastJ());
   bool isKinRange = NumericFunctions::inRange(k, firstK(), lastK());
   bool isIonBoundary = (i == firstI(true) or i == lastI(true));
   bool isJonBoundary = (j == firstJ(true) or j == lastJ(true));

   return  (isIinRange and isJinRange and isKinRange) or (isIinRange and isJonBoundary) or (isJinRange and isIonBoundary);
}

//------------------------------------------------------------//
