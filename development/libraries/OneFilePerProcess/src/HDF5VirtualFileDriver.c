#define _GNU_SOURCE

#include <stdlib.h>
#include <hdf5.h>

#include "RewriteFileName.h"

// Include the source file of the original POSIX-MPI virtual file driver from HDF5 library
// We need a reference to the driver Id
static hid_t OFPP_DriverID = 0;

// Our OFPP driver is an extension of the MPIPOSIX driver, therefore at argument checks we need to check for this ID.
#undef H5FD_MPIPOSIX
#define H5FD_MPIPOSIX      (OFPP_DriverID)

// Rename globally visable functions so that they are out of the way
static hid_t H5FD_mpiposix_init_disabled(void);
#define H5FD_mpiposix_init (H5FD_mpiposix_init_disabled)
#define H5FD_mpiposix_term (H5FD_mpiposix_term_disabled)
#define H5Pset_fapl_mpiposix (H5Pset_fapl_mpiposix_disabled)
#define H5Pget_fapl_mpiposix (H5Pget_fapl_mpiposix_disabled)

// include the source which this implementation derives from, but do enable the
// NDEBUG flag, because otherwise there will be link errors.
#ifndef NDEBUG
  #define RESET_NDEBUG 1
#else
  #define RESET_NDEBUG 0
#endif

#if RESET_NDEBUG
  #define NDEBUG
#endif

#include "H5FDmpiposix.c"

#if RESET_NDEBUG
  #undef NDEBUG
#endif


typedef struct OFPP_FileState
{
   // Base data
   H5FD_mpiposix_t m_mpiPosixFS;

   // Original MPI Communicator (as opposed how it is stored in m_mpiPosixFS)
   MPI_Comm m_comm;

   // file name rewrite pattern
   char *   m_fileNameRewritePattern;
} OFPP_FileState;

typedef struct OFPP_FAPL 
{
   // Base data
   H5FD_mpiposix_fapl_t m_mpiPosixFAPL;

   // Original MPI Communicator (as opposed how it is stored in m_mpiPosixFS)
   MPI_Comm m_comm;

   // file name rewrite pattern
   char *   m_fileNameRewritePattern;
} OFPP_FAPL;


static void * OFPP_fapl_copy( const void * other);
static void * OFPP_fapl_get( H5FD_t * file);
static herr_t OFPP_fapl_free( void * fapl);
H5FD_t * OFPP_openFile( const char * name, unsigned flags, hid_t fapl_id, haddr_t maxaddr);
static herr_t OFPP_closeFile(H5FD_t * file);


 
static H5FD_class_mpi_t interface =
     { {
       "mpiposix",                     /* name             */
       MAXADDR,                        /* maxaddr          */
       H5F_CLOSE_SEMI,                 /* fc_degree        */
       NULL,                           /* sb_size          */
       NULL,                           /* sb_encode        */
       NULL,                           /* sb_decode        */
       sizeof(OFPP_FAPL),              /* fapl_size        */
       OFPP_fapl_get,                  /* fapl_get         */
       OFPP_fapl_copy,                 /* fapl_copy        */
       OFPP_fapl_free,                 /* fapl_free        */
       0,                              /* dxpl_size        */
       NULL,                           /* dxpl_copy        */
       NULL,                           /* dxpl_free        */
       OFPP_openFile,                  /* open             */
       OFPP_closeFile,                 /* close            */
       H5FD_mpiposix_cmp,              /* cmp              */
       H5FD_mpiposix_query,            /* query            */
       NULL,                           /* get_type_map     */
       NULL,                           /* alloc            */
       NULL,                           /* free             */
       H5FD_mpiposix_get_eoa,          /* get_eoa          */
       H5FD_mpiposix_set_eoa,          /* set_eoa          */
       H5FD_mpiposix_get_eof,          /* get_eof          */
       H5FD_mpiposix_get_handle,       /* get_handle       */
       H5FD_mpiposix_read,             /* read             */
       H5FD_mpiposix_write,            /* write            */
       NULL,                           /* flush            */
       H5FD_mpiposix_truncate,         /* truncate         */
       NULL,                           /* lock             */
       NULL,                           /* unlock           */
       H5FD_FLMAP_DICHOTOMY            /* fl_map           */
       },  /* End of superclass information */
       H5FD_mpiposix_mpi_rank,         /* get_rank         */
       H5FD_mpiposix_mpi_size,         /* get_size         */
       H5FD_mpiposix_communicator      /* get_comm         */
     };
  
hid_t OFPP_init(void)
{
   if (H5I_VFL != H5Iget_type(OFPP_DriverID))
     OFPP_DriverID = H5FD_register((const H5FD_class_t *) & interface, sizeof(H5FD_class_mpi_t) , FALSE);
   
   return OFPP_DriverID;
}

void OFPP_term(void)
{
   herr_t status = H5FDunregister( OFPP_DriverID );
   assert( status >= 0 );
}

void H5Pset_fapl_ofpp( hid_t fapl, MPI_Comm comm, const char * fileNameRewritePattern, hbool_t useGpfs) 
{
  assert( MPI_COMM_NULL != comm );
  OFPP_init();

  H5P_genplist_t * plist = H5P_object_verify( fapl, H5P_FILE_ACCESS);
  assert( plist );

  // Set file access property list
  OFPP_FAPL fa;
  fa.m_mpiPosixFAPL.comm = MPI_COMM_SELF;
  fa.m_mpiPosixFAPL.use_gpfs = useGpfs;
  fa.m_comm = comm;
  if (fileNameRewritePattern == NULL)
     fa.m_fileNameRewritePattern = NULL;
  else
     fa.m_fileNameRewritePattern = strdup( fileNameRewritePattern );

  // set driver
  herr_t status = H5P_set_driver(plist, OFPP_DriverID, &fa);
  assert( status >= 0);
}

void H5Pget_fapl_ofpp( hid_t fapl, MPI_Comm * comm, const char * * fileNameRewritePattern, hbool_t * useGpfs )
{
  H5P_genplist_t * plist = H5P_object_verify( fapl, H5P_FILE_ACCESS);
  assert( plist );
  assert( OFPP_DriverID == H5P_get_driver(plist));
  OFPP_init();

  // get the file access property list
  OFPP_FAPL * fa = H5P_get_driver_info(plist);
  assert( fa );

  // copy the communicator
  if (comm != NULL)
    MPI_Comm_dup(fa->m_comm, comm);

  // copy the fileNameRewritePattern
  if (fileNameRewritePattern != NULL)
     *fileNameRewritePattern = strdup( fa->m_fileNameRewritePattern );

  // copy the use_gpfs
  if (useGpfs != NULL)
     *useGpfs = fa->m_mpiPosixFAPL.use_gpfs;

}


static void * OFPP_fapl_copy( const void * other)
{
   int mpiError = MPI_SUCCESS;

   const OFPP_FAPL * oldFapl  = (const OFPP_FAPL *) other;

   // construct a new object
   OFPP_FAPL * newFapl = H5MM_malloc(sizeof( OFPP_FAPL));

   // copy the MPIPOSIX communicator (which is always MPI_COMM_SELF)
   assert( oldFapl->m_mpiPosixFAPL.comm == MPI_COMM_SELF);
   mpiError = MPI_Comm_dup( oldFapl->m_mpiPosixFAPL.comm, & newFapl->m_mpiPosixFAPL.comm );
   assert( mpiError == MPI_SUCCESS );

   // GPFS?
   newFapl->m_mpiPosixFAPL.use_gpfs = oldFapl->m_mpiPosixFAPL.use_gpfs;

   // Copy the actual communicator
   mpiError = MPI_Comm_dup( oldFapl->m_comm, & newFapl->m_comm );
   assert( mpiError == MPI_SUCCESS );

   // assign it a copy of the file name rewrite pattern
   if (oldFapl->m_fileNameRewritePattern)
   {
      newFapl->m_fileNameRewritePattern = strdup( oldFapl->m_fileNameRewritePattern );
      assert( newFapl->m_fileNameRewritePattern );
   }
   else
   {
      newFapl->m_fileNameRewritePattern = 0;
   }

   return newFapl;
}

static void * OFPP_fapl_get( H5FD_t * file)
{
   int mpiError = MPI_SUCCESS;

   const OFPP_FileState * fileState  = (const OFPP_FileState *) file ;
   assert( fileState );
   assert( OFPP_DriverID == fileState->m_mpiPosixFS.pub.driver_id );

   // construct a new object
   OFPP_FAPL * newFapl = H5MM_malloc(sizeof(OFPP_FAPL));

   // assign it a copy of MPIPOSIX MPI communicator
   assert( fileState->m_mpiPosixFS.comm == MPI_COMM_SELF);
   mpiError = MPI_Comm_dup( fileState->m_mpiPosixFS.comm, & newFapl->m_mpiPosixFAPL.comm );
   assert( mpiError == MPI_SUCCESS );

   // GPFS?
   newFapl->m_mpiPosixFAPL.use_gpfs = fileState->m_mpiPosixFS.use_gpfs;

   // Copy the actual communicator
   mpiError = MPI_Comm_dup( fileState->m_comm, & newFapl->m_comm );
   assert( mpiError == MPI_SUCCESS );

   // assign it a copy of the file name rewrite pattern
   newFapl->m_fileNameRewritePattern = strdup( fileState->m_fileNameRewritePattern );
   assert( !fileState->m_fileNameRewritePattern || newFapl->m_fileNameRewritePattern );

   return newFapl;
}

static herr_t OFPP_fapl_free( void * fapl)
{
   OFPP_FAPL * thisFapl = (OFPP_FAPL *) fapl;
   assert( thisFapl );

   assert( thisFapl->m_mpiPosixFAPL.comm == MPI_COMM_SELF);
   MPI_Comm_free(& thisFapl->m_mpiPosixFAPL.comm);

   MPI_Comm_free(& thisFapl->m_comm );
  
   if (thisFapl->m_fileNameRewritePattern)
      free( thisFapl->m_fileNameRewritePattern );

   H5MM_xfree( thisFapl );
   
   return 0;
}

H5FD_t * OFPP_openFile( const char * name, unsigned flags, hid_t fapl_id, haddr_t maxaddr)
{
   assert( name );
   assert( maxaddr > 0 );
   assert( maxaddr != HADDR_UNDEF );
   assert( ! ADDR_OVERFLOW(maxaddr) );

   // Obtain a pointer to specific file access properties
   H5P_genplist_t * plist = H5P_object_verify( fapl_id, H5P_FILE_ACCESS );
   assert( !plist );

   // Get the file access properties
   OFPP_FAPL defaultFa;
   const OFPP_FAPL * fa = 0;
   if (H5P_FILE_ACCESS_DEFAULT == fapl_id || OFPP_DriverID != H5P_get_driver(plist))
   {  // Choose the default settings
      defaultFa.m_mpiPosixFAPL.comm = MPI_COMM_SELF;
      defaultFa.m_mpiPosixFAPL.use_gpfs = FALSE;
      defaultFa.m_comm = MPI_COMM_SELF;
      defaultFa.m_fileNameRewritePattern = 0;
      fa = & defaultFa;
   }
   else
   {  // get a reference to the given settings
      fa = (const OFPP_FAPL *) H5P_get_driver_info(plist) ;
      assert( fa );
   }

   // Construct the file name
   const char * fileName = 0;
   char * generatedFileName = 0;
   int mpiRank, mpiSize;
   MPI_Comm_rank( fa->m_comm, &mpiRank);
   MPI_Comm_size( fa->m_comm, &mpiSize);
   if ( fa->m_fileNameRewritePattern)
   {
      size_t generatedFileNameSize = rewriteFileName( fa->m_fileNameRewritePattern, name, mpiRank, mpiSize, NULL, 0);
      generatedFileName = malloc( generatedFileNameSize);
      rewriteFileName( fa->m_fileNameRewritePattern, name, mpiRank, mpiSize, generatedFileName, generatedFileNameSize);
      fileName = generatedFileName ; 
   }
   else
   {
      fileName = name;
   }

   // Open the file
   H5FD_mpiposix_t * file =(H5FD_mpiposix_t *) H5FD_mpiposix_open( fileName, flags, fapl_id, maxaddr ) ;

   // if that fails and we're opening an existing file, try again wih the normal file name
   if ( !file && (H5F_ACC_TRUNC & flags) == 0  && (H5F_ACC_CREAT & flags) == 0 && (H5F_ACC_EXCL & flags) == 0)
   {
      file = (H5FD_mpiposix_t *) H5FD_mpiposix_open( name, flags, fapl_id, maxaddr ) ;
   }

   // if the file could not be opened, return the null pointer
   if (file == NULL)
   {
      return NULL;
   }

   assert( file != NULL );

   // Transplant the file state in our extended data type
   OFPP_FileState * extendedFile = H5MM_malloc(sizeof(OFPP_FileState));
   assert( extendedFile);
   assert( sizeof(*extendedFile) >= sizeof(*file));
   memcpy( extendedFile, file, sizeof(*file));
   H5MM_xfree( file );
   
   // Add the extended information
   int mpiError = MPI_Comm_dup( fa->m_comm, & extendedFile->m_comm);
   assert( mpiError == MPI_SUCCEED);

   if (fa->m_fileNameRewritePattern)
      extendedFile->m_fileNameRewritePattern = strdup( fa->m_fileNameRewritePattern );
   else
      extendedFile->m_fileNameRewritePattern = 0;

   // Reclaim memory from the generated file name (if it was generated)
   if (generatedFileName != NULL)
      free( generatedFileName );

   return (H5FD_t *) extendedFile;
}


static herr_t OFPP_closeFile(H5FD_t * file)
{
   OFPP_FileState * extendedFile = ( OFPP_FileState * ) file;
   MPI_Comm_free( & extendedFile->m_comm );
   if (extendedFile->m_fileNameRewritePattern)
      free( extendedFile->m_fileNameRewritePattern );

   herr_t status = H5FD_mpiposix_close( file );
   assert( status >= 0 );

   return status;
}

