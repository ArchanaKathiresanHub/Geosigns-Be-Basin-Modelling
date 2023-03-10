#include "mpi.h"
#include <string.h>
#include "stdlib.h"
#include <sstream>
#include <iostream>

#include "hdf5.h"
#include "fileHandler.h"

#include "H5FDmpio.h"
//#define SAFE_RUN 1

bool errorFlag = false;

void reduce_op( float *invec, float *inoutvec, int *len, MPI_Datatype *datatype ) {

  for( int i = 0; i < * len; ++ i ) {

    if( invec[i] != 0 ) {
      if( inoutvec[i] != 0 ) {
        errorFlag = true;
      }
      inoutvec[i] = invec[i];
    }
  }
}

void FileHandler::openLocalFile( hid_t fileAccessPList ) {

  m_localFileId = H5Fopen( m_fileName.c_str(), H5F_ACC_RDONLY, fileAccessPList );

}

void FileHandler::openGlobalFile () {

  if( m_rank == 0 ) {
    m_globalFileId = H5Fcreate( m_fileName.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT );
  }

}

hid_t FileHandler::closeGlobalFile() {
  if( m_rank == 0 ) {
    return H5Fclose( m_globalFileId );
  }
  return 0;
}

void FileHandler::createGroup( const char* name ) {

  if( m_rank == 0 ) {
    // Greate a group in the global file
    m_groupId = H5Gcreate( m_globalFileId, name,  H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );

    if( m_groupId < 0 ) {
      std::cout << " ERROR Cannot open or create the group " << name << std::endl;
    }
  }
}

void FileHandler::closeSpaces( ) {
  H5Sclose( m_filespace );
  H5Sclose( m_memspace );
}

void FileHandler::closeGlobalDset( ) {
  H5Dclose( m_global_dset_id );
}

void FileHandler::writeAttributes( ) {

  int status = readAttributes( m_local_dset_id, m_global_dset_id );
  if( status < 0 ) {
    std::cout << " ERROR Cannot write attributes" << std::endl;
  }

}

void FileHandler::createDataset( const char* name , hid_t dtype ) {

  if( m_rank == 0 ) {

    // Write the global data into the global file
    m_memspace = H5Screate_simple( m_spatialDimension, m_count, NULL );

    if( m_groupId != H5P_DEFAULT ) {
      // Dataset is under the sub-group
      m_global_dset_id = H5Dcreate( m_groupId, name, dtype, m_memspace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
    } else {
      // Dataset is under the main group
      m_global_dset_id = H5Dcreate( m_globalFileId, name, dtype, m_memspace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
    }

    if( m_global_dset_id < 0 ) {
      std::cout << " ERROR Cannot create the group" << name << std::endl;
    }
    m_filespace = H5Dget_space( m_global_dset_id );
    H5Sselect_hyperslab( m_filespace, H5S_SELECT_SET, m_offset, NULL, m_count, NULL );
  }

}

void FileHandler::setSpatialDimension( int dimension ) {
  m_spatialDimension = dimension;
}

bool FileHandler::mergeFiles( const bool appendRank ) {

  hid_t status = 0;
  hid_t close_status = 0;
  hid_t iteration_status = 0;

  hid_t fileAccessPList = H5Pcreate(H5P_FILE_ACCESS);

  std::stringstream tmpName;
  if( appendRank ) {
    tmpName << m_tempDirName << "/{NAME}_{MPI_RANK}";
  } else {
    tmpName << m_tempDirName << "/{NAME}";
  }

  openLocalFile( fileAccessPList );

  H5Pclose( fileAccessPList );

  if( checkError( m_localFileId ) < 0 ) {
    if( m_rank == 0 ) {
      std::cout << " ERROR Cannot open the local file " << m_fileName << std::endl;
    }
    return false;
  }
  openGlobalFile();

  if( m_rank == 0 && m_globalFileId < 0 ) {
    std::cout << " ERROR Cannot open or create the global file " << m_fileName << std::endl;
    status = 1;
  }

  MPI_Bcast( &status, 1, MPI_INT, 0, m_comm );

  if( status > 0 ) {
    // global file cannot be created
    H5Fclose( m_localFileId );
    if( m_rank == 0 ) {
      std::cout << " ERROR Cannot close the local file " << m_fileName << std::endl;
    }
    return false;
  }
  // Iterate over the memebers of the local file

  iteration_status = H5Giterate ( m_localFileId, "/", 0, readDataset, this );

  status = H5Fclose( m_localFileId );

  close_status = checkError( status );

  status = closeGlobalFile();

  MPI_Bcast( &status, 1, MPI_INT, 0, m_comm );

  if( status < 0 || close_status < 0 || iteration_status < 0 ) {
    if( m_rank == 0 ) {
      std::cout << " ERROR Cannot exit while merging the file " << m_fileName << std::endl;
    }
    return false;
  }

  return true;

}

FileHandler::FileHandler ( MPI_Comm comm, const std::string & fileName, const std::string & tempDirName ) :
  m_comm(comm),
  m_fileName(fileName),
  m_tempDirName(tempDirName)
{
  MPI_Comm_rank( comm, &m_rank );

  for( int d = 0; d < MAX_FILE_DIMENSION; ++ d ) {
    m_offset [d] = 0;
    m_count  [d] = 0;
    m_dimensions [d] = 0;
  }

  MPI_Op_create(( MPI_User_function *)reduce_op, true, &m_op );
}

FileHandler::~FileHandler() {

  MPI_Op_free ( &m_op );
}

herr_t FileHandler::reallocateBuffers ( ssize_t dataSize ) {

  size_t valNumber = 0;
  size_t valCount  = 0;

  if( dataSize > 0 ) {
    valNumber = 1;

    for (int d = 0; d < m_spatialDimension && d < MAX_FILE_DIMENSION; ++d) {
      m_count [d] = m_dimensions[d];
      valNumber *= m_count[d];
    }
  }

  if( m_spatialDimension > 1 ) {
    m_data.resize( valNumber );
    m_sumData.resize( valNumber );
  } else {
    valCount = valNumber * dataSize;
    m_data1D.resize( valCount );
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

      m_attrData.resize(  dataSize * attrSize );

      status = H5Aread( localAttrId, dataTypeId, m_attrData.data() );

      if( status < 0 ) {
        H5Tclose( dataTypeId );
        H5Aclose( localAttrId );

        return -1;
      }
      // write Attribute
      hid_t space = H5Screate_simple ( dims, spatialDims, NULL );

      hid_t globalAttrId = H5Acreate( globalDataSetId, attrName, dataTypeId, space, H5P_DEFAULT, H5P_DEFAULT );

      status = H5Awrite ( globalAttrId, dataTypeId, m_attrData.data() );

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


int FileHandler::checkError ( hid_t value ) {

  int local_status  = 0;
  int global_status = 0;

  if( value < 0 ) {
    local_status = 1;
  }
  MPI_Allreduce ( &local_status, &global_status,  1, MPI_INT, MPI_SUM, m_comm );

  if( global_status > 0 ) {
    return -1;
  }

  return 0;

}

void readGroup(const char* name, FileHandler* reader, void* voidReader)
{
  std::stringstream groupName;
  groupName << "/" << name;

  reader->createGroup( name );

  reader->m_groupName = groupName.str().c_str();
  // Iterate over the members of the group
  H5Giterate ( reader-> m_localFileId, groupName.str().c_str(), nullptr, readDataset, voidReader );

  if( reader->m_rank == 0 ) {
    H5Gclose( reader->m_groupId );
  }
}

herr_t readDataset ( hid_t groupId, const char* name, void * voidReader)  {

  FileHandler* reader = static_cast<FileHandler *>( voidReader );

  if( groupId < 0 ) {

    if( reader->m_rank == 0 ) {
      std::cout << " ERROR Cannot iterate the group or dataset " << name << std::endl;
    }
    return -1;
  }

  hid_t dataSpaceId;
  int   status = 0;

  H5G_stat_t statbuf;

  status = H5Gget_objinfo ( groupId, name, 0, &statbuf );

  if( reader->checkError( status ) < 0 ) {
    if( reader->m_rank == 0 ) {
      std::cout << " ERROR Cannot get the object " <<  name << " info" << std::endl;
    }
    return -1;
  }

  switch ( statbuf.type ) {
    case H5G_GROUP:
      {
        readGroup(name, reader, voidReader);
        return 0;
      }
    case H5G_DATASET:
      break;
    case H5G_TYPE:
      break;
    default: ;

  }

  reader->m_local_dset_id = H5Dopen ( groupId, name, H5P_DEFAULT );

#ifdef SAFE_RUN
  if( reader->checkError ( reader->m_local_dset_id ) < 0 ) {
    return -1;
  }
#endif

  dataSpaceId = H5Dget_space ( reader->m_local_dset_id );

  reader->m_spatialDimension = H5Sget_simple_extent_dims ( dataSpaceId, reader->m_dimensions, 0 );

  H5Sclose( dataSpaceId );

  // Get the dataset datatype
  hid_t   dtype    = H5Dget_type( reader->m_local_dset_id );
  ssize_t dataSize = H5Tget_size(dtype);

  H5T_class_t storageTypeId = H5Tget_class( dtype );

  bool isFloatType = ( storageTypeId == H5T_FLOAT );

#ifdef SAFE_RUN
  if( reader->checkError ( dataSize ) < 0 || dataSize < 0 ) {
    return -1;
  }
#endif

  // Allocate data buffers
  reader->reallocateBuffers ( dataSize );

  // Read the local data
  reader->m_memspace = H5Screate_simple( reader->m_spatialDimension, reader->m_count, NULL );
  reader->m_filespace = H5Dget_space( reader->m_local_dset_id );

  H5Sselect_hyperslab( reader->m_filespace, H5S_SELECT_SET, reader->m_offset, NULL, reader->m_count, NULL );

  if( reader->m_spatialDimension > 1 ) {
    if( isFloatType ) {
      status = reader->merge2D ( name, dtype );
    } else {
      if( reader->m_rank == 0 ) {
        std::cout << " ERROR 2D dataset datatype is not FLOAT" << std::endl;
      }
      status = -1;
    }
  } else {
    status = reader->merge1D ( name, dtype );
  }

  if( reader->m_rank == 0 ) {

    if( status >= 0 ) {
      reader->writeAttributes();
    } else {
      std::cout << " ERROR Cannot write the dataset " << name << " into the group " << reader-> m_groupName << std::endl;
    }

    H5Sclose( reader->m_filespace );
    H5Sclose( reader->m_memspace );

    reader->closeGlobalDset();
  }

  H5Dclose( reader->m_local_dset_id );

#ifdef SAFE_RUN
  MPI_Bcast( &status, 1, MPI_INT, 0, reader->m_comm );

  if( status < 0 ) {
    return -1;
  }
#endif

  return 0;
}

herr_t FileHandler::merge1D ( const char* name, hid_t  dtype ) {

  herr_t status = H5Dread ( m_local_dset_id, dtype, m_memspace, m_filespace, H5P_DEFAULT, m_data1D.data() );

  closeSpaces();

  if( m_rank == 0 && status >= 0 ) {
    createDataset( name, dtype );

    status = H5Dwrite( m_global_dset_id, dtype, m_memspace, m_filespace, H5P_DEFAULT, m_data1D.data() );
  }
  return status;
}

herr_t FileHandler::merge2D ( const char* name, hid_t dtype ) {

  herr_t status = H5Dread ( m_local_dset_id, dtype, m_memspace, m_filespace, H5P_DEFAULT, m_data.data() );

  if( status < 0 ) {
    if( m_rank == 0 ) {
      H5Eprint ( H5E_DEFAULT, 0 );
      std::cout << "ERROR Cannot read dataset " << name << std::endl;
    }
    return status;
  }

  closeSpaces();

  // Collect all local data in a global array on rank 0
  if( m_rank == 0 ) {
    std::fill( m_sumData.begin(), m_sumData.end(), 0 );
  }

  errorFlag = false;

  MPI_Reduce( m_data.data(), m_sumData.data(), m_data.size(), MPI_FLOAT, m_op, 0, m_comm );

  if( errorFlag ) {
    if( m_rank == 0 ) {
      std::cout << "Basin_Warning: Duplicated data found in " << name << " dataset" << std::endl;
    }
  }

  if( m_rank == 0 && status >= 0 ) {
    createDataset( name, dtype );

    status = H5Dwrite( m_global_dset_id, dtype, m_memspace, m_filespace, H5P_DEFAULT, m_sumData.data() );
    if( status < 0 ) {
      if( m_rank == 0 ) {
        H5Eprint ( H5E_DEFAULT, 0 );
        std::cout << "ERROR Cannot write dataset " << name << std::endl;
      }
      return status;
    }
  }
  return status;
}

void FileHandler::setGlobalFileId( hid_t id ) {
  m_globalFileId = id;

}

void FileHandler::setLocalFileId( hid_t aLocalFileId ) {
  m_localFileId = aLocalFileId;
}

void FileHandler::setGroupId( hid_t anId ) {
  m_groupId = anId;
}

void FileHandler::setGlobalDsetId( hid_t anId ) {
  m_global_dset_id = anId;
}

void FileHandler::setMemspace( hid_t aMemspace ) {
  m_memspace = aMemspace;
}

void FileHandler::setFilespace( hid_t aFilespace ) {
  m_filespace = aFilespace;
}

const char * FileHandler::getFileName() const {
  return m_fileName.c_str();
}
