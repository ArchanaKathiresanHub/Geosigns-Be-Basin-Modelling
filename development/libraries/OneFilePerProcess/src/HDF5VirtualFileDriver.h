#ifndef ONEFILEPERPROCESS_HDF5VIRTUALFILEDRIVER_H
#define ONEFILEPERPROCESS_HDF5VIRTUALFILEDRIVER_H

// This Virtual File Driver for HDF5 allows to output in one file per process, 
// which will have better performance on non-parallel file systems.

#include <mpi.h>
#include <hdf5.h>

// Macro that can be used to identify the driver when using H5Pset_driver
#define H5FD_OFPP      (OFPP_init())

#ifdef __cplusplus
extern "C" {
#endif

   // Initialize the library. In general it is not necessary to call this
   // function
   hid_t OFPP_init(void);

   // Remove the library. In general it is not necessary to call this function
   void OFPP_term(void);


   // Set the OneFilePerProcessor file driver in the file access property list.
   // Parameters
   //    - fapl:  The file access property list
   //    - comm:  The MPI communicator. The function make a copy of the
   //             communicator, so changes to this object after this function
   //             call returns have no effect.
   //    - fileNameRewritePattern:
   //             A string specificying how to rewrite file names. The resulting
   //             file name will be this pattern in which the following placeholder
   //             are substituted by specific values.
   //               {NAME}     is substituted by the entire original file name
   //               {MPI_RANK} is substituted by the MPI rank of this process. 
   //               {MPI_SIZE} is substituted by total number of MPI processes.
   void H5Pset_fapl_ofpp( hid_t fapl, MPI_Comm comm, const char * fileNameRewritePattern, hbool_t useGpfs) ;

#ifdef __cplusplus
} // extern "C"
#endif


#endif
