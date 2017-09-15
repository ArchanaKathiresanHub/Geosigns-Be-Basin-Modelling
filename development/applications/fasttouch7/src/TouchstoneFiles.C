//                                                                      
// Copyright (C) 2012-2017 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

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
   for ( size_t ii = 0; ii < numCategories; ++ii ) m_filename >> vec[ii];
}

void TouchstoneFiles::readNumTimeSteps(size_t * numTimeSteps)
{
   m_filename >> *numTimeSteps;
}

void TouchstoneFiles::readArray(std::vector<double> & outputProperties)
{
   for ( size_t ii = 0; ii < outputProperties.size(); ++ii ) m_filename >> outputProperties[ii];
}

//write functions

void TouchstoneFiles::writeOrder(std::map <int, int> categoriesMappingOrder)
{
   m_filename << categoriesMappingOrder.size();
   for ( size_t ii = 0; ii < categoriesMappingOrder.size(); ++ii ) m_filename << categoriesMappingOrder[ii];
}

void TouchstoneFiles::writeNumTimeSteps(size_t numTimeSteps)
{
   m_filename << numTimeSteps;
}

void TouchstoneFiles::writeArray(std::vector<double> & outputProperties)
{
   for ( size_t ii = 0; ii < outputProperties.size(); ++ii ) m_filename << outputProperties[ii];
}

