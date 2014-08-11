
#include "mpi.h"
#include "hdf5.h"

#include "h5merge.h"
#include "fileHandler.h"

#include "strings.h"
#include "stdlib.h"
#include <sstream>
#include <fstream>
#include <iostream>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "H5FDmpio.h"
#include "HDF5VirtualFileDriver.h"

//#define SAFE_RUN 1

herr_t readDataset ( hid_t groupId, const char* name, void * voidReader)  {


   FileHandler* reader = static_cast<FileHandler *>( voidReader );

   if( groupId == NULL ) {
      
      if( reader->m_rank == 0 ) {       
         std::cout << " ERROR Cannot iterate the group or dataset " << name << std::endl;
      }
      return -1;
   }

   hid_t memspace, filespace;
   hid_t local_dset_id;
   hid_t dataSpaceId;
   int   status = 0;
 
   H5G_stat_t statbuf;

   double startTime = MPI_Wtime();
   status = H5Gget_objinfo ( groupId, name, 0, &statbuf );
   FileHandler::s_readDTime += MPI_Wtime() - startTime;

   if( reader->checkError( status ) < 0 ) {
      if( reader->m_rank == 0 ) {       
         std::cout << " ERROR Cannot get the object " <<  name << " info" << std::endl;
      }
      return -1;
   }

   switch ( statbuf.type ) {
   case H5G_GROUP:
      {
         std::stringstream groupName;
         groupName << "/" << name;

         if( reader->m_rank == 0 ) {       
            // Greate a group in the global file
            startTime = MPI_Wtime();
            if( !reader->m_reuse ) {
               reader->m_groupId = H5Gcreate( reader->m_globalFileId, name,  H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT ); 
            } 
            if( reader->m_groupId < 0 ) {
               std::cout << " ERROR Cannot open or create the group " << name << std::endl;
            }
            FileHandler::s_writeDTime += MPI_Wtime() - startTime;
            FileHandler::s_creatingGTime += MPI_Wtime() - startTime;
         }
         // Iterate over the members of the group
         H5Giterate ( reader-> m_localFileId, groupName.str().c_str(), 0, readDataset, voidReader );

         if( reader->m_rank == 0 ) {       
            H5Gclose( reader->m_groupId );
         }
         return 0;
      }
      break;
   case H5G_DATASET:
      break;
   case H5G_TYPE:
      break;
   default: ;

   } 

   startTime = MPI_Wtime();
   local_dset_id = H5Dopen ( groupId, name, H5P_DEFAULT );

#ifdef SAFE_RUN
   if( reader->checkError ( local_dset_id ) < 0 ) {
      return -1;
   }
#endif

   dataSpaceId = H5Dget_space ( local_dset_id );

   reader->m_spatialDimension = H5Sget_simple_extent_dims ( dataSpaceId, reader->m_dimensions, 0 );

   H5Sclose( dataSpaceId );
 
   // Get the dataset datatype
   hid_t   dtype    = H5Dget_type( local_dset_id );
   ssize_t dataSize = H5Tget_size(dtype);
 
#ifdef SAFE_RUN
   if( reader->checkError ( dataSize ) < 0 || dataSize < 0 ) {
      return -1;
   }
#endif

    // Allocate data buffers
   reader->reallocateBuffers ( dataSize );

   // Read the local data
   memspace = H5Screate_simple( reader->m_spatialDimension, reader->m_count, NULL ); 
   filespace = H5Dget_space( local_dset_id );

   H5Sselect_hyperslab( filespace, H5S_SELECT_SET, reader->m_offset, NULL, reader->m_count, NULL );

   double startTime1 = MPI_Wtime();
   H5Dread ( local_dset_id, dtype, memspace, filespace, H5P_DEFAULT, reader->m_data.data() );
   FileHandler::s_readingDTime += MPI_Wtime() - startTime1;
   FileHandler::s_readDTime += MPI_Wtime() - startTime;

   if( !reader->m_reuse ) {
      H5Sclose( filespace );
      H5Sclose( memspace );
   }
   
   // Summarize all local data in a global array
   if( reader->m_spatialDimension == 1 ) {
      if( reader->m_rank == 0 ) {
         reader->m_sumData = reader->m_data;
      }
   } else {
      startTime = MPI_Wtime();
      MPI_Reduce( reader->m_data.data(), reader->m_sumData.data(), reader->m_valCount, MPI_CHAR, MPI_SUM, 0, reader->m_comm );
      FileHandler::s_collectingTime +=  MPI_Wtime() - startTime;
   }

   if( reader->m_rank == 0 ) {
      startTime1 = MPI_Wtime();

      hid_t global_dset_id;
      if( !reader->m_reuse ) {
         // Write the global data into the global file
         memspace = H5Screate_simple( reader->m_spatialDimension, reader->m_count, NULL ); 
         
         startTime = MPI_Wtime();
         if( reader->m_groupId != H5P_DEFAULT ) {
            // Dataset is under the sub-group
            global_dset_id = H5Dcreate( reader->m_groupId, name, dtype, memspace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
         } else {
            // Dataset is under the main group
            global_dset_id = H5Dcreate( reader->m_globalFileId, name, dtype, memspace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
         }
         FileHandler::s_creatingDTime +=  MPI_Wtime() - startTime;
         
         if( global_dset_id < 0 ) {
            std::cout << " ERROR Cannot create the group" << name << std::endl;
         }
      } else {
         global_dset_id = local_dset_id;
      }

      if( !reader->m_reuse ) {
         filespace = H5Dget_space( global_dset_id );
         H5Sselect_hyperslab( filespace, H5S_SELECT_SET, reader->m_offset, NULL, reader->m_count, NULL );
      }
      startTime = MPI_Wtime();
      status = H5Dwrite( global_dset_id, dtype, memspace, filespace, H5P_DEFAULT, reader->m_sumData.data() );
      FileHandler::s_writingDTime +=  MPI_Wtime() - startTime;
      FileHandler::s_writeDTime +=  MPI_Wtime() - startTime1;

      if( status >= 0 ) {
         
         startTime = MPI_Wtime();
         if( !reader->m_reuse ) {
            status = reader->readAttributes( local_dset_id, global_dset_id ); 
            if( status < 0 ) {
               std::cout << " ERROR Cannot write attributes" << std::endl;
            }
            FileHandler::s_attributeTime +=  MPI_Wtime() - startTime;
         }
      } else {
         if( reader->m_rank == 0 ) {       
            std::cout << " ERROR Cannot write the dataset" << name << std::endl;
         }
      }

      startTime = MPI_Wtime();

      H5Sclose( filespace );
      H5Sclose( memspace );

      if( !reader->m_reuse ) {
         H5Dclose( global_dset_id );
      }
      FileHandler::s_writeDTime +=  MPI_Wtime() - startTime;
   }
 
   H5Dclose( local_dset_id );

#ifdef SAFE_RUN
   MPI_Bcast( &status, 1, MPI_INT, 0, reader->m_comm );
   
   if( status < 0 ) {
      return -1;
   }
#endif
   
   
   return 0;
}

bool mergeFiles( MPI_Comm comm, const std::string & fileName, const std::string & tempDirName, const bool reuse ) {
 
   double totalStart = MPI_Wtime();
   FileHandler reader ( comm );
 
   hid_t status = 0, close_status = 0, iteration_status = 0;

   reader.m_reuse = reuse;

   hid_t fileAccessPList = H5Pcreate(H5P_FILE_ACCESS);

   std::stringstream tmpName;
   tmpName << tempDirName << "/{NAME}_{MPI_RANK}";

   double startTime = MPI_Wtime();
   H5Pset_fapl_ofpp ( fileAccessPList, comm, tmpName.str().c_str(), 0 );  
   
   if( reader.m_reuse ) {
      reader.m_localFileId = H5Fopen( fileName.c_str(), H5F_ACC_RDWR, fileAccessPList );
   } else {
      reader.m_localFileId = H5Fopen( fileName.c_str(), H5F_ACC_RDONLY, fileAccessPList );
   }

   H5Pclose( fileAccessPList );
   FileHandler::s_readDTime += MPI_Wtime() - startTime;
   
   if( reader.checkError( reader.m_localFileId ) < 0 ) {
      if( reader.m_rank == 0 ) {       
         std::cout << " ERROR Cannot open the local file " << fileName << std::endl;
      }
      return false;
   }
  
   if( reader.m_rank == 0 ) {
      startTime = MPI_Wtime();   

      if( reader.m_reuse ) {
         reader.m_globalFileId = reader.m_localFileId; 
      } else {
         reader.m_globalFileId = H5Fcreate( fileName.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT ); 
      }
      FileHandler::s_writeDTime += MPI_Wtime() - startTime;

      if( reader.m_globalFileId < 0 ) {
         std::cout << " ERROR Cannot open or create the global file " << fileName << std::endl;
         status = 1; 
      }
   }
   
   startTime = MPI_Wtime();   
   MPI_Bcast( &status, 1, MPI_INT, 0, comm );
   FileHandler::s_collectingTime += MPI_Wtime() - startTime;

   if( status > 0 ) {
      // global file cannot be created
      H5Fclose( reader.m_localFileId ); 
      if( reader.m_rank == 0 ) {       
         std::cout << " ERROR Cannot close the local file " << fileName << std::endl;
      }
      return false;
   }
   // Iterate over the memebers of the local file
    
   iteration_status = H5Giterate ( reader.m_localFileId, "/", 0, readDataset, &reader );

   startTime = MPI_Wtime();
   status = H5Fclose( reader.m_localFileId );    
   FileHandler::s_readDTime += MPI_Wtime() - startTime;
  
   close_status = reader.checkError( status ); 
    
   if( reader.m_rank == 0 ) {
      if( !reader.m_reuse ) {
         startTime = MPI_Wtime();   
         status = H5Fclose( reader.m_globalFileId );  
         FileHandler::s_writeDTime += MPI_Wtime() - startTime;
      }
   }
 
   startTime = MPI_Wtime();   
   MPI_Bcast( &status, 1, MPI_INT, 0, comm );
   FileHandler::s_collectingTime += MPI_Wtime() - startTime;

   FileHandler::s_totalTime += MPI_Wtime() - totalStart;
   
   if( status < 0 || close_status < 0 || iteration_status < 0 ) {
      if( reader.m_rank == 0 ) {       
         std::cout << " ERROR Cannot exit while merging the file " << fileName << std::endl;
      }
      return false;
   }
   
   return true;
}

bool copyFile( std::string & dstPath, std::string & curPath )    
{
   bool status = true;
   const size_t bufSize = 8 * 1024 * 1024;
   char buf[ bufSize ];
   size_t size;
   
   int source = open( curPath.c_str(), O_RDONLY, 0 );
   int dest = open( dstPath.c_str(),  O_CREAT | O_WRONLY | O_TRUNC, 0644 );
   
   if( source < 0 || dest < 0 ) {
      perror(" CopyFile: ");

      status = false;
   }
   if( status ) {
      while (( size = read(source, buf, bufSize )) > 0) {
         if( write(dest, buf, size) < 0 ) {
            perror(" CopyFile: ");
            status = false;
            break;  
         }
      }
   }
   
   close(source);
   close(dest);

   return status;
}
