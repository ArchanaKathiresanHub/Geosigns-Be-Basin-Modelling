#ifndef _FASTCAULDRON__MATRIX_UNIT_TEST__H_
#define _FASTCAULDRON__MATRIX_UNIT_TEST__H_

#include "petsc.h"
#include "petscmat.h"

#include "PVTCalculator.h"

#include "propinterface.h"
#include "ElementVolumeGrid.h"
#include "LayerElement.h"
#include "PetscBlockVector.h"
#include "Subdomain.h"

#include "WallTime.h"

class MatrixUnitTest {

public :

   static void initialise ( AppCtx* cauldron );

   static void finalise ();

   static MatrixUnitTest& getInstance ();


   void solveForTimeStep ( const double                      startTime,
                           const double                      endTime,
                                 ElementVolumeGrid&          grid,
                           const LayerElementReferenceArray& elements );

   void solveForTimeStep ( const double     startTime,
                           const double     endTime );

protected :


   MatrixUnitTest ( AppCtx* cauldron );

   ~MatrixUnitTest ();


   void fillSystem ( Mat& mat,
                     PetscBlockVector<PVTComponents>& rhs,
                     const LayerElementReferenceArray& elements );

   void fillSystem ( Mat&                             mat,
                     PetscBlockVector<PVTComponents>& rhs,
                     const Subdomain&                 subdomain );

   double loadFunction ( const double x,
                         const double y,
                         const double z ) const;

   double dirichletBc ( const double x,
                        const double y,
                        const double z ) const;

   void printSolution ( Vec solutionVec,
                        const LayerElementReferenceArray& elements,
                        const ElementVolumeGrid& grid ) const;


   void printSolution ( Vec solutionVec,
                        const Subdomain& subdomain ) const;


   static PetscErrorCode residualMonitor (KSP ksp,PetscInt n,PetscReal rnorm,void *dummy);

   static MatrixUnitTest* m_matrixUnitTest;
   static bool m_fileIsOpen;
   static FILE* m_residualFile;

   static WallTime::Time iterationTime;

   Subdomain* m_subdomain;

   AppCtx* m_cauldron;

};

inline MatrixUnitTest& MatrixUnitTest::getInstance () {
   return *m_matrixUnitTest;
}



#endif // _FASTCAULDRON__MATRIX_UNIT_TEST__H_
