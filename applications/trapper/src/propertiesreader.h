/// Class PropertiesReader inherits from DataReader and
/// creates its own version of readData and readRecord to 
/// specifically read property info

#ifndef __propertiesreader__
#define __propertiesreader__

#include "datareader.h"

class PropertiesReader : public DataReader
{
public:
   string readData(const char * filename);

private:
   void readRecord (Record *rec);
   void initLineData (int size);
};

#endif
