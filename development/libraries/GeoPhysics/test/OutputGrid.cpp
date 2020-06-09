#include "OutputGrid.h"


/// Constructor sets size and provides dummy initialization to storageArray and usage
OutputGrid::OutputGrid( int theSize )
   : m_theSize( theSize )
   , m_storageArray( theSize )
{
   for( int n = 0; n < this->m_theSize; ++n )
	  m_storageArray[ n] = 0.0;
}

int OutputGrid::getSize() const
{
   return this->m_theSize;
}

double * OutputGrid::getVRe()
{
   return & this->m_storageArray[0];
}

double OutputGrid::printVR() const
{
   return m_storageArray[0];
}

