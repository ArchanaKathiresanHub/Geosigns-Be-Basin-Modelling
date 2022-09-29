#ifndef __H5_Parallel_PropertyList__
#define __H5_Parallel_PropertyList__

#include <mpi.h>

#include "h5_file_types.h"

class H5_Parallel_PropertyList : public H5_PropertyList
{
public:
   H5_PropertyList* clone() const
   { return new H5_Parallel_PropertyList(*this); }

   virtual hid_t createFilePropertyList( ) const ;

   virtual hid_t createCreateDatasetPropertyList( ) const;
   virtual hid_t createAccessDatasetPropertyList( ) const;
   virtual hid_t createRawTransferDatasetPropertyList( ) const;
         
   static void setOtherFileProcessOptions( const bool createDir =  true ) ;

   static void setOneNodeCollectiveBufferingOption();

   static bool isOneFileLustreEnabled() 
   { return s_oneFileLustre; }

   static bool isPrimaryPodEnabled() 
   { return s_primaryPod; }

   static std::string getTempDirName() 
   { return s_temporaryDirName; }

   static void setOneFileLustre ( bool oneFileLustre )
   {  s_oneFileLustre = oneFileLustre; }

   static void setPrimaryPod ( bool isPrimaryPod )
   {  s_primaryPod = isPrimaryPod; }
   
   static bool copyMergedFile( const std::string & fileName, const bool rank = true );

   static bool mergeOutputFiles ( const string & activityName, const std::string & localPath );

   static bool removeOutputFile ( const string & filePathName );
private:

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
