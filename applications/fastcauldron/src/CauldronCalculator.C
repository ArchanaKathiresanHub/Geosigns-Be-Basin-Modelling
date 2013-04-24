#include "CauldronCalculator.h"

#define DAPeriodic(pt) ((pt) == DMDA_BOUNDARY_PERIODIC )

//------------------------------------------------------------//

CauldronCalculator::CauldronCalculator ( AppCtx* appl ) {
   cauldron = appl;
}

//------------------------------------------------------------//

CauldronCalculator::~CauldronCalculator () {
}

//------------------------------------------------------------//

//
//
// Is there any way to eliminate the Depth array used here?
// And use the depths stored in the layer arrays?
//
void CauldronCalculator::setDOFs ( const DM   femGrid, 
                                   const Vec  depths, 
                                         Vec  dofNumbers, 
                                         int& stencilWidth ) {

  int i, j,k;
  int xs, ys, zs, xm, ym, zm;
  int maxNbCollapsedSegt = 0;

  DMDAGetCorners( femGrid, &xs, &ys, &zs, &xm, &ym, &zm ); 

  PETSC_3D_Array depth ( femGrid, depths );
  PETSC_3D_Array dof   ( femGrid, dofNumbers );

  for (i=xs; i<xs+xm; i++) {

    for (j=ys; j<ys+ym; j++) {

      if ( not cauldron->nodeIsDefined ( i, j )) continue;

      for (k=zs+zm-1; k>=zs; k--) {

	if (k == zs+zm-1) {
	  dof(k,j,i) = k;
	} else {
	  double dZ = depth(k,j,i) - depth(k+1,j,i);

	  if (dZ < 0.001) {
//  	  if (dZ < EPS1) {
	    dof(k,j,i) = dof(k+1,j,i);
	    maxNbCollapsedSegt = max(maxNbCollapsedSegt,((int)dof(k+1,j,i)-k));
	  } else {
	    dof(k,j,i) = k;
	  }

	}

      }

    }

  }
  
  double GlobalStencilWidth;
  double LocalStencilWidth = (double) maxNbCollapsedSegt;

  //  PetscGlobalMax(&LocalStencilWidth,&GlobalStencilWidth,PETSC_COMM_WORLD);
  MPI_Allreduce( &LocalStencilWidth, &GlobalStencilWidth, 1, MPIU_REAL, MPI_MAX, PETSC_COMM_WORLD);
  stencilWidth = (int) GlobalStencilWidth;

  if (( cauldron -> debug1 ) && ( FastcauldronSimulator::getInstance ().getRank () == 0 )) {
    PetscPrintf ( PETSC_COMM_WORLD, " Maximum number of degenerate segments: %d \n", stencilWidth );
  } 


}


//------------------------------------------------------------//

//
//
// This function should be removed when the dependancy on the depth array (for the 
// full FEM mesh) has been removed. It is used solely to enumerate the DOF array.
//
void CauldronCalculator::setDepths ( const DM  femGrid, 
                                     const bool includeBasement,
                                           Vec depths ) {

  using namespace Basin_Modelling;
  //
  //
  // The femGrid MUST have been allocated at this point.
  //
  int Z_Node_Count = 0;
  int* GlobalK;
  int I;
  int J;
  int K;
  int FEM_K_Index;
  int Number_Of_Segments;
  int xStart;
  int yStart;
  int zStart;
  int xCount;
  int yCount;
  int zCount;
  bool Include_Ghost_Values = true; 
  PETSC_3D_Array femDepths ( femGrid, depths, INSERT_VALUES, Include_Ghost_Values );

  Layer_Iterator layers;
  LayerProps_Ptr Current_Layer;

  if ( includeBasement ) {
     layers.Initialise_Iterator ( cauldron->layers, Ascending, Basement_And_Sediments, Active_Layers_Only );
  } else {
     layers.Initialise_Iterator ( cauldron->layers, Ascending, Sediments_Only, Active_Layers_Only );
  }

  // Copy the depths from all active layers to the depths array
  for ( layers.Initialise_Iterator (); ! layers.Iteration_Is_Done (); layers++ ) {
    Current_Layer = layers.Current_Layer ();

    Number_Of_Segments = Current_Layer->getNrOfActiveElements();
    GlobalK = new int [ Number_Of_Segments + 1 ];

    // Enumerate the GlobalK array with the Z-DOF number
    for ( K = 0; K <= Number_Of_Segments; K++ ){
      GlobalK [ K ] = Z_Node_Count;
      Z_Node_Count = Z_Node_Count + 1;
    }

    Z_Node_Count = Z_Node_Count - 1;

    // Get the size of the layer DA.
    DMDAGetCorners ( Current_Layer->layerDA, &xStart, &yStart, &zStart, &xCount, &yCount, &zCount );

    // Make a copy of the Layer_Depth array
    Current_Layer -> Current_Properties.Activate_Property ( Basin_Modelling::Depth );

    // Copy ALL the layer depths into the FEM Grid depths
    for ( I = xStart; I < xStart + xCount; I++ ) {

      for ( J = yStart; J < yStart + yCount; J++ ) {

        if ( cauldron->nodeIsDefined ( I, J )) {

          for ( K = zStart; K < zStart + zCount; K++ ) {
            FEM_K_Index = GlobalK[ K - zStart ];

            // Copy Layer Depth into the FEM Depth vector
            femDepths ( FEM_K_Index, J, I ) = Current_Layer -> Current_Properties ( Basin_Modelling::Depth, K, J, I );
          }

        }

      }

    }

    Current_Layer -> Current_Properties.Restore_Property ( Basin_Modelling::Depth );
    delete [] GlobalK;
  }

  femDepths.Restore_Global_Array ( Update_Excluding_Ghosts );
}

//------------------------------------------------------------//

void CauldronCalculator::setRealNodes ( const DM  femGrid, 
                                          Vec femDOFs, 
                                          Vec femRealNodes ) {

  using namespace Basin_Modelling;

  unsigned int  elementIndex;
  int  zNodeCount = 0;
  int* globalK;
  int  I;
  int  K;
  int  numberOfSegments;
  int  xStart;
  int  yStart;
  int  zStart;
  int  xCount;
  int  yCount;
  int  zCount;
  int  iPos;
  int  jPos;
  int  xIndex;
  int  yIndex;
  int  zIndex;
  bool includeGhostValues = true;

  double elementThickness [ 4 ];
  MatStencil elementDOFs [ 8 ];

  const CompoundLithology* elementLithology;
  bool normalElement;


  PETSC_3D_Array realNodes   ( femGrid, femRealNodes, INSERT_VALUES, includeGhostValues );
  PETSC_3D_Array nodalDOFs   ( femGrid, femDOFs, INSERT_VALUES, includeGhostValues );

  Layer_Iterator layers;
  LayerProps_Ptr currentLayer;

  layers.Initialise_Iterator ( cauldron->layers, Ascending, Sediments_Only, Active_Layers_Only );


  for ( layers.Initialise_Iterator (); ! layers.Iteration_Is_Done (); layers++ ) {
    currentLayer = layers.Current_Layer ();

    currentLayer -> Current_Properties.Activate_Property ( Basin_Modelling::Solid_Thickness, INSERT_VALUES, true );

    numberOfSegments = currentLayer->getNrOfActiveElements();
    globalK = new int [ numberOfSegments + 1 ];

    // globalK holds the DOF number in the z-direction IFF there were no degenerate segments
    for ( K = 0; K <= numberOfSegments; K++ ){
      globalK [ K ] = zNodeCount;
      zNodeCount = zNodeCount + 1;
    }

    // Subtract 1 here, because the bottom of the next layer (above) 
    // has the same DOF number as the top of the current layer.
    zNodeCount = zNodeCount - 1;

    // Get the size of the layer DA.
    DMDAGetCorners ( currentLayer->layerDA, &xStart, &yStart, &zStart, &xCount, &yCount, &zCount );

    // Why 'K <zCount - 1' ?
    // zCount - 1 because we are looping over the elements.
    for ( K = 0; K < zCount - 1; K++ ) {

      for ( elementIndex = 0; elementIndex < cauldron -> mapElementList.size (); elementIndex++ ) {

        // 'exists' here is if all the nodes of the element (in the plane) have valid data.
        if ( cauldron -> mapElementList [ elementIndex ].exists ) {

          iPos = cauldron->mapElementList [ elementIndex ].i [ 0 ];
          jPos = cauldron->mapElementList [ elementIndex ].j [ 0 ];

          // Copy segment lithology
          elementLithology = currentLayer->getLithology ( iPos, jPos );

          normalElement = false;

          // Get thicknesses of the 4 vertical segments that make up the element.
          // If all segments have zero-thickness then the element is not considered a 
          // normalElement (i.e.  normalElement = false)
          for ( I = 0; I < 4; I++ ) {
            iPos = cauldron->mapElementList [ elementIndex ].i [ I ];
            jPos = cauldron->mapElementList [ elementIndex ].j [ I ];

            elementThickness [ I ] = currentLayer -> Current_Properties  ( Basin_Modelling::Solid_Thickness, K, jPos, iPos );

            if ( elementThickness [ I ] == IBSNULLVALUE ) {
              elementThickness [ I ] = 0.0;
            }

            normalElement = normalElement || ( elementThickness [ I ] > 0.0 );
          }

          normalElement = normalElement && elementLithology->surfacePorosity () > 0.0;

          if ( normalElement ) {

            for ( I = 0; I < 8; I++ ) {
              zIndex = globalK [ K ] + ( I < 4 ? 1 : 0 );
              yIndex = cauldron->mapElementList [ elementIndex ].j [ I % 4 ];
              xIndex = cauldron->mapElementList [ elementIndex ].i [ I % 4 ];

              elementDOFs [ I ].i = xIndex;
              elementDOFs [ I ].j = yIndex;

              // Even though it is a 'normal_element' a check still needs 
              // to be performed to see if any segment has zero-thickness.
              // So that the DOF number in the Z-direction can be set.
              if ( zIndex == int ( nodalDOFs ( zIndex, yIndex, xIndex ))) {
                elementDOFs [ I ].k = zIndex;
              } else {
                // If a segment is zero-thickness (or is a part of a zero thickness stack of nodes)
                // then the DOF number must be set to the top most non-zero-thickness DOF number.
                elementDOFs [ I ].k = int ( nodalDOFs ( zIndex, yIndex, xIndex ));
              }

            }

            for ( I = 0; I < 8; I++ ) {
              realNodes ( elementDOFs [ I ].k, elementDOFs [ I ].j, elementDOFs [ I ].i ) = 1.0;
            }

          }

        }

      }

    }

#if 0
    // I am not sure why I added this block, but it seems to 
    // cause some problems with zero thickness parts of layers.
    for ( I = xStart; I < xStart + xCount; ++I ) {

      for ( J = yStart; J < yStart + yCount; ++J ) {

        for ( K = 0; K < zCount; K++ ) {

          if ( ! currentLayer->includedNodes ( I, J, K )) {
            realNodes ( globalK [ K ], J, I ) = 0.0;
          }

        }

      }

    }
#endif

    currentLayer -> Current_Properties.Restore_Property ( Basin_Modelling::Solid_Thickness );
    delete [] globalK;
  }

  realNodes.Restore_Global_Array ( Update_Including_Ghosts );
}

#if 0

void CauldronCalculator::setRealNodes ( const DM  femGrid, 
                                          Vec femDOFs, 
                                          Vec femRealNodes ) {

  using namespace Basin_Modelling;

  unsigned int  elementIndex;
  int  zNodeCount = 0;
  int* globalK;
  int  I;
  int  K;
  int  numberOfSegments;
  int  zCount;
  int  iPos;
  int  jPos;
  int  xIndex;
  int  yIndex;
  int  zIndex;
  bool includeGhostValues = true;

  double elementThickness [ 4 ];
  MatStencil elementDOFs [ 8 ];

  const CompoundLithology* elementLithology;
  bool normalElement;


  PETSC_3D_Array realNodes   ( femGrid, femRealNodes, INSERT_VALUES, includeGhostValues );
  PETSC_3D_Array nodalDOFs   ( femGrid, femDOFs, INSERT_VALUES, includeGhostValues );

  Layer_Iterator layers;
  LayerProps_Ptr currentLayer;

  layers.Initialise_Iterator ( cauldron->layers, Ascending, Sediments_Only, Active_Layers_Only );


  for ( layers.Initialise_Iterator (); ! layers.Iteration_Is_Done (); layers++ ) {
    currentLayer = layers.Current_Layer ();

    currentLayer -> Current_Properties.Activate_Property ( Basin_Modelling::Solid_Thickness, INSERT_VALUES, true );

    numberOfSegments = currentLayer->getNrOfActiveElements();
    globalK = new int [ numberOfSegments + 1 ];

    //
    // globalK holds the DOF number in the z-direction IFF there were no degenerate segments
    //
    for ( K = 0; K <= numberOfSegments; K++ ){
      globalK [ K ] = zNodeCount;
      zNodeCount = zNodeCount + 1;
    }

    //
    // Subtract 1 here, because the bottom of the next layer (above) 
    // has the same DOF number as the top of the current layer.
    //
    zNodeCount = zNodeCount - 1;

    //
    // Get the size of the layer DA.
    //
    DMDAGetCorners ( currentLayer->layerDA, PETSC_NULL, PETSC_NULL, PETSC_NULL, PETSC_NULL, PETSC_NULL, &zCount );

    //
    // Why 'K <zCount - 1' ?
    // zCount - 1 because we are looping over the elements.
    //
    for ( K = 0; K < zCount - 1; K++ ) {

      for ( elementIndex = 0; elementIndex < cauldron -> mapElementList.size (); elementIndex++ ) {

        //
        // 'exists' here is if all the nodes of the element (in the plane) have valid data.
        //
        if ( cauldron -> mapElementList [ elementIndex ].exists ) {

          iPos = cauldron->mapElementList [ elementIndex ].i [ 0 ];
          jPos = cauldron->mapElementList [ elementIndex ].j [ 0 ];

          //
          // Copy segment lithology
          //
          elementLithology = currentLayer->getLithology ( iPos, jPos );

          normalElement = false;

          //
          // Get thicknesses of the 4 vertical segments that make up the element.
          // If all segments have zero-thickness then the element is not considered a 
          //  normalElement (i.e.  normalElement = false)
          //
          for ( I = 0; I < 4; I++ ) {
            iPos = cauldron->mapElementList [ elementIndex ].i [ I ];
            jPos = cauldron->mapElementList [ elementIndex ].j [ I ];

            elementThickness [ I ] = currentLayer -> Current_Properties  ( Basin_Modelling::Solid_Thickness, K, jPos, iPos );

            if ( elementThickness [ I ] == IBSNULLVALUE ) {
              elementThickness [ I ] = 0.0;
            }

            normalElement = normalElement || ( elementThickness [ I ] > 0.0 );
          }

          normalElement = normalElement && elementLithology->surfacePorosity () > 0.0;

          //
          // Only if the element is a 'normalElement' are any non-dummy DOFs .
          //
          if ( normalElement ) {

            for ( I = 0; I < 8; I++ ) {
              zIndex = globalK [ K ] + ( I < 4 ? 1 : 0 );
              yIndex = cauldron->mapElementList [ elementIndex ].j [ I % 4 ];
              xIndex = cauldron->mapElementList [ elementIndex ].i [ I % 4 ];

              elementDOFs [ I ].i = xIndex;
              elementDOFs [ I ].j = yIndex;

              //
              // Even though it is a 'normal_element' a check still needs 
              // to be performed to see if any segment has zero-thickness.
              // So that the DOF number in the Z-direction can be set.
              //
              if ( zIndex == int ( nodalDOFs ( zIndex, yIndex, xIndex ))) {
                elementDOFs [ I ].k = zIndex;
              } else {
                //
                // If a segment is zero-thickness (or is a part of a zero thickness stack of nodes)
                // then the DOF number must be set to the top most non-zero-thickness DOF number.
                //
                elementDOFs [ I ].k = int ( nodalDOFs ( zIndex, yIndex, xIndex ));
              }

            }

            for ( I = 0; I < 8; I++ ) {
              realNodes ( elementDOFs [ I ].k, elementDOFs [ I ].j, elementDOFs [ I ].i ) = 1.0;
            }

          }

        }

      }

    }

    currentLayer -> Current_Properties.Restore_Property ( Basin_Modelling::Solid_Thickness );
    delete [] globalK;
  }

  realNodes.Restore_Global_Array ( Update_Including_Ghosts );
}

#endif

//------------------------------------------------------------//

int CauldronCalculator::createMatrixStructure ( const DM   Map_DA, 
                                                const DM   FEM_Grid_DA, 
                                                const Boolean2DArray& validNeedle,
                                                const Vec  Degrees_Of_Freedom,
                                                      Mat* J, 
                                                const int  stencilWidth ) {

  int                    ierr,xs,ys,nx,ny,i,j,slot,gxs,gys,gnx,gny;           
  int                    m,n,dim,s,*cols,k,nc,*rows,col,colz,cnt,l,p,*dnz,*onz;
  int                    istart,iend,jstart,jend,kstart,kend,zs,nz,gzs,gnz,ii,jj,kk,size;
  int                    dims[3],starts[3];
  MPI_Comm               comm;
  PetscScalar            *values;
  //  DAPeriodicType         wrap;
  DMDABoundaryType       wrapX;
  DMDABoundaryType       wrapY;
  DMDABoundaryType       wrapZ;
  ISLocalToGlobalMapping ltog;
  DMDAStencilType        st;

//    PetscFunctionBegin;
  /*     
         nc - number of components per grid point 
         col - number of colors needed in one direction for single component problem
  
  */
  ierr = DMDAGetInfo( FEM_Grid_DA, &dim, &m, &n, &p, 0, 0, 0, &nc, &s, &wrapX, &wrapY, &wrapZ, &st); CHKERRQ( ierr );
  col    = 2*s + 1;
  colz   = 2*s + 1 + 2 * stencilWidth;

//   double **nodeIsDefined = 0; 
//   Vec LocalNodeIsDefined;
//   DAGetLocalVector(Map_DA,&LocalNodeIsDefined);
//   DAGlobalToLocalBegin ( Map_DA, Node_Is_Defined, INSERT_VALUES, LocalNodeIsDefined );
//   DAGlobalToLocalEnd   ( Map_DA, Node_Is_Defined, INSERT_VALUES, LocalNodeIsDefined );
//   DAVecGetArray(Map_DA, LocalNodeIsDefined, (void **) &nodeIsDefined);

  double ***dof = 0;
  Vec LocalDOF;
  DMGetLocalVector( FEM_Grid_DA, &LocalDOF );
  DMGlobalToLocalBegin( FEM_Grid_DA, Degrees_Of_Freedom, INSERT_VALUES, LocalDOF );
  DMGlobalToLocalEnd( FEM_Grid_DA, Degrees_Of_Freedom, INSERT_VALUES, LocalDOF );
  DMDAVecGetArray( FEM_Grid_DA, LocalDOF, &dof );

  ierr = DMDAGetCorners( FEM_Grid_DA, &xs, &ys, &zs, &nx,& ny, &nz); CHKERRQ( ierr );
  ierr = DMDAGetGhostCorners( FEM_Grid_DA, &gxs, &gys, &gzs, &gnx, &gny, &gnz); CHKERRQ( ierr );
  
  ierr = PetscObjectGetComm(( PetscObject )FEM_Grid_DA, &comm); CHKERRQ( ierr );
  ierr = MPI_Comm_size( comm, &size ); CHKERRQ( ierr );

  /* create the matrix */
  /* create empty Jacobian matrix */
//   ierr = MatCreate(comm,nc*nx*ny*nz,nc*nx*ny*nz,PETSC_DECIDE,PETSC_DECIDE,J);CHKERRQ(ierr);  
  ierr = MatCreate( comm, J ); CHKERRQ( ierr );  
  MatSetSizes ( *J, nc * nx * ny * nz, nc * nx * ny * nz, PETSC_DECIDE, PETSC_DECIDE );
  ierr = PetscMalloc( col * col * colz * nc * nc * nc * sizeof( PetscScalar ), &values ); CHKERRQ( ierr );
  ierr = PetscMemzero( values, col * col * colz * nc * nc * nc * sizeof( PetscScalar )); CHKERRQ( ierr );
  ierr = PetscMalloc( nc * sizeof( int ), &rows ); CHKERRQ( ierr );
  ierr = PetscMalloc( col * col * colz * nc * sizeof( int ), &cols ); CHKERRQ( ierr );
  //  ierr = DAGetISLocalToGlobalMapping( FEM_Grid_DA,&ltog);CHKERRQ (ierr);
  ierr = DMGetLocalToGlobalMapping( FEM_Grid_DA, &ltog); CHKERRQ( ierr );

  int my_k_start, true_kk;

  /* determine the matrix preallocation information */
  ierr = MatPreallocateInitialize( comm, nc * nx * ny * nz, nc * nx * ny * nz, dnz, onz); CHKERRQ( ierr );
  for (i = xs; i < xs + nx; i ++) {
    istart = DAPeriodic( wrapX ) ? -s : ( PetscMax( -s, -i ));
    iend   = DAPeriodic( wrapX ) ?  s : ( PetscMin( s, m - i - 1 ));
    for (j = ys; j < ys + ny; j++ ) {
      jstart = DAPeriodic( wrapY ) ? -s : ( PetscMax( -s, -j )); 
      jend   = DAPeriodic( wrapY ) ?  s : ( PetscMin( s, n - j - 1 ));
      for (k = zs; k < zs + nz; k++ ) {

         if ( not validNeedle ( i, j )  or dof[k][j][i] != k ) {
//         if ( !nodeIsDefined[j][i] || (dof[k][j][i] != k) ) {
          slot = i - gxs + gnx * (j - gys) + gnx * gny * (k - gzs);
          l = 0; cnt = 0;
          rows[l]      = l + nc*(slot);
          cols[cnt++]  = rows[l];
          ierr = MatPreallocateSetLocal( ltog, nc, rows, ltog, cnt, cols, dnz, onz); CHKERRQ(ierr);
          continue;
        }
        my_k_start = k;
//         while ( (--my_k_start >= 0 ) && ( dof[my_k_start][j][i] != my_k_start ) );
        while ( true ) {
          if ( my_k_start == 0 ) break;
          my_k_start --;
          if ( dof[my_k_start][j][i] == my_k_start ) break;
        }
//         if (my_k_start != 0) {
//           do {
//             my_k_start -= 1;
//           } while (dof[my_k_start][j][i] != my_k_start);
//         }
        kstart = DAPeriodic( wrapZ ) ? -s : ( PetscMax( -( k - my_k_start ), -k )); 
        kend   = DAPeriodic( wrapZ ) ?  s : ( PetscMin( s, p - k - 1 ));
        
        slot = i - gxs + gnx * ( j - gys ) + gnx * gny * ( k - gzs );
        
        cnt  = 0;
        for ( l = 0; l < nc; l++ ) {
          for ( ii = istart; ii < iend + 1; ii ++ ) {
            for ( jj = jstart; jj < jend + 1; jj ++) {
              for (kk = kstart; kk < kend + 1; kk ++) {

                 if ( not validNeedle ( i + ii, j + jj )) {
//                 if (!nodeIsDefined[j+jj][i+ii]) {
                  true_kk = kk;
                  cols[cnt++]  = l + nc * ( slot + ii + gnx * jj + gnx * gny * true_kk );
                }
                else if ( st == DMDA_STENCIL_BOX ) {
                  if (( dof[k + kk][j + jj][i + ii] != k + kk ) && ( kk < 0 )) continue;
                  true_kk = int ( dof[k+kk][j+jj][i+ii] ) - int ( dof[k][j][i] );
                  cols[cnt++]  = l + nc * ( slot + ii + gnx * jj + gnx * gny * true_kk );
                }
              }
            }
          }
          rows[l] = l + nc*(slot);
        }
        ierr = MatPreallocateSetLocal(ltog,nc,rows,ltog,cnt,cols,dnz,onz);CHKERRQ(ierr);
      }
    }
  }
  /* set matrix type and preallocation */
  if (size > 1) {
    ierr = MatSetType(*J,MATMPIAIJ);CHKERRQ(ierr);
  } else {
    ierr = MatSetType(*J,MATSEQAIJ);CHKERRQ(ierr);
  }
  ierr = MatSeqAIJSetPreallocation(*J,0,dnz);CHKERRQ(ierr);  
  ierr = MatSeqBAIJSetPreallocation(*J,nc,0,dnz);CHKERRQ(ierr);  
  ierr = MatMPIAIJSetPreallocation(*J,0,dnz,0,onz);CHKERRQ(ierr);  
  ierr = MatMPIBAIJSetPreallocation(*J,nc,0,dnz,0,onz);CHKERRQ(ierr);  
  ierr = MatPreallocateFinalize(dnz,onz);CHKERRQ(ierr);
  ierr = MatSetLocalToGlobalMapping(*J,ltog,ltog);CHKERRQ(ierr);
  ierr = DMDAGetGhostCorners( FEM_Grid_DA,&starts[0],&starts[1],&starts[2],&dims[0],&dims[1],&dims[2]);CHKERRQ(ierr);
  ierr = MatSetStencil(*J,3,dims,starts,nc);CHKERRQ(ierr);

  /*
    For each node in the grid: we get the neighbors in the local (on processor ordering
    that includes the ghost points) then MatSetValuesLocal() maps those indices to the global
    PETSc ordering.
  */
  for (i=xs; i<xs+nx; i++) {
    istart = DAPeriodic(wrapX) ? -s : (PetscMax(-s,-i));
    iend   = DAPeriodic(wrapX) ?  s : (PetscMin(s,m-i-1));

    for (j=ys; j<ys+ny; j++) {
      jstart = DAPeriodic(wrapY) ? -s : (PetscMax(-s,-j)); 
      jend   = DAPeriodic(wrapY) ?  s : (PetscMin(s,n-j-1));

      for (k=zs; k<zs+nz; k++) {

         if ( not validNeedle ( i, j ) or (dof[k][j][i] != k) ) {
//         if ( !nodeIsDefined[j][i] || (dof[k][j][i] != k) ) {
          slot = i - gxs + gnx*(j - gys) + gnx*gny*(k - gzs);
          l = 0; cnt = 0;
          rows[l]      = l + nc*(slot);
          cols[cnt++]  = rows[l];
          ierr = MatSetValuesLocal(*J,nc,rows,cnt,cols,values,INSERT_VALUES);
          CHKERRQ(ierr);
          continue;
        }

        my_k_start = k;

        while ( true ) {
          if ( my_k_start == 0 ) break;
          my_k_start--;
          if ( dof[my_k_start][j][i] == my_k_start ) break;
        }

        kstart = DAPeriodic(wrapZ) ? -s : (PetscMax(-(k-my_k_start),-k)); 
        kend   = DAPeriodic(wrapZ) ?  s : (PetscMin(s,p-k-1));
        
        slot = i - gxs + gnx*(j - gys) + gnx*gny*(k - gzs);
        
        cnt  = 0;
        for (l=0; l<nc; l++) {
          for (ii=istart; ii<iend+1; ii++) {
            for (jj=jstart; jj<jend+1; jj++) {
              for (kk=kstart; kk<kend+1; kk++) {
                 if ( not validNeedle ( i + ii, j + jj )) {
//                 if (!nodeIsDefined[j+jj][i+ii]) {
                  true_kk = kk;
                  cols[cnt++]  = l + nc*(slot + ii + gnx*jj + gnx*gny*true_kk);
                }
                else if (st == DMDA_STENCIL_BOX) {
                  if ( (dof[k+kk][j+jj][i+ii] != k+kk) && (kk < 0) ) continue;
                  true_kk = int ( dof[k+kk][j+jj][i+ii]) - int ( dof[k][j][i] );
                  cols[cnt++]  = l + nc*(slot + ii + gnx*jj + gnx*gny*true_kk);
                }
              }
            }
          }
          rows[l] = l + nc*(slot);
        }
        ierr = MatSetValuesLocal(*J,nc,rows,cnt,cols,values,INSERT_VALUES);CHKERRQ(ierr);
      }
    }
  }

//   DAVecRestoreArray(Map_DA, LocalNodeIsDefined,  &nodeIsDefined);
//   DARestoreLocalVector(Map_DA,&LocalNodeIsDefined);

  DMDAVecRestoreArray(FEM_Grid_DA, LocalDOF,  &dof);
  DMRestoreLocalVector(FEM_Grid_DA,&LocalDOF);

  ierr = PetscFree(values);CHKERRQ(ierr);
  ierr = PetscFree(rows);CHKERRQ(ierr);
  ierr = PetscFree(cols);CHKERRQ(ierr);
  ierr = MatAssemblyBegin(*J,MAT_FINAL_ASSEMBLY);CHKERRQ(ierr);  
  ierr = MatAssemblyEnd(*J,MAT_FINAL_ASSEMBLY);CHKERRQ(ierr);  
  PetscFunctionReturn(0);
}

//------------------------------------------------------------//
