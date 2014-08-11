#include "mpi.h"
#include "hdf5.h"
#include "fileHandler.h"

double FileHandler::s_collectingTime = 0.0;
double FileHandler::s_readingDTime = 0.0;
double FileHandler::s_writingDTime = 0.0;
double FileHandler::s_creatingDTime = 0.0;
double FileHandler::s_creatingGTime = 0.0;
double FileHandler::s_readingATime = 0.0;
double FileHandler::s_writingATime = 0.0;
double FileHandler::s_attributeTime = 0.0;
double FileHandler::s_readDTime = 0.0;
double FileHandler::s_writeDTime = 0.0;
double FileHandler::s_totalTime = 0.0;

FileHandler:: FileHandler ( MPI_Comm comm ) {

   m_comm = comm;
   MPI_Comm_rank( comm, &m_rank );

   m_valCount  = 0;
   m_attrCount = 0;

   m_groupId   = H5P_DEFAULT;
   m_localFileId = H5P_DEFAULT;
   m_globalFileId = H5P_DEFAULT;

   for( int d = 0; d < MAX_FILE_DIMENSION; ++ d ) {
      m_offset [d] = 0;
      m_count  [d] = 0;
      m_dimensions [d] = 0;
   }
   m_spatialDimension = 0;
   m_reuse = false;

}

herr_t FileHandler::reallocateBuffers ( ssize_t dataSize ) {

   if( dataSize >= 0 ) {
      m_valCount = dataSize;
      
      for (int d = 0; d < m_spatialDimension; ++d) {
         m_count [d] = m_dimensions[d];       
         m_valCount *= m_count[d];
      } 
      m_data.resize( m_valCount );
      m_sumData.resize( m_valCount );
   }
   return 0;
}

herr_t FileHandler::readAttributes( hid_t localDataSetId, hid_t globalDataSetId )  {

   hid_t status;
   // Get the number of attributes for the dataSet.
   int attrNumber = H5Aget_num_attrs(localDataSetId);
   
   hsize_t spatialDims [MAX_FILE_DIMENSION];
   char attrName [MAX_ATTRIBUTE_NAME_SIZE];
   
   // Iterate over the number of Attributes 
   if( attrNumber > 0 ) {
      
      for( int indx = 0; indx < attrNumber; ++ indx ) {
         // read Attribute
         hid_t localAttrId = H5Aopen_idx(localDataSetId, indx );   
         
         status = H5Aget_name( localAttrId, MAX_ATTRIBUTE_NAME_SIZE, attrName );   
         attrName[status] = '\0';
         
         hid_t dataTypeId = H5Aget_type( localAttrId );
         
         hid_t dataSpace  = H5Aget_space( localAttrId );
         
         int   dims = H5Sget_simple_extent_dims( dataSpace, spatialDims, 0 );
         
         size_t attrSize = H5Sget_simple_extent_npoints( dataSpace );
         
         H5Sclose( dataSpace );
         //  Get the size of the type
         ssize_t dataSize = H5Tget_size(dataTypeId);
         
         m_attrCount = dataSize * attrSize;
         m_attrData.resize( m_attrCount );
         
         double startTime = MPI_Wtime();
         status = H5Aread( localAttrId, dataTypeId, m_attrData.data() );
         FileHandler::s_readingATime += MPI_Wtime() - startTime;

         if( status < 0 ) {
            H5Tclose( dataTypeId );
            H5Aclose( localAttrId );

            return -1;
         }
         // write Attribute
         hid_t space = H5Screate_simple ( dims, spatialDims, NULL );
         
         hid_t globalAttrId = H5Acreate( globalDataSetId, attrName, dataTypeId, space, H5P_DEFAULT, H5P_DEFAULT );

         startTime = MPI_Wtime();
         status = H5Awrite ( globalAttrId, dataTypeId, m_attrData.data() );
         FileHandler::s_writingATime +=  MPI_Wtime() - startTime;
         
         H5Tclose( dataTypeId );
         H5Aclose( localAttrId );
         H5Aclose( globalAttrId );
         H5Sclose( space );

         if( status < 0 ) {
            return -1;
         }
      } 
   } 
   return 0;
}

void FileHandler::setGlobalId( hid_t id ) {
   m_globalFileId = id;

}

void FileHandler::setSpatialDimension( int dimension ) {
   m_spatialDimension = dimension;

}
void FileHandler::setReuseOption( const bool anOption ) {
   m_reuse = anOption;

}

int FileHandler::checkError ( hid_t value ) {
   
   int local_status  = 0;
   int global_status = 0;
   
   if( value < 0 ) {
      local_status = 1;
   }
   double startTime = MPI_Wtime();
   MPI_Allreduce ( &local_status, &global_status,  1, MPI_INT, MPI_SUM, m_comm ); 
   FileHandler::s_collectingTime += MPI_Wtime() - startTime;

   if( global_status > 0 ) {
      return -1;
   }
 
   return 0;

}
