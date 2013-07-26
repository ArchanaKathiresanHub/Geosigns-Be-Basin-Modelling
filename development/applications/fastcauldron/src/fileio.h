#ifndef _FILEIO_H_
#define _FILEIO_H_

#include <string>
using namespace std;

// void checkFileSystem( string& str );

string generatePropertyFileName( const string&  outputDirectory, 
				 const bool     isMinorSnapshot, 
				 const double   time );

string generatePropertyGridName( string        propertyName, 
				 string        surfaceName,
				 string        formationName, 
				 const double  time );


#endif /* _FILEIO_H_ */
