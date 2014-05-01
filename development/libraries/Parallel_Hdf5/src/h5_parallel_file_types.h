//
// Overwrite Serial code with parallel
//

#ifndef __H5_Parallel_PropertyList__
#define __H5_Parallel_PropertyList__

#include "petsc.h"

#include "hdf5.h"
#include "h5_file_types.h"
#include "HDF5VirtualFileDriver.h"

extern bool IBS_SerializeIO;

class H5_Parallel_PropertyList : public H5_PropertyList
{
public:
   static bool s_oneFilePerProcess;
   static string s_temporaryDirName;

   static void setOneFilePerProcess ( bool oneFilePerProcess ) {  s_oneFilePerProcess = oneFilePerProcess; }
   static void setTempDirName ( char * temporaryDirName ) {  s_temporaryDirName = temporaryDirName; }
public:
   H5_Parallel_PropertyList () : H5_PropertyList () { }

   H5_PropertyList* clone (void) const { return new H5_Parallel_PropertyList (*this); }

   virtual hid_t createFilePropertyList (void) const 
   {
      static bool UseMPIInfoTested = false;

      static MPI_Info info = MPI_INFO_NULL;

      if (!UseMPIInfoTested) 
      {
	 if (IBS_SerializeIO)
	 {
	    if (getenv ("REPORT_FILE_LOCKING") != NULL)
	    {
	       int MyRank = -1;
	       MPI_Comm_rank(MPI_COMM_WORLD, &MyRank);
	       if (MyRank == 0) fprintf (stderr, "** Serializing HDF5 I/O as much as possible\n");
	    }
	    // Create info to be attached to HDF5 file
	    MPI_Info_create(& info);

	    MPI_Info_set (info, "collective_buffering", "true");
	    MPI_Info_set (info, "romio_ds_read", "disable");
	    MPI_Info_set (info, "romio_ds_write", "disable");

	    MPI_Info_set (info, "romio_cb_read", "enable");
	    MPI_Info_set (info, "romio_cb_write", "enable");

	    MPI_Info_set (info, "cb_nodes", "1");
	 }
	 UseMPIInfoTested = true;
      }

      hid_t plist = H5Pcreate (H5P_FILE_ACCESS);
      
      
      if( s_oneFilePerProcess ) {

         std::stringstream fileName;
         fileName << s_temporaryDirName << "/{NAME}_{MPI_RANK}";
          
         H5Pset_fapl_ofpp (plist, PETSC_COMM_WORLD, fileName.str().c_str(), 0); 
      } else {
         H5Pset_fapl_mpio (plist, PETSC_COMM_WORLD, info);  
      }

      return plist;
   }

   virtual hid_t createDatasetPropertyList (void) const
   {
     // set parallel read/write on file
      hid_t pList = H5P_DEFAULT;

     // COLLECTIVE data transfer no selected at the moment as the number of
     // MPI derived datatypes exceed MPI_TYPE_MAX on IRIX systems
     if( not s_oneFilePerProcess ) {
        pList = H5Pcreate (H5P_DATASET_XFER);
        
#if defined (sgi)
        H5Pset_dxpl_mpio (pList, H5FD_MPIO_INDEPENDENT);
#else
        H5Pset_dxpl_mpio (pList, H5FD_MPIO_COLLECTIVE);
#endif
     } 

     return pList;
   }
         
   static bool setOneFilePerProcessOption () {
      PetscBool oneFilePerProcess;
      char      temporaryDirName [ PETSC_MAX_PATH_LEN ];
      char *    tmpDir; 
           
      PetscOptionsGetString ( PETSC_NULL, "-onefileperprocess", temporaryDirName, PETSC_MAX_PATH_LEN, &oneFilePerProcess );
           
      if( oneFilePerProcess ) {
         if( temporaryDirName[0] == '\0' ) {
            tmpDir = getenv( "TMPDIR" );
         } else {
            tmpDir = temporaryDirName;
         }
      
         if( tmpDir == NULL ) {
            PetscPrintf ( PETSC_COMM_WORLD, " MeSsAgE WARNING $TMPDIR is not set, 'one file per process' option cannot be used.\n");    
            oneFilePerProcess = PETSC_FALSE;
         } else {
            H5_Parallel_PropertyList::setTempDirName ( tmpDir );
            PetscPrintf ( PETSC_COMM_WORLD, "Set %s for output\n", tmpDir );    
         }
      }
      H5_Parallel_PropertyList::setOneFilePerProcess ( oneFilePerProcess );

      return oneFilePerProcess;
   }
   
};

class H5_Independent_Parallel_PropertyList : public H5_Parallel_PropertyList
{
public:
   H5_PropertyList* clone (void) const { return new H5_Independent_Parallel_PropertyList (*this); }  
   ~H5_Independent_Parallel_PropertyList () {}

   hid_t createDatasetPropertyList (void) const
   {
      // set parallel read/write on file
      hid_t pList = H5P_DEFAULT;
  
      if( not s_oneFilePerProcess ) {
         pList = H5Pcreate (H5P_DATASET_XFER);

         // COLLECTIVE data transfer no selected at the moment as the number of
         // MPI derived datatypes exceed MPI_TYPE_MAX on IRIX systems
         
#if defined (sgi)
         H5Pset_dxpl_mpio (pList, H5FD_MPIO_INDEPENDENT);
#else
         H5Pset_dxpl_mpio (pList, H5FD_MPIO_COLLECTIVE);
#endif
      }
      return pList;
   }

};

#endif
