#ifndef _FILEIO_H_
#define _FILEIO_H_

#include <string>

// void checkFileSystem( string& str );

std::string generatePropertyFileName( const std::string&  outputDirectory,
				 const bool     isMinorSnapshot, 
				 const double   time );

std::string generatePropertyGridName(std::string        propertyName,
	std::string        surfaceName,
	std::string        formationName,
				 const double  time );


#endif /* _FILEIO_H_ */
