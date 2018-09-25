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
bool        H5_Parallel_PropertyList :: s_oneFileLustre = false;

std::string H5_Parallel_PropertyList :: s_temporaryDirName;

MPI_Info    H5_Parallel_PropertyList :: s_mpiInfo = MPI_INFO_NULL;


hid_t H5_Parallel_PropertyList :: createFilePropertyList( const bool readOnly ) const
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
      if( not readOnly ) {
         H5Pset_fapl_mpio (plist, PETSC_COMM_WORLD, s_mpiInfo);

         if( s_primaryPod or s_oneFileLustre ) {
            // Disable cache
            // set B-tree to roughly same size as 'stripe size' (default stripe size 1Mb)
            // https://www.nersc.gov/users/training/online-tutorials/introduction-to-scientific-i-o/?start=5
            // hsize_t btree_ik = 10880; //(chunkSize - 4096) / 96;
            // H5Pset_istore_k(plist, btree_ik);

            // disable cache evictions
            H5AC_cache_config_t mdc_config;
            memset(&mdc_config, 0, sizeof(mdc_config));
            mdc_config.version = H5AC__CURR_CACHE_CONFIG_VERSION;
            H5Pget_mdc_config(plist, &mdc_config);
            mdc_config.evictions_enabled = 0;
            mdc_config.flash_incr_mode = H5C_flash_incr__off;
            mdc_config.incr_mode = H5C_incr__off;
            mdc_config.decr_mode = H5C_decr__off;
            H5Pset_mdc_config(plist, &mdc_config);
         }
      }
   }


#else
   H5Pset_fapl_mpio (plist, PETSC_COMM_WORLD, s_mpiInfo);
#endif
   return plist;
}

hid_t H5_Parallel_PropertyList :: createDatasetPropertyList( const bool readOnly ) const
{
   // set parallel read/write on file
   hid_t pList = H5P_DEFAULT;

   if( not s_oneFilePerProcess and not readOnly )
   {
      pList = H5Pcreate (H5P_DATASET_XFER);
      H5Pset_dxpl_mpio (pList, H5FD_MPIO_COLLECTIVE);
   }

   return pList;
}

bool H5_Parallel_PropertyList :: setOneFilePerProcessOption( const bool createDir )
{
   PetscBool noOfpp     = PETSC_FALSE;
   PetscBool primaryPod = PETSC_FALSE;
   PetscBool ofLustre   = PETSC_FALSE;

#ifndef _MSC_VER
   PetscOptionsHasName ( PETSC_NULL, "-noofpp", &noOfpp );
   PetscOptionsHasName ( PETSC_NULL, "-lustre", &ofLustre );

   if( !noOfpp and !ofLustre ) {

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
            setPrimaryPod ( PETSC_FALSE );
       } else {
            tmpDir = getenv( "TMPDIR" );
         }
      } else {
         if( primaryPod ) {

            // Create a temporary dir unique name and broadcast it
            int rank;

            MPI_Comm_rank(PETSC_COMM_WORLD, &rank);
            int tempLen = 0;
            if( createDir ) {
               if( rank == 0 ) {
                  char templateName[] = "ProjectXXXXXX";


                  // if mkstemp is successful
                  strcat( temporaryDirName, "/" );
                  strcat( temporaryDirName, templateName );
                  char * tempName = mkdtemp( temporaryDirName );
                  if (tempName != 0) {

                     tmpDir = temporaryDirName;
                     tempLen = strlen( temporaryDirName );
                  }
               }

               MPI_Bcast( &tempLen, 1, MPI_INT, 0, PETSC_COMM_WORLD );
               if( tempLen > 0 ) {
                  MPI_Bcast( temporaryDirName, tempLen, MPI_CHAR, 0, PETSC_COMM_WORLD );
                  tmpDir = temporaryDirName;
               } else {
                  setPrimaryPod ( PETSC_FALSE );
                  tmpDir = 0;
                  PetscPrintf ( PETSC_COMM_WORLD, "Temporary dir cannot be created.\n" );
               }
            } else {
               tmpDir = temporaryDirName;
            }
         } else {
            tmpDir = temporaryDirName;
         }
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
      if( not ofLustre ) {
         setOneFilePerProcess ( !noOfpp );
      } else {
         setOneFileLustre ( true );
      }
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
             PetscPrintf ( PETSC_COMM_WORLD, "  Basin_Error: Could not copy the file %s.\n", filePathName.c_str() );
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
bool H5_Parallel_PropertyList ::mergeOutputFiles ( const string & activityName, const std::string & localPath ) {

   if( not H5_Parallel_PropertyList::isOneFilePerProcessEnabled() and not H5_Parallel_PropertyList::isPrimaryPodEnabled ()) {
      return true;
   }

#ifdef _MSC_VER
   return true;
#else
   int rank;
   MPI_Comm_rank(PETSC_COMM_WORLD, &rank);

   PetscBool noFileCopy = PETSC_FALSE;
   PetscOptionsHasName( PETSC_NULL, "-nocopy", &noFileCopy );
   PetscBool noFileRemove = PETSC_FALSE;
   PetscOptionsHasName( PETSC_NULL, "-noremove", &noFileRemove );

   string fileName = activityName + "_Results.HDF" ;
   ibs::FilePath filePathName( localPath );
   filePathName << fileName;

   bool status = false;

   if( not  H5_Parallel_PropertyList::isPrimaryPodEnabled () ) {
      PetscPrintf ( PETSC_COMM_WORLD, "Merging of output files.\n" );
      status = mergeFiles ( allocateFileHandler( PETSC_COMM_WORLD, filePathName.path(), H5_Parallel_PropertyList::getTempDirName(),( noFileCopy ? CREATE : REUSE )));
      
      if( !noFileCopy and status ) {
         status = H5_Parallel_PropertyList::copyMergedFile( filePathName.path() );
      }
   } else {
      status = true;
      if( not noFileCopy ) {
         PetscPrintf ( PETSC_COMM_WORLD, "Copying of output files from Lustre to TCS storage.\n" );
         
         status = H5_Parallel_PropertyList::copyMergedFile( filePathName.path(), false );
      } 
      if( status and rank == 0  and not noFileRemove ) {
         // remove the file
         removeOutputFile ( filePathName.cpath () );
          
         // remove the directory
         removeOutputFile ( localPath );
         
         // remove the temporary directory
         removeOutputFile ( std::string("") );
      }
   }
   MPI_Barrier( PETSC_COMM_WORLD );

   if( !status ) {
      PetscPrintf ( PETSC_COMM_WORLD, "  Basin_Error: Could not merge/copy the file %s.\n", filePathName.cpath() );
   }

   return status;
#endif
}

bool H5_Parallel_PropertyList ::removeOutputFile ( const string & filePathName ) {

   ibs::FilePath fileName(H5_Parallel_PropertyList::getTempDirName() );
   fileName << filePathName;

   int status = std::remove( fileName.cpath() );
   if (status == -1) {
      PetscPrintf ( PETSC_COMM_WORLD, " %s  Basin_Warning:  Unable to remove the file, because '%s' \n", filePathName.c_str (), std::strerror(errno) );
      return false;
   }
   return true;
}




