#include <petsc.h>
#include <hdf5.h>

#include "h5_parallel_file_types.h"
#include "HDF5VirtualFileDriver.h"


bool        H5_Parallel_PropertyList :: s_oneFilePerProcess = false;

std::string H5_Parallel_PropertyList :: s_temporaryDirName;

MPI_Info    H5_Parallel_PropertyList :: s_mpiInfo = MPI_INFO_NULL;


hid_t H5_Parallel_PropertyList :: createFilePropertyList() const 
{
   hid_t plist = H5Pcreate (H5P_FILE_ACCESS);
   
   if( s_oneFilePerProcess ) 
   {
      std::stringstream fileName;
      fileName << s_temporaryDirName << "/{NAME}_{MPI_RANK}";
       
      H5Pset_fapl_ofpp (plist, PETSC_COMM_WORLD, fileName.str().c_str(), 0); 
   } 
   else 
   {
      H5Pset_fapl_mpio (plist, PETSC_COMM_WORLD, s_mpiInfo);  
   }

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
         
bool H5_Parallel_PropertyList :: setOneFilePerProcessOption()
{
   PetscBool oneFilePerProcess = PETSC_FALSE;
   char      temporaryDirName [ PETSC_MAX_PATH_LEN ];
   char *    tmpDir = 0; 
        
   PetscOptionsGetString ( PETSC_NULL, "-onefileperprocess", temporaryDirName, PETSC_MAX_PATH_LEN, &oneFilePerProcess );
        
   if( oneFilePerProcess ) {
      if( temporaryDirName[0] == '\0' ) {
         tmpDir = getenv( "CAULDRON_TMPDIR" );
      } else {
         tmpDir = temporaryDirName;
      }
   
      if( tmpDir == NULL ) {
         PetscPrintf ( PETSC_COMM_WORLD, " MeSsAgE WARNING $CAULDRON_TMPDIR is not set, 'one file per process' option cannot be used.\n");    
         oneFilePerProcess = PETSC_FALSE;
      } else {
         setTempDirName ( tmpDir );
         PetscPrintf ( PETSC_COMM_WORLD, "Set %s for output or/and input\n", tmpDir ); 
      }
   }
   setOneFilePerProcess ( oneFilePerProcess );

   return oneFilePerProcess;
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


