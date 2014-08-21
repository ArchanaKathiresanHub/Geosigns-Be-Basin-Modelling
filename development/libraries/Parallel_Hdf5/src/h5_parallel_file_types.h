#ifndef __H5_Parallel_PropertyList__
#define __H5_Parallel_PropertyList__

#include <mpi.h>

#include "h5_file_types.h"

class H5_Parallel_PropertyList : public H5_PropertyList
{
public:
   H5_PropertyList* clone() const
   { return new H5_Parallel_PropertyList(*this); }

   virtual hid_t createFilePropertyList() const ;

   virtual hid_t createDatasetPropertyList() const;
         
   static bool setOneFilePerProcessOption( const bool flag = false ) ;

   static void setOneNodeCollectiveBufferingOption();

   static bool isOneFilePerProcessEnabled() 
   { return s_oneFilePerProcess; }

   static std::string getTempDirName() 
   { return s_temporaryDirName; }

   static bool copyMergedFile( std::string & fileName );
private:
   static bool s_oneFilePerProcess;
   static std::string s_temporaryDirName;
   static MPI_Info s_mpiInfo;

   static void setOneFilePerProcess ( bool oneFilePerProcess )
   {  s_oneFilePerProcess = oneFilePerProcess; }

   static void setTempDirName ( const char * temporaryDirName )
   {  s_temporaryDirName = temporaryDirName; }
};

#endif
