#ifndef DATA_PIPE_H
#define DATA_PIPE_H

#include <cstdio>
#include <cstring>
#include <cstdlib>

#include <stdexcept>

//#define TXT_PIPE 
//#define LOG_READING_OPERATIONS 

# if !defined( TXT_PIPE ) && !defined( BIN_PIPE )
#define BIN_PIPE
#endif

#ifdef TXT_PIPE

#define MAX_TYPE_NAME_LENGTH 128

inline bool writeData( FILE * fp, const int    & v ) { return  fprintf( fp, "int %d\n",     v ) > 0; }
inline bool writeData( FILE * fp, const size_t & v ) { return  fprintf( fp, "size_t %zu\n", v ) > 0; }
inline bool writeData( FILE * fp, const double & v ) { return  fprintf( fp, "double %f\n",  v ) > 0; }

template <typename T> inline bool parseData( FILE * fp, const char * fmt, const char * typeName, T & v )
{
   char tpNm[MAX_TYPE_NAME_LENGTH];
   return fscanf( fp, fmt, tpNm, &v )  == 2 && !strcmp( tpNm, typeName );
}

inline bool readData( FILE * fp, int    & v ) { return parseData( fp, "%128s %d\n",  "int",    v ); }
inline bool readData( FILE * fp, size_t & v ) { return parseData( fp, "%128s %zu\n", "size_t", v ); }
inline bool readData( FILE * fp, double & v ) { return parseData( fp, "%128s %lf\n", "double", v ); }
#endif

#ifdef BIN_PIPE
template <typename T> inline bool writeData( FILE * fp, const T & v ) { return fwrite( &v, sizeof( v ), 1, fp ) == 1; }
template <typename T> inline bool readData(  FILE * fp,       T & v ) { return fread(  &v, sizeof( v ), 1, fp ) == 1; }
#endif

class DataPipe
{
public:
   DataPipe( const char * filename ) 
      : m_fileName( filename )
      , m_fptr(0)
#ifdef LOG_READING_OPERATIONS
      , m_logfptr(0)
#endif
   {
   }

   ~DataPipe()
   {
      if ( m_fptr    ) { fclose( m_fptr ); }
#ifdef LOG_READING_OPERATIONS
      if ( m_logfptr ) { fclose( m_logfptr ); }
#endif
   }

   template <typename T> DataPipe & operator << ( const T & outData )
   {
      if ( !m_fptr ) m_fptr = fopen( m_fileName, "w" );
      if ( !m_fptr ) throw std::runtime_error( "Can not open file for writing" );

      if ( !writeData( m_fptr, outData ) ) throw std::runtime_error( "Can not write to DataPipe" );
      return *this;
   }

   template <typename T> DataPipe & operator >> ( T & inData )
   {
      if ( !m_fptr ) m_fptr = fopen( m_fileName, "rb" );
      if ( !m_fptr ) throw std::runtime_error( "Can not open file for reading" );
#ifndef LOG_READING_OPERATIONS
      bool ok = readData( m_fptr, inData );
#else
      if ( !m_logfptr ) m_logfptr = fopen( "datapipe.log", "w" );
      if ( !m_logfptr ) throw std::runtime_error( "Can not open file for logging" );

      bool ok = readData( m_fptr, inData );           // read data 
      ok = ok ? writeData( m_logfptr, inData ) : ok;  // duplicate data to log file
#endif
      if (! ok ) throw std::runtime_error( "Can not read from DataPipe" );
      return *this;
   }

private:

   DataPipe( const DataPipe & );
   DataPipe & operator = ( const DataPipe * &);

   const char * m_fileName; // file name which will be used for pipe
   FILE       * m_fptr;     // file handle

#ifdef LOG_READING_OPERATIONS
   FILE       * m_logfptr; // file handle for log file
#endif
};

#endif
