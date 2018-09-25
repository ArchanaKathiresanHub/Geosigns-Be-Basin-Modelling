#ifndef __H5_Parallel_PropertyList__
#define __H5_Parallel_PropertyList__

#include <mpi.h>

#include "h5_file_types.h"

class H5_Parallel_PropertyList : public H5_PropertyList
{
public:
   H5_PropertyList* clone() const
   { return new H5_Parallel_PropertyList(*this); }

   virtual hid_t createFilePropertyList( const bool readOnly ) const ;

   virtual hid_t createDatasetPropertyList( const bool readOnly ) const;
         
   static bool setOneFilePerProcessOption( const bool createDir =  true ) ;

   static void setOneNodeCollectiveBufferingOption();

   static bool isOneFilePerProcessEnabled() 
   { return s_oneFilePerProcess; }

   static bool isOneFileLustreEnabled() 
   { return s_oneFileLustre; }

   static bool isPrimaryPodEnabled() 
   { return s_primaryPod; }

   static std::string getTempDirName() 
   { return s_temporaryDirName; }

   static void setOneFilePerProcess ( bool oneFilePerProcess )
   {  s_oneFilePerProcess = oneFilePerProcess; }

   static void setOneFileLustre ( bool oneFileLustre )
   {  s_oneFileLustre = oneFileLustre; }
 
   static void setPrimaryPod ( bool oneFilePerProcess )
   {  s_primaryPod = oneFilePerProcess; }
   
   static bool copyMergedFile( const std::string & fileName, const bool rank = true );

   static bool mergeOutputFiles ( const string & activityName, const std::string & localPath );

   static bool removeOutputFile ( const string & filePathName );
private:

   // "OneFilePerProcess" mode is a default mode for all the simulators.
   // 
   // If TMPDIR environment variable is defined it will be used to create temporary output files - one file per each MPI process.
   // All temporary files will be merged at the end of the simulation into a one file and saved in the project output directory. 
   // Alternatively a temporary directory can be specified with a command-line option "-onefileperprocess <pathToTempDir>".

   // To disable oneFilePerProcess mode use a command-line option "-noofpp"
   // Output files will be written directly to the project file output directory.
   static bool s_oneFilePerProcess;

   // "PrimaryPod" is an option to write the results to a shared Lustre directory on a cluster
   // To enable primaryPod mode use a command-line option "-primaryPod <pathToSharedLustreDirOnCluster>"
   //
   // If enabled, a temporary dir will be created in pathToSharedLustreDirOnCluster and the output files will be written there.
   // At the end of the simulation the results will be copied to the project output directory and the temporary directory will be deleted.
   //
   // Options which can be used along with -primaryPod:
   //  "-nocopy" - the results will not be copied from the temporary dir
   //  "-noremove" - the temporary dir will not be removed

   // Note: Volume output (3D HDF files) will be written/stored with chunked layout. HDF cache evictions will be disabled.
   // (see DistributedMapWriter::setChunking())

   static bool s_primaryPod;

   // "Lustre" mode is an equivalent to "-noofpp" command-line option, but with chunked storage enabled.
   // Command-line option: "-lustre".
   // The output files will be written to the project output directory.
   // Volume (3D) HDF files will be written/stored with chunked datasets. HDF cache evictions will be disabled.

   static bool s_oneFileLustre;

   static std::string s_temporaryDirName;
   static MPI_Info s_mpiInfo;

   static void setTempDirName ( const char * temporaryDirName )
   {  s_temporaryDirName = temporaryDirName; }
};

#endif
