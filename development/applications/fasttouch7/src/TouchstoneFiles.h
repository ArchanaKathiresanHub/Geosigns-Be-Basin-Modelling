#ifndef FASTTOUCH7_TOUCHSTONEFILES_H
#define FASTTOUCH7_TOUCHSTONEFILES_H

#include <vector>

#include "DataPipe.h"
#include <map>
#include <vector>

//this class defines how the touchstone results should be read

class TouchstoneFiles 
{
private:
   DataPipe  m_filename;
   static const int numCategories = 6;
public:
// read methods
   TouchstoneFiles(const char * filename);
   void readOrder(std::vector<int> & order);
   void readNumTimeSteps(size_t * numTimeSteps);
   void readArray(std::vector<double> & OutputProperties);
// write methods
	void writeOrder(std::map <int, int> categoriesMappingOrder) ;
	void writeNumTimeSteps(size_t numTimeSteps);  
	void writeArray(std::vector<double> & OutputProperties); 
};

#endif
