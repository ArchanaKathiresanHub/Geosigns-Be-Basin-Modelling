#include "MatrixUnitTest.h"
#include "FastcauldronSimulator.h"
// #include "include/private/matimpl.h" 

#include <iostream>
#include <iomanip>
#include <ostream>
#include <sstream>
#include <cmath>


#include "layer.h"
#include "layer_iterators.h"

#include "PetscBlockVector.h"

#include "GenericFortranMatrix.h"
#include "FastcauldronSimulator.h"
#include "LayerElement.h"
#include "Mesh3DIndex.h"
#include "NumericFunctions.h"

#include "BoundaryId.h"

#include "globaldefs.h"
#include "utils.h"

using namespace std;


PetscErrorCode MatrixUnitTest::residualMonitor (KSP ksp,PetscInt n,PetscReal rnorm,void *dummy) {

   if ( FastcauldronSimulator::getInstance ().getRank () == 0 and m_fileIsOpen ) {
      WallTime::Duration d = WallTime::clock () - iterationTime;
      double t = d.floatValue ();

      // PetscPrintf ( PETSC_COMM_WORLD, " residual ( %i ) = %e;  %e \n", n, rnorm, t );
      PetscFPrintf ( PETSC_COMM_WORLD, m_residualFile, " residual ( %i ) = %e;\n", n, rnorm );

      iterationTime = WallTime::clock ();
   }

   PetscSynchronizedFlush ( PETSC_COMM_WORLD );
   return 0;
}

FILE* MatrixUnitTest::m_residualFile;
bool MatrixUnitTest::m_fileIsOpen = false;
MatrixUnitTest* MatrixUnitTest::m_matrixUnitTest = 0;
WallTime::Time MatrixUnitTest::iterationTime;


MatrixUnitTest::MatrixUnitTest ( AppCtx* cauldron ) {
   m_cauldron = cauldron;

   if ( not m_fileIsOpen ) {
      std::stringstream fileNameBuffer;
      fileNameBuffer << FastcauldronSimulator::getInstance ().getProjectName () << "_" 
                     << FastcauldronSimulator::getInstance ().getSize () << "_" << UnitTest3FileName;
      std::string fileName = fileNameBuffer.str ();

      PetscFOpen ( PETSC_COMM_WORLD, fileName.c_str (), "w", &m_residualFile );
      m_fileIsOpen = true;
   }


   cout << endl << endl << " Allocating subdomain " << endl;

   m_subdomain = new Subdomain ( *m_cauldron->layers [ 0 ],
                                 *m_cauldron->layers [ m_cauldron->layers.size () - 3 ]);


   cout << endl << endl;

}

MatrixUnitTest::~MatrixUnitTest () {

   if ( m_fileIsOpen ) {
   // if ( FastcauldronSimulator::getInstance ().getRank () == 0 and m_fileIsOpen ) {
      PetscSynchronizedFlush ( PETSC_COMM_WORLD );
      PetscFClose ( PETSC_COMM_WORLD, m_residualFile );
      m_fileIsOpen = false;
      delete m_subdomain;
   }

}

double MatrixUnitTest::loadFunction ( const double x,
                                      const double y,
                                      const double z ) const {
   return -6.0;
}

double MatrixUnitTest::dirichletBc ( const double x,
                                     const double y,
                                     const double z ) const {
   return x * x + y * y + z * z;
}

void MatrixUnitTest::fillSystem ( Mat& mat,
                                  PetscBlockVector<PVTComponents>& rhs,
                                  const LayerElementReferenceArray& elements ) {

   // static const VolumeData::BoundaryId boundaries [ 4 ] = { VolumeData::GAMMA_2, VolumeData::GAMMA_3, VolumeData::GAMMA_4, VolumeData::GAMMA_5 };
   static const VolumeData::BoundaryId boundaries [ 6 ] = { VolumeData::GAMMA_2, VolumeData::GAMMA_3, VolumeData::GAMMA_4, VolumeData::GAMMA_5, VolumeData::ShallowFace, VolumeData::DeepFace };

   // static const int indexShiftI [ 4 ] = {  0, 1, 0, -1 };
   // static const int indexShiftJ [ 4 ] = { -1, 0, 1,  0 };

   static const int indexShiftI [ 6 ] = {  0, 1, 0, -1,  0, 0 };
   static const int indexShiftJ [ 6 ] = { -1, 0, 1,  0,  0, 0 };
   static const int indexShiftK [ 6 ] = {  0, 0, 0,  0,  1, -1 };

   GenericFortranMatrix<NumberOfPVTComponents> workSpace;
   GenericFortranMatrix<NumberOfPVTComponents> block;
   MatStencil rowConnectivityMap [ NumberOfPVTComponents ];
   MatStencil colConnectivityMap [ NumberOfPVTComponents ];
   int i;
   int j;
   int k;
   int c;

   bool activeElement;

   double deltaValues [ 6 ];
   double deltaX = 1.0 / double ( FastcauldronSimulator::getInstance ().getElementGrid ().getNumberOfXElements () - 1 );
   double deltaY = 1.0 / double ( FastcauldronSimulator::getInstance ().getElementGrid ().getNumberOfYElements () - 1 );
   double deltaZ = 1.0 / double ( elements.lastK () - elements.firstK ());
   double x;
   double y;
   double z;

   // The order of the deltas must match the order of the daces above.
   deltaValues [ 0 ] = deltaY;
   deltaValues [ 1 ] = deltaX;
   deltaValues [ 2 ] = deltaY;
   deltaValues [ 3 ] = deltaX;
   deltaValues [ 4 ] = deltaZ;
   deltaValues [ 5 ] = deltaZ;

   std::stringstream buffer;

   const LayerElement* neighbour;

   for ( i = elements.firstI (); i <= elements.lastI (); ++i ) {
      x = double ( i ) * deltaX;

      for ( j = elements.firstJ (); j <= elements.lastJ (); ++j ) {
         y = double ( j ) * deltaY;

         for ( k = elements.firstK (); k <= elements.lastK (); ++k ) {
            z = double ( k ) * deltaZ;

            const LayerElement& volumeElement = *elements ( i, j, k );

            block.zero ();

            activeElement = volumeElement.isActive ();

            for ( c = 0; c < NumberOfPVTComponents; ++c ) {
               rowConnectivityMap [ c ].i = volumeElement.getIPosition ();
               rowConnectivityMap [ c ].j = volumeElement.getJPosition ();
               rowConnectivityMap [ c ].k = volumeElement.getGlobalKPosition ();
               rowConnectivityMap [ c ].c = c;

               colConnectivityMap [ c ].i = volumeElement.getIPosition ();
               colConnectivityMap [ c ].j = volumeElement.getJPosition ();
               colConnectivityMap [ c ].k = volumeElement.getGlobalKPosition ();
               colConnectivityMap [ c ].c = c;
            }

            block.zero ();

            workSpace.identity ();
            workSpace *= -2.0 / ( deltaX * deltaX );
            block = workSpace;

            workSpace.identity ();
            workSpace *= -2.0 / ( deltaY * deltaY );
            block += workSpace;

            workSpace.identity ();
            workSpace *= -2.0 / ( deltaZ * deltaZ );
            block += workSpace;

            MatSetValuesBlockedStencil ( mat, 1, rowConnectivityMap, 1, colConnectivityMap, block.data (), INSERT_VALUES );

            rhs ( volumeElement.getPosition (), Mesh3DIndex::GLOBAL ) = loadFunction ( x, y, z );
            // rhs ( volumeElement.getIPosition (), volumeElement.getJPosition (), volumeElement.getGlobalKPosition ()) = loadFunction ( x, y );

            if ( activeElement or not activeElement ) {

               int face;

               for ( face = 0; face < 6; ++face ) {
                  VolumeData::BoundaryId id = boundaries [ face ];
                  neighbour = volumeElement.getNeighbour ( id );

                  if ( not volumeElement.isOnDomainBoundary ( id ) and neighbour != 0 ) { // volumeElement.isActiveBoundary ( id ) and 

                     for ( c = 0; c < NumberOfPVTComponents; ++c ) {
                        colConnectivityMap [ c ].i = neighbour->getIPosition ();
                        colConnectivityMap [ c ].j = neighbour->getJPosition ();
                        colConnectivityMap [ c ].k = neighbour->getGlobalKPosition ();

                        colConnectivityMap [ c ].c = c;
                     }

                     block.identity ();
                     block *= 1.0 / ( deltaValues [ face ] * deltaValues [ face ]);
                     MatSetValuesBlockedStencil ( mat, 1, rowConnectivityMap, 1, colConnectivityMap, block.data (), INSERT_VALUES );
                  } else if ( volumeElement.isOnDomainBoundary ( id )) { // volumeElement.isActiveBoundary ( id ) and 

                     const int iPos = volumeElement.getIPosition () + indexShiftI [ face ];
                     const int jPos = volumeElement.getJPosition () + indexShiftJ [ face ];
                     const int kPos = volumeElement.getGlobalKPosition () + indexShiftK [ face ];

                     double boundaryX = double ( iPos ) * deltaX;
                     double boundaryY = double ( jPos ) * deltaY;
                     double boundaryZ = double ( kPos ) * deltaZ;

                     rhs ( volumeElement.getPosition (), Mesh3DIndex::GLOBAL ) += -dirichletBc ( boundaryX, boundaryY, boundaryZ ) / ( deltaValues [ face ] * deltaValues [ face ]);
                     // rhs ( volumeElement.getIPosition (), volumeElement.getJPosition (), volumeElement.getGlobalKPosition ()) += -dirichletBc ( boundaryX, boundaryY ) / ( deltaValues [ face ] * deltaValues [ face ]);
                  }

               }

            }

         }

      }

   }

   MatAssemblyBegin ( mat, MAT_FINAL_ASSEMBLY );
   MatAssemblyEnd   ( mat, MAT_FINAL_ASSEMBLY );
}

#if 1
void MatrixUnitTest::fillSystem ( Mat&                             mat,
                                  PetscBlockVector<PVTComponents>& rhs,
                                  const Subdomain&                 subdomain ) {

   static const VolumeData::BoundaryId boundaries [ 6 ] = { VolumeData::GAMMA_2, VolumeData::GAMMA_3, VolumeData::GAMMA_4, VolumeData::GAMMA_5, VolumeData::ShallowFace, VolumeData::DeepFace };

   static const int indexShiftI [ 6 ] = {  0, 1, 0, -1,  0, 0 };
   static const int indexShiftJ [ 6 ] = { -1, 0, 1,  0,  0, 0 };
   static const int indexShiftK [ 6 ] = {  0, 0, 0,  0,  1, -1 };

   GenericFortranMatrix<NumberOfPVTComponents> workSpace;
   GenericFortranMatrix<NumberOfPVTComponents> block;
   MatStencil rowConnectivityMap [ NumberOfPVTComponents ];
   MatStencil colConnectivityMap [ NumberOfPVTComponents ];
   int i;
   int j;
   int k;
   int c;

   bool activeElement;

   double deltaValues [ 6 ];
   double deltaX = 1.0 / double ( FastcauldronSimulator::getInstance ().getElementGrid ().getNumberOfXElements () - 1 );
   double deltaY = 1.0 / double ( FastcauldronSimulator::getInstance ().getElementGrid ().getNumberOfYElements () - 1 );
   double deltaZ = 1.0 / double ( subdomain.numberOfElements () - 1 );
   double x;
   double y;
   double z;

   // The order of the deltas must match the order of the daces above.
   deltaValues [ 0 ] = deltaY;
   deltaValues [ 1 ] = deltaX;
   deltaValues [ 2 ] = deltaY;
   deltaValues [ 3 ] = deltaX;
   deltaValues [ 4 ] = deltaZ;
   deltaValues [ 5 ] = deltaZ;

   std::stringstream buffer;
   const SubdomainElement* neighbour;
   const LayerElement* layerNeighbour;

   Subdomain::ConstActiveLayerIterator iter;

   subdomain.initialiseLayerIterator ( iter );

   cout << " MatrixUnitTest::fillSystem " << endl;

   while ( not iter.isDone ()) {

      const FormationSubdomainElementGrid& elementGrid = *iter;

      cout << " processing layer " << elementGrid.getFormation ().layername << "  "
           << elementGrid.firstI () << "  " << elementGrid.lastI () << "  " 
           << elementGrid.firstJ () << "  " << elementGrid.lastJ () << "  " 
           << elementGrid.firstK () << "  " << elementGrid.lastK () << "  " 
           << endl;

      for ( i = elementGrid.firstI (); i <= elementGrid.lastI (); ++i ) {
         x = double ( i ) * deltaX;

         for ( j = elementGrid.firstJ (); j <= elementGrid.lastJ (); ++j ) {
            y = double ( j ) * deltaY;

            for ( k = elementGrid.firstK (); k <= elementGrid.lastK (); ++k ) {

               const SubdomainElement& element = elementGrid ( i, j, k );
               const LayerElement& volumeElement = element.getLayerElement ();

               cout << " layer element " << endl << volumeElement.image () << endl << endl;

               z = double ( element.getK ()) * deltaZ;

               block.zero ();

               activeElement = volumeElement.isActive ();

               for ( c = 0; c < NumberOfPVTComponents; ++c ) {
                  rowConnectivityMap [ c ].i = element.getI ();
                  rowConnectivityMap [ c ].j = element.getJ ();
                  rowConnectivityMap [ c ].k = element.getK ();
                  rowConnectivityMap [ c ].c = c;

                  colConnectivityMap [ c ].i = element.getI ();
                  colConnectivityMap [ c ].j = element.getJ ();
                  colConnectivityMap [ c ].k = element.getK ();
                  colConnectivityMap [ c ].c = c;
               }

               block.zero ();

               workSpace.identity ();
               workSpace *= -2.0 / ( deltaX * deltaX );
               block = workSpace;

               workSpace.identity ();
               workSpace *= -2.0 / ( deltaY * deltaY );
               block += workSpace;

               workSpace.identity ();
               workSpace *= -2.0 / ( deltaZ * deltaZ );
               block += workSpace;

               MatSetValuesBlockedStencil ( mat, 1, rowConnectivityMap, 1, colConnectivityMap, block.data (), INSERT_VALUES );

               rhs ( element.getK (), element.getJ (), element.getI ()) = loadFunction ( x, y, z );
               // rhs ( volumeElement.getIPosition (), volumeElement.getJPosition (), volumeElement.getGlobalKPosition ()) = loadFunction ( x, y );

               if ( i == 0 and j == 0 ) {
                  cout << " position "<< i << "  " << j << "  " << k << "  " << element.getK () << endl;
               }

               // What am I doing here? A or not A = true!
               if ( activeElement or not activeElement ) {

                  int face;
                  bool elementIsOnBoundary;

                  for ( face = 0; face < 6; ++face ) {
                     VolumeData::BoundaryId id = boundaries [ face ];
                     neighbour = element.getNeighbour ( id );
                     layerNeighbour = volumeElement.getNeighbour ( id );

                     bool XXX = layerNeighbour != 0 and layerNeighbour->isActive () and neighbour->getLayerElement ().getFormation ()->layername == elementGrid.getFormation ().layername;

                     if ( XXX and not volumeElement.isOnDomainBoundary ( id ) and volumeElement.getNeighbour ( id ) != 0 ) { // volumeElement.isActiveBoundary ( id ) and 
                     // if (( iter->getK () > 0 and iter->getK () < subdomain.numberOfElements () - 1 ) and not volumeElement.isOnDomainBoundary ( id ) and volumeElement.getNeighbour ( id ) != 0 ) { // volumeElement.isActiveBoundary ( id ) and 
                     // if ( not volumeElement.isOnDomainBoundary ( id ) and neighbour != 0 ) { // volumeElement.isActiveBoundary ( id ) and 

                     cout << " adding face " 
                          << element.getI () << "  " << element.getJ () << "  " << element.getK () << "  " 
                          << VolumeData::boundaryIdImage ( id ) << "  "
                          << element.getLayerElement ().getFormation ()->layername << "  "
                          << layerNeighbour->getFormation ()->layername << "  "
                          << endl;


                        for ( c = 0; c < NumberOfPVTComponents; ++c ) {
                           colConnectivityMap [ c ].i = neighbour->getI ();
                           colConnectivityMap [ c ].j = neighbour->getJ ();
                           colConnectivityMap [ c ].k = neighbour->getK ();

                           colConnectivityMap [ c ].c = c;
                        }

                        block.identity ();
                        block *= 1.0 / ( deltaValues [ face ] * deltaValues [ face ]);
                        MatSetValuesBlockedStencil ( mat, 1, rowConnectivityMap, 1, colConnectivityMap, block.data (), INSERT_VALUES );
                     } else if ( volumeElement.isOnDomainBoundary ( id )) { // volumeElement.isActiveBoundary ( id ) and 

                        const int iPos = element.getI () + indexShiftI [ face ];
                        const int jPos = element.getJ () + indexShiftJ [ face ];
                        const int kPos = element.getK () + indexShiftK [ face ];

                        double boundaryX = double ( iPos ) * deltaX;
                        double boundaryY = double ( jPos ) * deltaY;
                        double boundaryZ = double ( kPos ) * deltaZ;

                        rhs ( element.getK (), element.getJ (), element.getI ()) += -dirichletBc ( boundaryX, boundaryY, boundaryZ ) / ( deltaValues [ face ] * deltaValues [ face ]);
                        // rhs ( volumeElement.getIPosition (), volumeElement.getJPosition (), volumeElement.getGlobalKPosition ()) += -dirichletBc ( boundaryX, boundaryY ) / ( deltaValues [ face ] * deltaValues [ face ]);
                     }

                  }

               }

            }

         }

      }

      ++iter;
   }

   MatAssemblyBegin ( mat, MAT_FINAL_ASSEMBLY );
   MatAssemblyEnd   ( mat, MAT_FINAL_ASSEMBLY );
}
#else
void MatrixUnitTest::fillSystem ( Mat&                             mat,
                                  PetscBlockVector<PVTComponents>& rhs,
                                  const Subdomain&                 subdomain ) {

   static const VolumeData::BoundaryId boundaries [ 6 ] = { VolumeData::GAMMA_2, VolumeData::GAMMA_3, VolumeData::GAMMA_4, VolumeData::GAMMA_5, VolumeData::ShallowFace, VolumeData::DeepFace };

   static const int indexShiftI [ 6 ] = {  0, 1, 0, -1,  0, 0 };
   static const int indexShiftJ [ 6 ] = { -1, 0, 1,  0,  0, 0 };
   static const int indexShiftK [ 6 ] = {  0, 0, 0,  0,  1, -1 };

   GenericFortranMatrix<NumberOfPVTComponents> workSpace;
   GenericFortranMatrix<NumberOfPVTComponents> block;
   MatStencil rowConnectivityMap [ NumberOfPVTComponents ];
   MatStencil colConnectivityMap [ NumberOfPVTComponents ];
   int i;
   int j;
   int k;
   int c;

   bool activeElement;

   double deltaValues [ 6 ];
   double deltaX = 1.0 / double ( FastcauldronSimulator::getInstance ().getElementGrid ().getNumberOfXElements () - 1 );
   double deltaY = 1.0 / double ( FastcauldronSimulator::getInstance ().getElementGrid ().getNumberOfYElements () - 1 );
   double deltaZ = 1.0 / double ( subdomain.numberOfElements () - 1 );
   double x;
   double y;
   double z;

   // The order of the deltas must match the order of the daces above.
   deltaValues [ 0 ] = deltaY;
   deltaValues [ 1 ] = deltaX;
   deltaValues [ 2 ] = deltaY;
   deltaValues [ 3 ] = deltaX;
   deltaValues [ 4 ] = deltaZ;
   deltaValues [ 5 ] = deltaZ;

   std::stringstream buffer;
   const SubdomainElement* neighbour;

   Subdomain::ConstActiveLayerAllElementIterator iter;

   subdomain.initialiseElementIterator ( iter );

   while ( not iter.isDone ()) {

      const LayerElement& volumeElement = iter->getLayerElement ();

      x = double ( iter->getI ()) * deltaX;
      y = double ( iter->getJ ()) * deltaY;
      z = double ( iter->getK ()) * deltaZ;

      block.zero ();

      activeElement = volumeElement.isActive ();

      for ( c = 0; c < NumberOfPVTComponents; ++c ) {
         rowConnectivityMap [ c ].i = iter->getI ();
         rowConnectivityMap [ c ].j = iter->getJ ();
         rowConnectivityMap [ c ].k = iter->getK ();
         rowConnectivityMap [ c ].c = c;

         colConnectivityMap [ c ].i = iter->getI ();
         colConnectivityMap [ c ].j = iter->getJ ();
         colConnectivityMap [ c ].k = iter->getK ();
         colConnectivityMap [ c ].c = c;
      }

      block.zero ();

      workSpace.identity ();
      workSpace *= -2.0 / ( deltaX * deltaX );
      block = workSpace;

      workSpace.identity ();
      workSpace *= -2.0 / ( deltaY * deltaY );
      block += workSpace;

      workSpace.identity ();
      workSpace *= -2.0 / ( deltaZ * deltaZ );
      block += workSpace;

      MatSetValuesBlockedStencil ( mat, 1, rowConnectivityMap, 1, colConnectivityMap, block.data (), INSERT_VALUES );

      rhs ( iter->getK (), iter->getJ (), iter->getI ()) = loadFunction ( x, y, z );
      // rhs ( volumeElement.getIPosition (), volumeElement.getJPosition (), volumeElement.getGlobalKPosition ()) = loadFunction ( x, y );

      // What am I doing here?
      if ( activeElement or not activeElement ) {

         int face;
         bool elementIsOnBoundary;

         for ( face = 0; face < 6; ++face ) {
            VolumeData::BoundaryId id = boundaries [ face ];
            neighbour = iter->getNeighbour ( id );

            if ( not volumeElement.isOnDomainBoundary ( id ) and volumeElement.getNeighbour ( id ) != 0 ) { // volumeElement.isActiveBoundary ( id ) and 
            // if (( iter->getK () > 0 and iter->getK () < subdomain.numberOfElements () - 1 ) and not volumeElement.isOnDomainBoundary ( id ) and volumeElement.getNeighbour ( id ) != 0 ) { // volumeElement.isActiveBoundary ( id ) and 
            // if ( not volumeElement.isOnDomainBoundary ( id ) and neighbour != 0 ) { // volumeElement.isActiveBoundary ( id ) and 

               for ( c = 0; c < NumberOfPVTComponents; ++c ) {
                  colConnectivityMap [ c ].i = neighbour->getI ();
                  colConnectivityMap [ c ].j = neighbour->getJ ();
                  colConnectivityMap [ c ].k = neighbour->getK ();

                  colConnectivityMap [ c ].c = c;
               }

               block.identity ();
               block *= 1.0 / ( deltaValues [ face ] * deltaValues [ face ]);
               MatSetValuesBlockedStencil ( mat, 1, rowConnectivityMap, 1, colConnectivityMap, block.data (), INSERT_VALUES );
            } else if ( volumeElement.isOnDomainBoundary ( id )) { // volumeElement.isActiveBoundary ( id ) and 

               const int iPos = iter->getI () + indexShiftI [ face ];
               const int jPos = iter->getJ () + indexShiftJ [ face ];
               const int kPos = iter->getK () + indexShiftK [ face ];

               double boundaryX = double ( iPos ) * deltaX;
               double boundaryY = double ( jPos ) * deltaY;
               double boundaryZ = double ( kPos ) * deltaZ;

               rhs ( iter->getK (), iter->getJ (), iter->getI ()) += -dirichletBc ( boundaryX, boundaryY, boundaryZ ) / ( deltaValues [ face ] * deltaValues [ face ]);
               // rhs ( volumeElement.getIPosition (), volumeElement.getJPosition (), volumeElement.getGlobalKPosition ()) += -dirichletBc ( boundaryX, boundaryY ) / ( deltaValues [ face ] * deltaValues [ face ]);
            }

         }

      }

      ++iter;
   }

   MatAssemblyBegin ( mat, MAT_FINAL_ASSEMBLY );
   MatAssemblyEnd   ( mat, MAT_FINAL_ASSEMBLY );
}

#endif



void MatrixUnitTest::printSolution ( Vec solutionVec,
                                     const LayerElementReferenceArray& elements,
                                     const ElementVolumeGrid& grid ) const {

   PetscBlockVector<PVTComponents> solution;
   std::stringstream buffer;

   const int k = elements.lastK () - elements.firstK ();

   double deltaX = 1.0 / double ( FastcauldronSimulator::getInstance ().getElementGrid ().getNumberOfXElements () - 1 );
   double deltaY = 1.0 / double ( FastcauldronSimulator::getInstance ().getElementGrid ().getNumberOfYElements () - 1 );
   double deltaZ = 1.0 / double ( elements.lastK () - elements.firstK ());
   double x;
   double y;
   double z;

   int i;
   int j;

   z = double ( elements.lastK ()) * deltaZ;

   int maxI;
   int maxJ;
   double error;
   double maxError = 0;

   solution.setVector ( grid, solutionVec, INSERT_VALUES, true );

   buffer.precision ( 6 );
   buffer.flags ( ios::scientific );

   buffer << endl;
   buffer << " parameters " << deltaX << "  " << deltaY << "  " << deltaZ << "  " << k << "  " << grid.lastK () << endl;

   for ( i = grid.firstI (); i <= grid.lastI (); ++i ) {
      x = double ( i ) * deltaX;

      for ( j = grid.firstJ (); j <= grid.lastJ (); ++j ) {
         y = double ( j ) * deltaY;

         error = solution ( k - 1, j, i )( pvtFlash::C1 ) - dirichletBc ( x, y, z );
         error = solution ( elements.lastK (), j, i )( pvtFlash::C1 ) - dirichletBc ( x, y, z );

         if ( std::abs ( error ) > maxError ) {
            maxError = error;
            maxI = i;
            maxJ = j;
         }

         // buffer << " solution ( " << x << ", " << y << "  ) = " 
         //        << setw ( 18 ) << solution ( elements.lastK (), j, i )( pvtFlash::C1 ) << "  " 
         //        // << setw ( 18 ) << solution ( k - 1, j, i )( pvtFlash::C1 ) << "  " 
         //        << setw ( 18 ) << error << endl;

         buffer << " solution ( " << setw ( 2 ) << i + 1 << ", " << setw ( 2 ) << j + 1 << "  ) = " 
                << setw ( 12 ) << solution ( elements.lastK (), j, i )( pvtFlash::C1 ) << "  " 
                // << setw ( 18 ) << solution ( k - 1, j, i )( pvtFlash::C1 ) << "  " 
                << setw ( 12 ) << error << endl;

      }

   }

   buffer << endl;
   buffer << flush;

#if 0
   // Should find maximum over all processors.
   buffer << endl;
   buffer << " Maximum error: " << maxError << endl;
   buffer << " Occurred at  : " << maxI << "  " << maxJ << endl;
   buffer << " Solution     : " << solution ( k - 1, maxJ, maxI ) << endl;
   buffer << endl;
   buffer << endl;
#endif

   PetscSynchronizedFlush ( PETSC_COMM_WORLD );
   PetscSynchronizedFPrintf ( PETSC_COMM_WORLD, m_residualFile, buffer.str ().c_str ());
   PetscSynchronizedFlush ( PETSC_COMM_WORLD );
   PetscSynchronizedFPrintf ( PETSC_COMM_WORLD, m_residualFile, "\n");
   PetscSynchronizedFlush ( PETSC_COMM_WORLD );
}

void MatrixUnitTest::printSolution ( Vec solutionVec,
                                     const Subdomain& subdomain ) const {

   const ElementVolumeGrid& grid = subdomain.getVolumeGrid ( PVTComponents::BlockSize );

   PetscBlockVector<PVTComponents> solution;
   std::stringstream buffer;

   const int k = subdomain.numberOfElements () - 1;

   double deltaX = 1.0 / double ( FastcauldronSimulator::getInstance ().getElementGrid ().getNumberOfXElements () - 1 );
   double deltaY = 1.0 / double ( FastcauldronSimulator::getInstance ().getElementGrid ().getNumberOfYElements () - 1 );
   double deltaZ = 1.0 / double ( subdomain.numberOfElements () - 1 );
   double x;
   double y;
   double z;

   int i;
   int j;

   z = double ( subdomain.numberOfElements () - 1 ) * deltaZ;

   int maxI;
   int maxJ;
   double error;
   double maxError = 0;

   solution.setVector ( grid, solutionVec, INSERT_VALUES, true );

   buffer.precision ( 6 );
   buffer.flags ( ios::scientific );

   buffer << endl;
   buffer << " parameters " << deltaX << "  " << deltaY << "  " << deltaZ << "  " << k << "  " << grid.lastK () << endl;

   for ( i = grid.firstI (); i <= grid.lastI (); ++i ) {
      x = double ( i ) * deltaX;

      for ( j = grid.firstJ (); j <= grid.lastJ (); ++j ) {
         y = double ( j ) * deltaY;

         error = solution ( k - 1, j, i )( pvtFlash::C1 ) - dirichletBc ( x, y, z );
         error = solution ( subdomain.numberOfElements () - 1, j, i )( pvtFlash::C1 ) - dirichletBc ( x, y, z );

         if ( std::abs ( error ) > maxError ) {
            maxError = error;
            maxI = i;
            maxJ = j;
         }

         buffer << " solution ( " << setw ( 2 ) << i + 1 << ", " << setw ( 2 ) << j + 1 << "  ) = " 
                << setw ( 12 ) << solution ( subdomain.numberOfElements () - 1, j, i )( pvtFlash::C1 ) << "  " 
                << setw ( 12 ) << error << endl;

      }

   }

   buffer << endl;
   buffer << flush;

#if 0
   // Should find maximum over all processors.
   buffer << endl;
   buffer << " Maximum error: " << maxError << endl;
   buffer << " Occurred at  : " << maxI << "  " << maxJ << endl;
   buffer << " Solution     : " << solution ( k - 1, maxJ, maxI ) << endl;
   buffer << endl;
   buffer << endl;
#endif

   PetscSynchronizedFlush ( PETSC_COMM_WORLD );
   PetscSynchronizedFPrintf ( PETSC_COMM_WORLD, m_residualFile, buffer.str ().c_str ());
   PetscSynchronizedFlush ( PETSC_COMM_WORLD );
   PetscSynchronizedFPrintf ( PETSC_COMM_WORLD, m_residualFile, "\n");
   PetscSynchronizedFlush ( PETSC_COMM_WORLD );
}


void MatrixUnitTest::solveForTimeStep ( const double                      startTime,
                                        const double                      endTime,
                                              ElementVolumeGrid&          grid,
                                        const LayerElementReferenceArray& elements ) {

   if ( m_cauldron->getUnitTestNumber () == 3 ) {

      WallTime::Time timeStepStartTime = WallTime::clock ();
      WallTime::Time eventStartTime;
      WallTime::Duration totalDuration;

      Mat mat;
      Vec rhsVec;
      Vec solutionVec;
      KSP solver;


      PVTComponents*** array;

      PetscBlockVector<PVTComponents> components;
      PetscBlockVector<PVTComponents> rhs;
      int i;
      int j;
      int k;

      DMDALocalInfo info;

      
      DMDAGetLocalInfo ( grid.getDa (), &info );


      DMCreateGlobalVector ( grid.getDa (), &rhsVec );
      DMCreateGlobalVector ( grid.getDa (), &solutionVec );

      DMCreateMatrix ( grid.getDa (), MATBAIJ, &mat );

      rhs.setVector ( grid, rhsVec, INSERT_VALUES, true );

      for ( i = elements.firstI (); i <= elements.lastI (); ++i ) {

         for ( j = elements.firstJ (); j <= elements.lastJ (); ++j ) {

            for ( k = elements.firstK (); k <= elements.lastK (); ++k ) {
               rhs ( elements ( i, j, k )->getPosition (), Mesh3DIndex::GLOBAL ).zero ();
            }

         }

      }

      eventStartTime = WallTime::clock ();
      fillSystem ( mat, rhs, elements );
      rhs.restoreVector ( UPDATE_INCLUDING_GHOSTS );

#if 0
      if ( FastcauldronSimulator::getInstance ().getRank () == 0 ) {
         cout << " time taken to fill matrix: "; // << WallTime::clock () - eventStartTime << endl;
         WallTime::print ( cout, WallTime::clock () - eventStartTime );
         cout << endl;
      }
#endif

#if 1
      if ( endTime > 149.8 or true ) { // present day!
      // if ( endTime < 0.05 ) { // present day!

         PetscViewer viewer;
         PetscViewerSetFormat(PETSC_VIEWER_STDOUT_WORLD, PETSC_VIEWER_ASCII_MATLAB );
         PetscViewerCreate ( PETSC_COMM_WORLD, &viewer);
         PetscViewerSetType(viewer, PETSCVIEWERASCII );
         PetscViewerSetFormat(viewer, PETSC_VIEWER_ASCII_MATLAB );

         // if ( startTime < 1 or endTime < 1 ) {
         std::stringstream matrixFileNameBuffer;
         matrixFileNameBuffer << FastcauldronSimulator::getInstance ().getProjectName () << "_" 
                              << FastcauldronSimulator::getInstance ().getSize () << "_" << UnitTest3MatrixFileName;

         std::string matrixFileName = matrixFileNameBuffer.str ();
         // PetscViewerBinaryOpen(PETSC_COMM_WORLD, matrixFileName.c_str (),FILE_MODE_WRITE, &viewer);
         PetscViewerASCIIOpen(PETSC_COMM_WORLD, matrixFileName.c_str (),&viewer);

         VecView ( rhsVec, viewer );
         MatView ( mat, viewer );
         PetscViewerDestroy ( &viewer );
      }
#endif

      VecSet ( solutionVec, 0.0 );

      VecAssemblyBegin ( solutionVec );
      VecAssemblyEnd ( solutionVec );

      eventStartTime = WallTime::clock ();
      KSPCreate ( PETSC_COMM_WORLD, &solver );
      KSPSetFromOptions ( solver );
      PetscSynchronizedFlush ( PETSC_COMM_WORLD );
      // cout << flush;
      KSPSetOperators ( solver, mat, mat, DIFFERENT_NONZERO_PATTERN );

#if 0
      if ( FastcauldronSimulator::getInstance ().getRank () == 0 ) {
         cout << " time taken to set operators: "; // << WallTime::clock () - eventStartTime << endl;
         WallTime::print ( cout, WallTime::clock () - eventStartTime );
         cout << endl;
      }
#endif

      PetscSynchronizedFlush ( PETSC_COMM_WORLD );
      // cout << flush;

      if ( FastcauldronSimulator::getInstance ().getRank () == 0 ) {
         PetscSynchronizedFlush ( PETSC_COMM_WORLD );
         PetscFPrintf ( PETSC_COMM_WORLD, m_residualFile, "\n" );
         PetscFPrintf ( PETSC_COMM_WORLD, m_residualFile, "------------------------------------------------------------\n" );
         PetscFPrintf ( PETSC_COMM_WORLD, m_residualFile, "Starting solve for time step %e %e\n", startTime, endTime );
         PetscSynchronizedFlush ( PETSC_COMM_WORLD );
      }

#if 0
      if ( FastcauldronSimulator::getInstance ().getRank () == 0 ) {
         PetscPrintf ( PETSC_COMM_WORLD, "\n" );
         PetscPrintf ( PETSC_COMM_WORLD, "------------------------------------------------------------\n" );
         PetscPrintf ( PETSC_COMM_WORLD, "Starting solve for time step %e %e\n", startTime, endTime );
      }
#endif

      iterationTime = WallTime::clock ();
      KSPMonitorSet ( solver, residualMonitor, PETSC_NULL, PETSC_NULL );
      eventStartTime = WallTime::clock ();
      setSolverTolerance ( solver, 1.0e-10 );

      // PC prec;
      int its;

      // KSPGetPC(solver,&prec);
      // PCSetType(prec,PCNONE);
      // PCSetType(prec,PCBJACOBI);
      // setSolverMaxIterations ( solver, 200 );
      iterationTime = WallTime::clock ();
      KSPSolve ( solver, rhsVec, solutionVec );
      PetscSynchronizedFlush ( PETSC_COMM_WORLD );
      KSPGetIterationNumber(solver,&its);

#if 0
      if ( FastcauldronSimulator::getInstance ().getRank () == 0 ) {
         cout << " time taken to solve: " << its << "  "; // << WallTime::clock () - eventStartTime << endl;
         WallTime::print ( cout, WallTime::clock () - eventStartTime );
         cout << endl;
      }
#endif

      printSolution ( solutionVec, elements, grid );

      rhs.setVector ( grid, solutionVec, INSERT_VALUES );


      VecDestroy ( &rhsVec );
      VecDestroy ( &solutionVec );
      MatDestroy ( &mat );

#if 0
      if ( FastcauldronSimulator::getInstance ().getRank () == 0 ) {
         cout << " overall time: "; // << WallTime::clock () - eventStartTime << endl;
         WallTime::print ( cout, WallTime::clock () - timeStepStartTime );
         cout << endl;
      }
#endif

      exit ( 1 );
   }

}


void MatrixUnitTest::initialise ( AppCtx* cauldron ) {
   m_matrixUnitTest = new MatrixUnitTest ( cauldron );
}

void MatrixUnitTest::finalise () {
   delete m_matrixUnitTest;
}

void MatrixUnitTest::solveForTimeStep ( const double startTime,
                                        const double endTime ) {


   if ( m_cauldron->getUnitTestNumber () == 3 ) {

      Subdomain& subdomain = *m_subdomain;

      subdomain.setActivity ( endTime );

      const ElementVolumeGrid& grid = subdomain.getVolumeGrid ( PVTComponents::BlockSize );

      WallTime::Time timeStepStartTime = WallTime::clock ();
      WallTime::Time eventStartTime;
      WallTime::Duration totalDuration;

      Mat mat;
      Vec rhsVec;
      Vec solutionVec;
      KSP solver;

      PVTComponents*** array;

      PetscBlockVector<PVTComponents> components;
      PetscBlockVector<PVTComponents> rhs;
      int i;
      int j;
      int k;

      DMDALocalInfo info;

      
      DMDAGetLocalInfo ( grid.getDa (), &info );


      DMCreateGlobalVector ( grid.getDa (), &rhsVec );
      DMCreateGlobalVector ( grid.getDa (), &solutionVec );

      DMCreateMatrix ( grid.getDa (), MATBAIJ, &mat );

      rhs.setVector ( grid, rhsVec, INSERT_VALUES, true );

      Subdomain::ActiveLayerAllElementIterator iter;

      subdomain.initialiseElementIterator ( iter );

      while ( not iter.isDone ()) {
         rhs ( iter->getK (), iter->getJ (), iter->getI ()).zero ();
         ++iter;
      }

      eventStartTime = WallTime::clock ();
      fillSystem ( mat, rhs, subdomain );
      rhs.restoreVector ( UPDATE_INCLUDING_GHOSTS );

#if 1
      if ( endTime > 148.9 or true ) { // present day!
      // if ( endTime < 0.05 ) { // present day!

         PetscViewer viewer;
         PetscViewerSetFormat(PETSC_VIEWER_STDOUT_WORLD, PETSC_VIEWER_ASCII_MATLAB );
         PetscViewerCreate ( PETSC_COMM_WORLD, &viewer);
         PetscViewerSetType(viewer, PETSCVIEWERASCII );
         PetscViewerSetFormat(viewer, PETSC_VIEWER_ASCII_MATLAB );

         // if ( startTime < 1 or endTime < 1 ) {
         std::stringstream matrixFileNameBuffer;
         matrixFileNameBuffer << FastcauldronSimulator::getInstance ().getProjectName () << "_" 
                              << FastcauldronSimulator::getInstance ().getSize () << "_" << UnitTest3MatrixFileName;

         std::string matrixFileName = matrixFileNameBuffer.str ();
         // PetscViewerBinaryOpen(PETSC_COMM_WORLD, matrixFileName.c_str (),FILE_MODE_WRITE, &viewer);
         PetscViewerASCIIOpen(PETSC_COMM_WORLD, matrixFileName.c_str (),&viewer);

         cout << " matrixFileName.c_str () " << matrixFileName.c_str () << flush << endl;

         VecView ( rhsVec, viewer );
         MatView ( mat, viewer );
         PetscViewerDestroy ( &viewer );
      }
#endif

      VecSet ( solutionVec, 0.0 );

      VecAssemblyBegin ( solutionVec );
      VecAssemblyEnd ( solutionVec );

      eventStartTime = WallTime::clock ();
      KSPCreate ( PETSC_COMM_WORLD, &solver );
      KSPSetFromOptions ( solver );
      PetscSynchronizedFlush ( PETSC_COMM_WORLD );
      KSPSetOperators ( solver, mat, mat, DIFFERENT_NONZERO_PATTERN );

      PetscSynchronizedFlush ( PETSC_COMM_WORLD );

      if ( FastcauldronSimulator::getInstance ().getRank () == 0 ) {
         PetscSynchronizedFlush ( PETSC_COMM_WORLD );
         PetscFPrintf ( PETSC_COMM_WORLD, m_residualFile, "\n" );
         PetscFPrintf ( PETSC_COMM_WORLD, m_residualFile, "------------------------------------------------------------\n" );
         PetscFPrintf ( PETSC_COMM_WORLD, m_residualFile, "Starting solve for time step %e %e\n", startTime, endTime );
         PetscSynchronizedFlush ( PETSC_COMM_WORLD );
      }

      iterationTime = WallTime::clock ();
      KSPMonitorSet ( solver, residualMonitor, PETSC_NULL, PETSC_NULL );
      eventStartTime = WallTime::clock ();
      setSolverTolerance ( solver, 1.0e-10 );

      int its;

      iterationTime = WallTime::clock ();
      KSPSolve ( solver, rhsVec, solutionVec );
      PetscSynchronizedFlush ( PETSC_COMM_WORLD );
      KSPGetIterationNumber(solver,&its);

      printSolution ( solutionVec, subdomain );

      rhs.setVector ( grid, solutionVec, INSERT_VALUES );

      VecDestroy ( &rhsVec );
      VecDestroy ( &solutionVec );
      MatDestroy ( &mat );

      exit ( 1 );
   }

}

