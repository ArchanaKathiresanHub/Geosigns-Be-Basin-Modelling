#include <iostream>
#include <sstream>
#include <cmath>
#include <iomanip>
using namespace std;

// Make sure the C++ MPI bindings don't panic
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

// Number of files to read
#ifndef F
  #define F 1
#endif



static const int MAX_DIMENSION = 10;

struct AppCtx {

   int size;
   int rank;
   int valCount;

   int spatialDimension;
   int procs [ MAX_DIMENSION ];
   int subranks [ MAX_DIMENSION ];

   int* data;

   hid_t file_id;

   hsize_t dimensions [ MAX_DIMENSION ];
   hsize_t count [ MAX_DIMENSION ];
   hsize_t offset [ MAX_DIMENSION ];

   // hsize_t chunk [ MAX_DIMENSION ];

};

static herr_t readDimension ( hid_t groupId, const char* name, void * voidReader);

static herr_t readDataset ( hid_t groupId, const char* name, void * voidReader);

static const int MAX_DIMENSION = 10;

struct AppCtx {

   int size;
   int rank;
   int valCount;

   int spatialDimension;
   int procs [ MAX_DIMENSION ];
   int subranks [ MAX_DIMENSION ];

   int* data;

   hid_t file_id;

   hsize_t dimensions [ MAX_DIMENSION ];
   hsize_t count [ MAX_DIMENSION ];
   hsize_t offset [ MAX_DIMENSION ];

   // hsize_t chunk [ MAX_DIMENSION ];

};

static herr_t readDimension ( hid_t groupId, const char* name, void * voidReader);

static herr_t readDataset ( hid_t groupId, const char* name, void * voidReader);


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

   AppCtx reader;

    /*
     * MPI variables
     */
    MPI_Comm comm  = MPI_COMM_WORLD;
    MPI_Info info  = MPI_INFO_NULL;

    /*
     * Initialize MPI
     */
    MPI_Init(&argc, &argv);
    MPI_Comm_size(comm, &reader.size);
    MPI_Comm_rank(comm, &reader.rank);

    // Set default distribution
    for (int d = 0; d < MAX_DIMENSION; ++d)
    {
       reader.procs [ d ] = 1;
       reader.subranks [ d ] = 0;
    }

    // Compute carthesian block distribution on X,Y direction
    procDivision ( reader.size, reader.procs [ 1 ], reader.procs [ 0 ] );
    reader.subranks [ 0 ] = reader.rank % reader.procs [ 0 ];
    reader.subranks [ 1 ] = ( reader.rank - reader.subranks [ 0 ])  / reader.procs [ 0 ];


    /*
     * Set up file access property list with parallel I/O access
     */
    hid_t fileAccessPList = H5Pcreate(H5P_FILE_ACCESS);
    H5Pset_fapl_mpio(fileAccessPList, comm, info);

    MPI_Barrier(MPI_COMM_WORLD);
    double t0 = MPI_Wtime();

    for (int f = 0; f < F; ++f)
    {
       /*
        * Create a new file collectively and release property list identifier.
        */
       std::stringstream fileName;
       fileName << std::setfill('0') << std::setw(4) << f << "-" << H5FILE_NAME;
       reader.file_id = H5Fopen(fileName.str().c_str(), H5F_ACC_RDONLY, fileAccessPList);

       // Compute dimensions
       H5Giterate ( reader.file_id, "/", 0, readDimension, &reader );

       for (int d = 0; d < reader.spatialDimension; ++d)
       {
          reader.count [ d ] = arraySize ( reader.procs [ d ], reader.subranks [ d ], reader.dimensions [ d ]);
          reader.offset [ d ] = startPosition ( reader.procs [ d ], reader.subranks [ d ], reader.dimensions[ d ]);
       }


       /*
        * Initialize data buffer
        */
       reader.valCount = 1;
       for (int d = 0; d < reader.spatialDimension; ++d)
         reader.valCount *= reader.count[d];

       reader.data = new int[reader.valCount];
       for (int i=0; i < reader.valCount; i++) {
          reader.data[i] = reader.valCount + 10;
       }

       H5Giterate ( reader.file_id, "/", 0, readDataset, &reader );

       delete [] reader.data;
       H5Fclose(reader.file_id);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    const double t1 = MPI_Wtime();

    if (reader.rank == 0)
    {
       std::cout << "P = " << reader.size << std::endl;
       std::cout << "READ T = " << (t1 - t0) << std::endl;
    }

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

int endPosition ( int size, int rank, int dimension ) {

   int end = 0;
   int i;

   for ( i = 0; i <= rank; ++i ) {
      end += arraySize ( size, i, dimension );
   }

   return end - 1;
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

herr_t readDimension ( hid_t groupId, const char* name, void * voidReader)  {

   AppCtx* reader = (AppCtx*)(voidReader);

   hid_t dset = H5Dopen ( reader->file_id, name, H5P_DEFAULT);
   hid_t dataSpaceId = H5Dget_space (dset);
   reader->spatialDimension = H5Sget_simple_extent_dims (dataSpaceId, reader->dimensions, 0);

   H5Dclose ( dset );
   return 1;
}


herr_t readDataset ( hid_t groupId, const char* name, void * voidReader)  {

   AppCtx* reader = (AppCtx*)(voidReader);

   hid_t memspace, filespace;
   hid_t dset_id;
   hid_t plist_id;
   hid_t status;

   dset_id = H5Dopen ( reader->file_id, name, H5P_DEFAULT );
   memspace = H5Screate_simple(reader->spatialDimension, reader->count, NULL);
   filespace = H5Dget_space(dset_id);
   H5Sselect_hyperslab(filespace, H5S_SELECT_SET, reader->offset, NULL, reader->count, NULL);
   plist_id = H5Pcreate(H5P_DATASET_XFER);

   H5Pset_dxpl_mpio(plist_id, H5FD_MPIO_COLLECTIVE);

   status = H5Dread (dset_id, H5T_NATIVE_INT, memspace, filespace,
                     plist_id, reader->data);

   H5Dclose(dset_id);
   H5Sclose(filespace);
   H5Sclose(memspace);
   H5Pclose(plist_id);
   return 0;
}

