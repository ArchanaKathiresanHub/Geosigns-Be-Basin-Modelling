#include "InputGrid.h"

/// Constructor sets time and size and provides dummy initialization to temperature and usage
InputGrid::InputGrid( double theTime, double theTemperature, int theSize )
   : m_theTime( theTime ),
     m_theSize( theSize ),
     m_theTemperature( m_theSize ),
     m_theActiveNodes()
{
   for( int n = 0; n < this->m_theSize; ++n )
   {
      m_theTemperature[n] = theTemperature;

      if ( n % 2 == 0 )
         m_theActiveNodes.push_back( n ) ;
   }
}


double InputGrid::getTime() const
{
   return this->m_theTime;
}

int InputGrid::getSize() const
{
   return this->m_theSize;
}

const double * InputGrid::getTemperature() const
{
   return & this->m_theTemperature[0];
}

/// Alternating usage of nodes. Even numbers are used, odd ones not
const int * InputGrid::getActiveNodes() const
{
   return & this->m_theActiveNodes[0];
}

int InputGrid::getNumberOfActiveNodes() const
{ 
   return this->m_theActiveNodes.size(); 
}

