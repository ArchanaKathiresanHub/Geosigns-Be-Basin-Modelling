#ifndef ONEFILEPERPROCESS_HDF5VIRTUALFILEDRIVER_H
#define ONEFILEPERPROCESS_HDF5VIRTUALFILEDRIVER_H

#include <mpi.h>
#include <H5public.h>


#define H5FD_OFPP      (OFPP_init())

#ifdef __cplusplus
extern "C" {
#endif

   // pattern: Rule how to rewrite file names:
   // {NAME}: The complete string given as file name
   // {MPI_RANK}: replaced by MPI RANK
   // {MPI_SIZE}: replaced by total number of processors
   // evertyhing else is copied verbatim

   // Ideas to implement later
   // {DIR}: everything before the the last directory separator (e.g. '/') if it includes one
   // {FILE}: The file name excluding the path
   // {BASE}: The file name exclusing the path and extension ( everything after the last '.')
   // {EXT}: The extension excluding the '.'

   hid_t OFPP_init(void);
   void OFPP_term(void);
   void H5Pset_fapl_ofpp( hid_t fapl, MPI_Comm comm, const char * fileNameRewritePattern, hbool_t useGpfs) ;
   void H5Pget_fapl_ofpp( hid_t fapl, MPI_Comm *comm, const char ** fileNameRewritePattern, hbool_t * useGpfs);

#ifdef __cplusplus
} // extern "C"
#endif


#endif
