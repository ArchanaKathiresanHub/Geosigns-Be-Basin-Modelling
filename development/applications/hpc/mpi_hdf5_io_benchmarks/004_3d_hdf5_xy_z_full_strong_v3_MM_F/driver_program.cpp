
//=====================================================//
// 3D HDF5 parallel I/O                                //
// Strong scaling                                      //
// WORKS ONLY FOR MPI PROCESSES : 4, 16, 64, 256, 1024 //
//=====================================================//

#include "mpi.h"
#include "hdf5.h"

#include <iostream>
#include <cmath>
#include <sstream>
#include <string>

using std::endl;
using std::cout;
using std::pow;
using std::string;
using std::ostringstream;

// the main function

int main (int argc, char **argv)
{
     // MPI variables

     int mpi_size;
     int mpi_rank;
     MPI_Comm comm  = MPI_COMM_WORLD;
     MPI_Info info  = MPI_INFO_NULL;

     // Initialize MPI

     MPI_Init(&argc, &argv);
     MPI_Comm_size(comm, &mpi_size);
     MPI_Comm_rank(comm, &mpi_rank);

     // local parameters

     const int I_DO_MAX = 1;
     const int NUM_FILES = 100;

     // local variables

     herr_t  status;

     // benchmark loop

     for (int iKK = 0; iKK != I_DO_MAX; ++iKK) {
          for (int iL = 0; iL != NUM_FILES; iL++) {

               if (mpi_rank == 0) {
                    cout << "---------------------------------------------------->> "
                         << iKK << " , " << iL << endl;
               }

               // local parameters

               const string H5FILE_NAME_BASE = "SDS_chnk_";
               const char * DATASETNAME = "DoubleArray";
               const int COEF_A = iL+1;
               const int NX = COEF_A * 256;
               const int NY = COEF_A * 256;
               const int NZ = 10;
               const int CH_NZ = NZ;
               const int RANK = 3;

               // HDF5 APIs definitions

               hid_t     file_id, dset_id;     // file and dataset identifiers
               hid_t     filespace, memspace;  // file and memory dataspace identifiers
               hsize_t   dimsf[RANK];          // dataset dimensions
               hsize_t   chunk_dims[RANK];     // chunk dimensions
               double    *data;                // pointer to data buffer to write
               hsize_t	count[RANK];	       // hyperslab selection parameters
               hsize_t	stride[RANK];
               hsize_t	block[RANK];
               hsize_t	offset[RANK];
               hid_t	plist_id;             // property list identifier
               int       i;

               // Set up file access property list with parallel I/O access

               plist_id = H5Pcreate(H5P_FILE_ACCESS);
               H5Pset_fapl_mpio(plist_id, comm, info);

               // Create a new file collectively and release property list identifier

               ostringstream ss_a;
               ss_a << iL;
               string tmp_str_a = ss_a.str();
               string H5FILE_NAME = H5FILE_NAME_BASE + tmp_str_a + ".h5";

               file_id = H5Fcreate(H5FILE_NAME.c_str(),
                                   H5F_ACC_TRUNC,
                                   H5P_DEFAULT,
                                   plist_id);

               H5Pclose(plist_id);

               // Create the dataspace for the dataset

               dimsf[0] = NX;
               dimsf[1] = NY;
               dimsf[2] = NZ;

               const int pSqRoot = pow(mpi_size,0.5);

               const int CH_NX = NX/pSqRoot;
               const int CH_NY = NY/pSqRoot;

               chunk_dims[0] = CH_NX;
               chunk_dims[1] = CH_NY;
               chunk_dims[2] = CH_NZ;

               filespace = H5Screate_simple(RANK, dimsf, NULL);
               memspace  = H5Screate_simple(RANK, chunk_dims, NULL);

               // Create chunked dataset

               plist_id = H5Pcreate(H5P_DATASET_CREATE);

               H5Pset_chunk(plist_id, RANK, chunk_dims);

               dset_id = H5Dcreate2(file_id,
                                    DATASETNAME,
                                    H5T_NATIVE_DOUBLE,
                                    filespace,
                                    H5P_DEFAULT,
                                    plist_id,
                                    H5P_DEFAULT);

               H5Pclose(plist_id);
               H5Sclose(filespace);

               // Each process defines dataset in memory and writes it to the hyperslab
               // in the file

               count[0] = 1;
               count[1] = 1;
               count[2] = 1;

               stride[0] = 1;
               stride[1] = 1;
               stride[2] = 1;

               block[0] = chunk_dims[0];
               block[1] = chunk_dims[1];
               block[2] = chunk_dims[2];

               offset[0] = (mpi_rank%pSqRoot) * CH_NX;
               offset[1] = (mpi_rank/pSqRoot) * CH_NY;
               offset[2] = 0;

               // Select hyperslab in the file

               filespace = H5Dget_space(dset_id);

               status = H5Sselect_hyperslab(filespace,
                                            H5S_SELECT_SET,
                                            offset,
                                            stride,
                                            count,
                                            block);

               // Initialize data buffer

               const int TOTAL_ELEM = chunk_dims[0]*chunk_dims[1]*chunk_dims[2];

               data = new double [TOTAL_ELEM];

               for (i = 0; i != TOTAL_ELEM; ++i) {
                    data[i] = static_cast<double>(mpi_rank + 1);
               }

               // Create property list for collective dataset write

               plist_id = H5Pcreate(H5P_DATASET_XFER);

               H5Pset_dxpl_mpio(plist_id,
                                H5FD_MPIO_COLLECTIVE);

               status = H5Dwrite(dset_id,
                                 H5T_NATIVE_DOUBLE,
                                 memspace,
                                 filespace,
                                 plist_id,
                                 data);

               MPI_Barrier(comm);

               delete [] data;

               // Close/release resources

               H5Dclose(dset_id);
               H5Sclose(filespace);
               H5Sclose(memspace);
               H5Pclose(plist_id);
               H5Fclose(file_id);

               // remove file

               if (mpi_rank == 0) {
                    status = system("rm SDS_chnk_*.h5");
               }
          }
     }

     MPI_Finalize();

     return status;
}

// end
