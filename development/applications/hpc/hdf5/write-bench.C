#include <iostream>
#include <sstream>
#include <cmath>
#include <iomanip>
using namespace std;


// FIX errors raied by C++ binding of MPI
#ifdef SEEK_SET
#undef SEEK_SET
#endif

#ifdef SEEK_CUR
#undef SEEK_CUR
#endif

#ifdef SEEK_END
#undef SEEK_END
#endif

#include "mpi.h"
#include "hdf5.h"
#include "H5FDmpio.h"
#include "stdlib.h"

#define H5FILE_NAME     "Output.h5"
#define DATASETNAME     "IntArray"

// Number of dimensions of the datasets
#ifndef DIM
  #define DIM    4
#endif

// Size of each dataset
#ifndef N
  #define N  { 80, 80, 60, 1 }
#endif

// Size of chunks
#ifndef C
  #define C  { 4, 3, 5, 1 }
#endif

// Number of datasets per file
#ifndef D
  #define D 3
#endif

// Number of files
#ifndef F
  #define F 1
#endif

// Whether to enable chunking in HDF5
#ifndef HAVE_CHUNKING
  #define HAVE_CHUNKING  true
#endif

// Alignment of HDF5 objects
#ifndef ALIGN
  #define ALIGN (1<<12)
#endif

// Size number of procs in direction.
// Rank "rank" in direction
// dimension number of elements in direction
int arraySize ( int size, int rank, int dimension );

// Size number of procs in direction.
// Rank "rank" in direction
// dimension number of elements in direction
int startPosition ( int size, int rank, int dimension );

// Size is number of processors in total
// pnx will be the number of processors in x
// pny will be the number of processors in y
void procDivision ( int size, int& pnx, int& pny );

int main (int argc, char **argv) {
    /*
     * HDF5 APIs definitions
     */
    hid_t       file_id, dset_id;         /* file and dataset identifiers */
    hid_t       filespace, memspace;      /* file and memory dataspace identifiers */
    hsize_t     dimsf[DIM] = N;                 /* dataset dimensions */
    hsize_t     count[DIM];               /* hyperslab selection parameters */
    hsize_t     offset[DIM];
    hsize_t     chunk[DIM] = C;
    int         i;
    herr_t      status;

    int procs [ DIM ];

    int subranks [ DIM ];

    /*
     * MPI variables
     */
    int mpi_size, mpi_rank;
    MPI_Comm comm  = MPI_COMM_WORLD;
    MPI_Info info  = MPI_INFO_NULL;

    /*
     * Initialize MPI
     */
    MPI_Init(&argc, &argv);
    MPI_Comm_size(comm, &mpi_size);
    MPI_Comm_rank(comm, &mpi_rank);

    // Set default distribution
    for (int d = 0; d < DIM; ++d)
    {
       procs [ d ] = 1;
       subranks [ d ] = 0;
    }

    // Compute carthesian block distribution on X,Y direction
    procDivision ( mpi_size, procs [ 1 ], procs [ 0 ] );
    subranks [ 0 ] = mpi_rank % procs [ 0 ];
    subranks [ 1 ] = ( mpi_rank - subranks [ 0 ])  / procs [ 0 ];

    /*
     * Set up file access property list with parallel I/O access
     */
    hid_t fileAccessPList = H5Pcreate(H5P_FILE_ACCESS);

        MPI_Info_create(& info);

        MPI_Info_set (info, "collective_buffering", "false");
        MPI_Info_set (info, "romio_ds_read", "disable");
        MPI_Info_set (info, "romio_ds_write", "disable");

        MPI_Info_set (info, "romio_cb_read", "disable");
        MPI_Info_set (info, "romio_cb_write", "disable");

//      MPI_Info_set (info, "cb_nodes", "1");

    H5Pset_fapl_mpio(fileAccessPList, comm, info);

    // Set alignment
    H5Pset_alignment(fileAccessPList, 4096, ALIGN);


    /*
     * Create the dataspace for the dataset.
     */

    hid_t dcpl;
    if ( HAVE_CHUNKING ) {
       dcpl = H5Pcreate (H5P_DATASET_CREATE);
       status = H5Pset_chunk (dcpl, DIM, chunk);
    }
    else
    {
       dcpl = H5P_DEFAULT;
    }


    for (int d = 0; d < DIM; ++d)
    {
       count [ d ] = arraySize ( procs [ d ], subranks [ d ], dimsf [ d ]);
       offset [ d ] = startPosition ( procs [ d ], subranks [ d ], dimsf [ d ]);
    }


#ifdef DEBUG
    if ( mpi_rank == 0 ) {

       cout << " dims   "  << mpi_rank << "   "
            << dimsf [ 0 ] << "  "
            << dimsf [ 1 ] << "  "
            << dimsf [ 2 ] << "  "
            << endl << endl;
    }

    MPI_Barrier ( comm );

    for ( int ii = 0; ii < mpi_size; ++ii ) {

       if ( ii == mpi_rank ) {

          cout << " count  "  << mpi_rank << "   "
               << count [ 0 ] << "  "
               << count [ 1 ] << "  "
               << count [ 2 ] << "  "
               << endl;

          cout << " offset "  << mpi_rank << "   "
               << offset [ 0 ] << "  "
               << offset [ 1 ] << "  "
               << offset [ 2 ] << "  "
               << endl << endl;
       }

       MPI_Barrier ( comm );

    }
#endif

    /*
     * Create property list for collective dataset write.
     */
    hid_t DatasetTransferPList= H5Pcreate(H5P_DATASET_XFER);

    H5Pset_dxpl_mpio(DatasetTransferPList, H5FD_MPIO_COLLECTIVE);

    /*
     * Initialize data buffer
     */
    size_t totalSize = 1;
    for (int d = 0; d < DIM; ++d)
      totalSize *= count[d];

    int * data = new int[totalSize];
    for (i=0; i < totalSize; i++) {
       data[i] = mpi_rank + 10;
    }


    MPI_Barrier(MPI_COMM_WORLD);
    const double t0 = MPI_Wtime( );

    for (int f = 0; f < F; ++f)
    {
       /*
        * Create a new file collectively and release property list identifier.
        */
       std::stringstream fileName;
       fileName << std::setfill('0') << std::setw(4) << f << "-" << H5FILE_NAME;
       file_id = H5Fcreate(fileName.str().c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, fileAccessPList);
       int numberOfDataSets = D;
       for ( i = 1; i <= numberOfDataSets; ++i ) {

          std::stringstream dataSet;

          dataSet << "IntArray_" << i;

          filespace = H5Screate_simple(DIM, dimsf, NULL);

          /*
           * Create the dataset with default properties and close filespace.
           */

          // dset_id = H5Dcreate(file_id, DATASETNAME, H5T_NATIVE_INT, filespace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
          dset_id = H5Dcreate(file_id, dataSet.str ().c_str (), H5T_NATIVE_INT, filespace, H5P_DEFAULT, dcpl, H5P_DEFAULT );
          // dset_id = H5Dcreate(file_id, DATASETNAME, H5T_NATIVE_INT, filespace, H5P_DEFAULT, dcpl, H5P_DEFAULT );
          H5Sclose(filespace);

          /*
           * Each process defines dataset in memory and writes it to the hyperslab
           * in the file.
           */

          memspace = H5Screate_simple(DIM, count, NULL);

          /*
           * Select hyperslab in the file.
           */
          filespace = H5Dget_space(dset_id);
          H5Sselect_hyperslab(filespace, H5S_SELECT_SET, offset, NULL, count, NULL);


          status = H5Dwrite(dset_id, H5T_NATIVE_INT, memspace, filespace, DatasetTransferPList, data);

          /*
           * Close/release resources.
           */
          H5Dclose(dset_id);
          H5Sclose(filespace);
          H5Sclose(memspace);
       }
       H5Fclose(file_id);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    const double t1 = MPI_Wtime();

    if (mpi_rank == 0)
    {
       std::cout << "P = " << mpi_size << std::endl;
       std::cout << "WRITE T = " << (t1 - t0 ) << std::endl;
    }

    delete [] data;
    H5Pclose(DatasetTransferPList);
    H5Pclose(fileAccessPList);
    MPI_Finalize();

    return 0;
}


int arraySize ( int size, int rank, int dimension ) {
   return int ( std::floor (( dimension + size - rank - 1 ) / size ));
}

int startPosition ( int size, int rank, int dimension ) {

   int start = 0;
   int i;

   for ( i = 0; i < rank; ++i ) {
      start += arraySize ( size, i, dimension );
   }

   return start;
}


void procDivision ( int size, int& pnx, int& pny ) {

   int sqrtSize = int ( std::sqrt ( size ));
   int i;

   pnx = size;
   pny = 1;

   for ( i = 2; i <= sqrtSize; ++i ) {

      if ( size % i == 0 ) {
         pnx = size / i;
         pny = i;
      }

   }

}

