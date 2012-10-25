#include "volumereader.h"

//
// body of Volume Reader, dervied from Data Reader
// to read volume data from Cauldron Project File
//

//
// PUBLIC METHODS
//
string VolumeReader::readData(const char* filename)
{
 string error;
 openProject (filename);

 return error;
}

//
// PRIVATE METHODS
//
void VolumeReader::readRecord (Record *rec)
{
}

