#include <petsc.h>
#include <hdf5.h>

#include <H5FDmpio.h>
#include "h5_parallel_file_types.h"

#include "FilePath.h"

#ifndef _MSC_VER
#include "HDF5VirtualFileDriver.h"
#include "h5merge.h"
#endif


bool        H5_Parallel_PropertyList :: s_oneFilePerProcess = false;
bool        H5_Parallel_PropertyList :: s_primaryPod = false;

std::string H5_Parallel_PropertyList :: s_temporaryDirName;

MPI_Info    H5_Parallel_PropertyList :: s_mpiInfo = MPI_INFO_NULL;


hid_t H5_Parallel_PropertyList :: createFilePropertyList() const 
{
   hid_t plist = H5Pcreate (H5P_FILE_ACCESS);
   
#ifndef _MSC_VER
   if( s_oneFilePerProcess ) 
   {
      ibs::FilePath fileName( s_temporaryDirName );
      fileName << "{NAME}_{MPI_RANK}";
       
      H5Pset_fapl_ofpp (plist, PETSC_COMM_WORLD, fileName.cpath(), 0); 
   } 
   else 
   {
      H5Pset_fapl_mpio (plist, PETSC_COMM_WORLD, s_mpiInfo);  
   }
#else
	  H5Pset_fapl_mpio (plist, PETSC_COMM_WORLD, s_mpiInfo);  
#endif
   return plist;
}

hid_t H5_Parallel_PropertyList :: createDatasetPropertyList() const
{
   // set parallel read/write on file
   hid_t pList = H5P_DEFAULT;
   
   if( not s_oneFilePerProcess ) 
   {
      pList = H5Pcreate (H5P_DATASET_XFER);
      H5Pset_dxpl_mpio (pList, H5FD_MPIO_COLLECTIVE);
   } 
   
   return pList;
}

bool H5_Parallel_PropertyList :: setOneFilePerProcessOption( )
{
   PetscBool noOfpp = PETSC_FALSE;
   PetscBool primaryPod = PETSC_FALSE;
  
#ifndef _MSC_VER
   PetscOptionsHasName ( PETSC_NULL, "-noofpp", &noOfpp );
   
   if( !noOfpp ) {   

      const char * tmpDir = 0; 
      
      char temporaryDirName [ PETSC_MAX_PATH_LEN ];
      memset ( temporaryDirName, 0, PETSC_MAX_PATH_LEN );
      
      PetscBool oneFilePerProcess;
      PetscOptionsGetString ( PETSC_NULL, "-onefileperprocess", temporaryDirName, PETSC_MAX_PATH_LEN, &oneFilePerProcess );
      PetscOptionsGetString ( PETSC_NULL, "-primaryPod", temporaryDirName, PETSC_MAX_PATH_LEN, &primaryPod );

      setPrimaryPod ( primaryPod );
    
      if( temporaryDirName[0] == 0 ) {
         if( primaryPod ) {
            PetscPrintf ( PETSC_COMM_WORLD, "PrimaryPod directory is not defined. Please specify a shared dir name.\n" );           
         } else {
            tmpDir = getenv( "TMPDIR" );
         }
      } else {
         tmpDir = temporaryDirName;
      }
      
      if( tmpDir == NULL ) {
         noOfpp = PETSC_TRUE;
      } else {
         setTempDirName ( tmpDir );
         PetscPrintf ( PETSC_COMM_WORLD, "Set %s for output or/and input\n", tmpDir ); 
      }
   }
#else
   noOfpp = PETSC_TRUE;
#endif
   
   if( not primaryPod ) {
      setOneFilePerProcess ( !noOfpp );
   }
   return !noOfpp;
}

bool H5_Parallel_PropertyList :: copyMergedFile( const std::string & filePathName, const bool appendRank )
{
#ifdef _MSC_VER
	return false;
#else
	
   int rank;
   bool status = true;

   MPI_Comm_rank(PETSC_COMM_WORLD, &rank);

   if( rank == 0 ) {
      PetscBool noFileCopy = PETSC_FALSE;
      PetscOptionsHasName( PETSC_NULL, "-nocopy", &noFileCopy );

       if( !noFileCopy ) {
          ibs::FilePath curPath( getTempDirName() );

          if( appendRank ) { curPath << ( filePathName + "_0" ); }
          else curPath << filePathName;

          status = copyFile ( filePathName, curPath.path() );
          if( !status ) {
             PetscPrintf ( PETSC_COMM_WORLD, "  MeSsAgE ERROR Could not copy the file %s.\n", filePathName.c_str() );               
          }
       }
   }
   return status;
#endif
}

void H5_Parallel_PropertyList ::  setOneNodeCollectiveBufferingOption()
{
   PetscBool useAllNodesForCollectiveBuffering = PETSC_FALSE;
   PetscOptionsGetBool( PETSC_NULL, "-useallnodesforcb", &useAllNodesForCollectiveBuffering, NULL );
   
   if (! useAllNodesForCollectiveBuffering)
   {
      MPI_Info_create(& s_mpiInfo);
      MPI_Info_set (s_mpiInfo, "collective_buffering", "true");

      MPI_Info_set (s_mpiInfo, "romio_ds_read", "disable");
      MPI_Info_set (s_mpiInfo, "romio_ds_write", "disable");

      MPI_Info_set (s_mpiInfo, "romio_cb_read", "enable");
      MPI_Info_set (s_mpiInfo, "romio_cb_write", "enable");

      MPI_Info_set (s_mpiInfo, "cb_nodes", "1");
   }
}


