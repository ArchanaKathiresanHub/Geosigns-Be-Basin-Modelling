#include "Interface/Snapshot.h"
#include "Interface/GridMap.h"

#include "LocalGridInterpolator.h"


namespace GenexSimulation
{

using DataAccess::Interface::Snapshot;
using DataAccess::Interface::GridMap;

LocalGridInterpolator::LocalGridInterpolator()
{
   m_A = 0;
   m_B = 0;
   m_firstI = m_lastI = m_firstJ = m_lastJ = 0;
   m_rows = m_columns = 0;
}
LocalGridInterpolator::~LocalGridInterpolator()
{
  clear();
}
void LocalGridInterpolator::setValueFromGridMap(const double &value, const int &GridI,const int &GridJ, BufferType theType)
{
   int dataI = GridI - m_firstI;
   int dataJ = GridJ - m_firstJ;
   switch(theType)
   {
      case A:
        m_A[dataI][dataJ] = value; 
      break;
      case B:
        m_B[dataI][dataJ] = value; 
      break;
      default:
      break;
   }
}
double LocalGridInterpolator::getValueForGrid(const int &GridI, const int &GridJ, BufferType theType) const
{
   int dataI = GridI - m_firstI;
   int dataJ = GridJ - m_firstJ;
   double ret = 0.0;

	//assert(dataI > m_lastI || dataJ > m_lastJ);

   switch(theType)
   {
      case A:
        ret = m_A[dataI][dataJ];
      break;
      case B:
        ret = m_B[dataI][dataJ];
      break;
      default:
      break;
   }
   return ret;
}
double LocalGridInterpolator::evaluateProperty(const int &I, const int &J, const double &in_timeInstance) const
{
   BufferType typeA=A;
   BufferType typeB=B;
   
   return (getValueForGrid(I,J,typeA) + getValueForGrid(I,J,typeB) * in_timeInstance);
}
void LocalGridInterpolator::initialize(const GridMap *theGrid)
{
   m_firstI = theGrid->firstI();
   m_lastI  = theGrid->lastI();
   m_firstJ = theGrid->firstJ();
   m_lastJ  = theGrid->lastJ();
   
   m_rows   = m_lastI - m_firstI + 1;
   m_columns= m_lastJ - m_firstJ + 1;
     
   m_A = allocate2DArray(m_rows, m_columns);
   m_B = allocate2DArray(m_rows, m_columns);
}
void LocalGridInterpolator::clear()
{
   delete2DArray(m_A, m_rows);
   delete2DArray(m_B, m_rows);
   m_firstI = m_lastI = m_firstJ = m_lastJ = 0;
   m_rows = m_columns = 0;   
}

void LocalGridInterpolator::compute(const Snapshot *start, const GridMap *dataAtStart, const Snapshot *end, const GridMap *dataAtEnd)
{  
   dataAtStart->retrieveData();
   dataAtEnd->retrieveData();
    
   //if first time(most likely)
   if(!m_A || !m_B )
   {
      initialize(dataAtStart); 
   }
   //if matrices have been allocated, check for bounds... 
   else if(m_A && m_B )
   {
     //if bounds are different clear and reallocate
      if ( 
           m_firstI != dataAtStart->firstI() ||
           m_lastI  != dataAtStart->lastI () ||
           m_firstJ != dataAtStart->firstJ() ||
           m_lastJ  != dataAtStart->lastJ () 
         )
      {
         clear(); 
         initialize(dataAtStart);  
      } 
   }
   
   //main functionality
   double aTime = start->getTime();
   double bTime = end->getTime();
   double BMinusA = bTime - aTime;
     
   double aValue = 0.0;
   double bValue = 0.0;
   unsigned int depth = dataAtStart->getDepth();    
   unsigned int i = 0;
   unsigned int j = 0;
   for(i = dataAtStart->firstI(); i <= dataAtStart->lastI(); ++i)
   {
      for(j = dataAtStart->firstJ(); j<= dataAtStart->lastJ(); ++j)
      {
         aValue = dataAtStart->getValue(i, j, depth - 1);
         bValue = dataAtEnd  ->getValue(i, j, depth - 1);

         double valueA = bTime * aValue / BMinusA - aTime * bValue / BMinusA;
         double valueB = (bValue - aValue)/BMinusA;
         
         setValueFromGridMap(valueA, i, j, A);
         setValueFromGridMap(valueB, i, j, B);
      }
   }
     
   dataAtStart->restoreData();
   dataAtEnd->restoreData();
 
}
double **LocalGridInterpolator::allocate2DArray(const int &rows, const int &columns)
{
  int i, j;
  double **array=0;
  array = new double *[rows];
  for(i = 0; i < rows; i++)
  {
    array[i] = new double[columns];
    for(j=0; j < columns; j++)
    {
      array[i][j] = 0.0;
    }
  }
  return array;
}
void LocalGridInterpolator::delete2DArray(double **array, const int &rows)
{
   int i;
   if(array)
   {
      for(i = 0; i < rows; i++)
      {
         delete [] array[i];
      }
      delete [] array;
      array = 0;
   }
}

}
// 
  // 
  //             ( x - a )            ( b - x )
  //     P(x) =  --------- * f(b)  +  --------- * f (a)
  //             ( b - a )            ( b - a )
  // 
  // 
  //             x * ( f(b) - f(a))    b * f (a)    a * f (b)
  //          =  ------------------ +  --------  -  ---------
  //                  ( b - a )        ( b - a )    ( b - a )
  // 
  //          = A + B x
  // 
  // 
  //        b * f (a)     a * f (b)
  //   A =  ---------  -  ---------
  //        ( b - a )     ( b - a )
  // 
  // 
  //        ( f(b) - f(a))    
  //   B  =  --------------
  //           ( b - a )
  // 
  // 
  // 
  //  Rearranging the calculation like this reduces the flop count
  //  considerably, from 6 (including a division) to 2, per evaluation.
  // 
  // 


