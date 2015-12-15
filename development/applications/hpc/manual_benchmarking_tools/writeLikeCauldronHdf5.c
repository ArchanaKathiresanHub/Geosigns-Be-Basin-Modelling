#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <hdf5.h>
#include <string.h>


void carthesian( int P, int * Px, int * Py)
{
   int fitness = 1;
   *Px = P;
   *Py = 1;

   for (int py = 1; py <= P; ++py)
   {
      if (P % py == 0)
      {
         int px = P / py;

         int currentFitness = px < py ? px : py;

         if (currentFitness > fitness )
         {
            fitness = currentFitness;
            *Px = px;
            *Py = py;
         }
      }
   }
}

#define DIM  5
typedef enum  Order_ { TimeFirst, SpaceFirst } Order;

void reorder( hsize_t * array, Order order )
{
   if (order == SpaceFirst)
   {
      for (int i = 0; i < DIM/2; ++i)
      {
         hsize_t tmp = array[i];
         array[i] = array[DIM-i-1];
         array[DIM-i-1] = tmp;
      }
   }
}

   

int main( int argc, char ** argv)
{
   // Init MPI
   MPI_Init( &argc, &argv );

   // Parse command line
   assert( argc >= 3 && "usage: hdf5LikeCauldron ALIGN ORDER");
   hsize_t alignment = atoi(argv[1]);
   Order order = strcmp(argv[2], "space")==0? SpaceFirst : TimeFirst;


   // Determine topology

   MPI_Comm comm = MPI_COMM_WORLD;

   int procs;
   MPI_Comm_size( comm, &procs );

   int procsI, procsJ;
   carthesian( procs, & procsI, & procsJ);

   int rank = 0;
   MPI_Comm_rank( comm, &rank);
   int rankI = rank % procsI;
   int rankJ = rank / procsI;


   // Time domain
   double T0 = 200; // Ma
   double T1 = 0; // Ma
   int timeSteps = 100;

   // Properties
   int props = 5; // 5 continuous and 5 discontinuous properties

   // Physical domain
   int I=600, J=500;
   int L = 10;
   int layer_offsets[11] = { 0, 40, 50, 55, 60, 80, 81, 82, 90, 92,100 };
   double ages[11] = {200, 200, 190, 180, 100, 10, 3, 2, 1, 0.5, 0.0};

   // Domain decomposition
   int I_chunk = (I+procsI-1)/procsI;
   int I_local = (rankI < procsI-1) ? I_chunk : (I - (procsI-1) * I_chunk);
   int I_offset = I_chunk * rankI;

   int J_chunk = (J+procsJ-1)/procsJ;
   int J_local = (rankJ < procsJ-1) ? J_chunk : (J - (procsJ-1) * J_chunk);
   int J_offset = J_chunk * rankJ;

   assert( I_local >= 0);
   assert( J_local >= 0);

   if (rank == 0)
      printf("DOMAIN DECOMPOSITION\n");
   for (int p = 0; p < procs; ++p)
   {
      if (p == rank)
      {
         printf("rank = %2d, I_local = %3d, J_local = %3d, I_offset = %3d, J_offset = %3d\n",
               rank, I_local, J_local, I_offset, J_offset
               );
         fflush(stdout);
      }

      MPI_Barrier(comm);
   }
   if (rank == 0)
      printf("\n\nWRITING FILE\n");

   // Allocate Grids
   double *temperature = malloc(sizeof(double)*I_local*J_local*layer_offsets[10]*props);
   double *porosity= malloc(sizeof(double)*I_local*J_local*(layer_offsets[10] + L)*props);


   // Init Hdf5
   hid_t fapl = H5Pcreate(H5P_FILE_ACCESS);
   H5Pset_fapl_mpio(fapl, comm, MPI_INFO_NULL);
   H5Pset_alignment(fapl, 4096, alignment);

   // match chunks with domain decomposition
   hid_t dcpl = H5Pcreate(H5P_DATASET_CREATE);
   hsize_t chunk[DIM] = { 1, props, layer_offsets[L], J_chunk, I_chunk }; reorder(chunk, order);
   H5Pset_chunk(dcpl, DIM, chunk );

   // Set I/O independent
   hid_t dxpl = H5Pcreate(H5P_DATASET_XFER);
   H5Pset_dxpl_mpio(dxpl, H5FD_MPIO_INDEPENDENT);
//   H5Pset_dxpl_mpio(dxpl, H5FD_MPIO_COLLECTIVE);

   // create file and dataset
   hsize_t globalC[DIM] = { timeSteps, props, layer_offsets[L], J, I }; reorder(globalC, order);
   hsize_t globalD[DIM] = { timeSteps, props, layer_offsets[L] + L, J, I }; reorder(globalD, order);
   hid_t globalSpaceC = H5Screate_simple(DIM, globalC, NULL);
   hid_t globalSpaceD = H5Screate_simple(DIM, globalD, NULL);
   hid_t file = H5Fcreate( "testdata.h5", H5F_ACC_TRUNC, H5P_DEFAULT, fapl);
   hid_t datasetC = H5Dcreate( file, "temperature", H5T_NATIVE_DOUBLE, globalSpaceC, H5P_DEFAULT, dcpl, H5P_DEFAULT);
   hid_t datasetD = H5Dcreate( file, "porosity", H5T_NATIVE_DOUBLE, globalSpaceD, H5P_DEFAULT, dcpl, H5P_DEFAULT);
   
   // Do the main loop.
   for (int t = 0; t < timeSteps; ++t )
   {
      double age = T0 - t * (T0-T1) / (timeSteps - 1);

      // determine layer number and K
      int l = 0;
      for (int i = 0; i < L; ++i)
      {
         if (ages[i] < age)
            break;
         l = i;
      }
      assert( age <= ages[l] );
      int K = layer_offsets[l] + (double) (layer_offsets[l+1]-layer_offsets[l]) * (ages[l]-age)/(ages[l]-ages[l+1]);
      int Kl = K + l;

      if (rank == 0)
      {
         printf("Age=%f, Depositing layer %d. K = %d, Kl = %d\n", age, l, K, Kl);
      }
      

      // fill grids with data
      for (int i = 0; i < I_local*J_local*K*props; ++i)
         temperature[i] = (double) i / age;

      for (int i = 0; i < I_local*J_local*Kl*props; ++i)
         porosity[i] = age / i;

   
      // Write file
      hsize_t blockC[DIM] = {1, props, K, J_local, I_local }; reorder(blockC, order); // continuous data
      hsize_t blockD[DIM] = {1, props, Kl, J_local, I_local }; reorder(blockD, order); // discontinuous data
      hsize_t offset[DIM] = {t, 0, 0, J_offset, I_offset }; reorder(offset, order);
      hsize_t localSpaceC = H5Screate_simple( DIM, blockC, NULL);
      hsize_t localSpaceD = H5Screate_simple( DIM, blockD, NULL);

      H5Sselect_hyperslab( globalSpaceC, H5S_SELECT_SET, offset, NULL, blockC, NULL);
      H5Dwrite( datasetC, H5T_NATIVE_DOUBLE, localSpaceC, globalSpaceC, dxpl, temperature);

      H5Sselect_hyperslab( globalSpaceD, H5S_SELECT_SET, offset, NULL, blockD, NULL);
      H5Dwrite( datasetD, H5T_NATIVE_DOUBLE, localSpaceD, globalSpaceD, dxpl, porosity);

      H5Sclose( localSpaceC );
      H5Sclose( localSpaceD );
   }

   // Close everything
   H5Dclose( datasetC );
   H5Dclose( datasetD );
   H5Fclose( file );
   H5Sclose( globalSpaceC );
   H5Sclose( globalSpaceD );
   H5Pclose( fapl );
   H5Pclose( dxpl );
   H5Pclose( dcpl );

   free(porosity);
   free(temperature);

   MPI_Finalize();
   return 0;
}

