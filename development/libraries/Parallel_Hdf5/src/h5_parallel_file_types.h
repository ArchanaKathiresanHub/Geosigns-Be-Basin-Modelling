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
         
   static bool setOneFilePerProcessOption() ;

   static void setOneNodeCollectiveBufferingOption();

   static bool isOneFilePerProcessEnabled() 
   { return s_oneFilePerProcess; }
   static bool isPrimaryPodEnabled() 
   { return s_primaryPod; }

   static std::string getTempDirName() 
   { return s_temporaryDirName; }

   static void setOneFilePerProcess ( bool oneFilePerProcess )
   {  s_oneFilePerProcess = oneFilePerProcess; }
 
   static void setPrimaryPod ( bool oneFilePerProcess )
   {  s_primaryPod = oneFilePerProcess; }
   
   static bool copyMergedFile( const std::string & fileName, const bool rank = true );

   static bool mergeOutputFiles ( const string & activityName, const std::string & localPath );

   static bool removeOutputFile ( const string & filePathName );
private:
   static bool s_oneFilePerProcess;
   static bool s_primaryPod;
   static std::string s_temporaryDirName;
   static MPI_Info s_mpiInfo;

   static void setTempDirName ( const char * temporaryDirName )
   {  s_temporaryDirName = temporaryDirName; }
};

#endif
