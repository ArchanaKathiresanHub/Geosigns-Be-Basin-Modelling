//
// Overwrite Serial code with parallel
//

#ifndef __H5_Parallel_PropertyList__
#define __H5_Parallel_PropertyList__

#include "petsc.h"

#include "hdf5.h"
#include "h5_file_types.h"

extern bool IBS_SerializeIO;

class H5_Parallel_PropertyList : public H5_PropertyList
{
public:
   H5_Parallel_PropertyList () : H5_PropertyList () {}
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
      H5Pset_fapl_mpio (plist, PETSC_COMM_WORLD, info);

      return plist;
   }

   virtual hid_t createDatasetPropertyList (void) const
   {
     // set parallel read/write on file
     hid_t pList = H5Pcreate (H5P_DATASET_XFER);

     // COLLECTIVE data transfer no selected at the moment as the number of
     // MPI derived datatypes exceed MPI_TYPE_MAX on IRIX systems

     #if defined (sgi)
        H5Pset_dxpl_mpio (pList, H5FD_MPIO_INDEPENDENT);
     #else
	H5Pset_dxpl_mpio (pList, H5FD_MPIO_COLLECTIVE);
     #endif

     return pList;
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
     hid_t pList = H5Pcreate (H5P_DATASET_XFER);

     // COLLECTIVE data transfer no selected at the moment as the number of
     // MPI derived datatypes exceed MPI_TYPE_MAX on IRIX systems

     #if defined (sgi)
        H5Pset_dxpl_mpio (pList, H5FD_MPIO_INDEPENDENT);
     #else
	H5Pset_dxpl_mpio (pList, H5FD_MPIO_COLLECTIVE);
     #endif

     return pList;
   }

};

#endif
