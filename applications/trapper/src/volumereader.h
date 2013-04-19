#ifndef __volreader__
#define __volreader__

#include "datareader.h"

class VolumeReader : public DataReader
{
public:
 string readData(const char* filename);
 
private:
 void readRecord (Record *rec);
};

#endif
