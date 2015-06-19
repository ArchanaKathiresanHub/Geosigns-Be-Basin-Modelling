#include "TouchstoneFiles.h"

#include <iostream>

TouchstoneFiles::TouchstoneFiles(const char * filename)
   : m_filename(filename)
{
}

//read functions
void TouchstoneFiles::readOrder(std::vector<int> & vec)
{
   size_t numCategories;
   m_filename >> numCategories;
   vec.resize(numCategories);
   for ( int ii = 0; ii < numCategories; ++ii ) m_filename >> vec[ii];
}

void TouchstoneFiles::readNumTimeSteps( size_t * numTimeSteps)
{
   m_filename >> *numTimeSteps;
}

void TouchstoneFiles::readArray( std::vector<double> & outputProperties )
{
   for(int ii = 0; ii < outputProperties.size( ); ++ii) m_filename >> outputProperties [ ii ] ;			
}

//write functions

void TouchstoneFiles::writeOrder(std::map <int, int> categoriesMappingOrder )
{
   m_filename << categoriesMappingOrder.size();
   for ( int ii = 0; ii < categoriesMappingOrder.size(); ++ii ) m_filename << categoriesMappingOrder[ii];
}

void TouchstoneFiles::writeNumTimeSteps(size_t numTimeSteps)
{
   m_filename << numTimeSteps;
}

void TouchstoneFiles::writeArray(std::vector<double> & outputProperties )
{
   for(int ii = 0; ii < outputProperties.size( ); ++ii) m_filename << outputProperties [ ii ] ;	
}


