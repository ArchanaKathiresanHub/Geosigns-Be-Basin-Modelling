#ifndef ONEFILEPERPROCESS_FILEHANDLER_APPEND_H
#define ONEFILEPERPROCESS_FILEHANDLER_APPEND_H

#include "fileHandler.h"

class FileHandlerAppend : public FileHandler {
public:
   FileHandlerAppend( MPI_Comm comm, const std::string & fileName, const std::string & tempDirName );

   virtual ~FileHandlerAppend() {}

   /// \brief Create or open the global file for update
   void openGlobalFile( );
   /// \brief Create or open a group in global file
   void createGroup( const char* name );

   /// \brief Create or open a dataset in global file
   /// \param [in] name The name of dataset
   /// \param [in] type The datatype of the datase
   void createDataset( const char* name , hid_t dtype );

   void writeAttributes();

private:
   /// \brief Indicates whether or not the existing file is opened for update
   bool m_update = true;
};


#endif
