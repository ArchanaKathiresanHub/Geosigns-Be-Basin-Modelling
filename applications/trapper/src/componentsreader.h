/// Class ComponensReader inherits from DataReader
/// and redefines some of the protected virtual methods
/// that handle tables reading details

#ifndef __componentsreader__
#define __componentsreader__

#include "datareader.h"

class ComponentsReader : public DataReader
{
public:
   string readData(const char* filename);
   static double getTotalGas (Record *rec);
   static double getTotalOil (Record *rec);
   static double getWetGas (Record *rec);
protected:
   virtual void readRecord (Record *rec);
   virtual void initLineData (int size);
};

#endif
