#ifdef sgi
   #ifdef _STANDARD_C_PLUS_PLUS
      #include<iostream>
      using namespace std;
   #else // !_STANDARD_C_PLUS_PLUS
      #include<iostream.h>
   #endif // _STANDARD_C_PLUS_PLUS
#else // !sgi
   #include <iostream>
   using namespace std;
#endif // sgi

#include "Interface/ObjectFactory.h"
#include "Interface/SerialGrid.h"
#include "Interface/SerialGridMap.h"
#include "Interface/SerialMapWriter.h"

using namespace DataAccess;
using namespace Interface;

GridMap * ObjectFactory::produceGridMap (const Parent * owner, unsigned int childIndex,
      const Grid * grid, double undefinedValue, unsigned int depth, float *** values)
{
   return new SerialGridMap (owner, childIndex, grid, undefinedValue, depth, values);
}
GridMap * ObjectFactory::produceGridMap (const Parent * owner, unsigned int childIndex,
      const Grid * grid, double value, unsigned int depth)
{
   return new SerialGridMap (owner, childIndex, grid, value, depth);
}

GridMap * ObjectFactory::produceGridMap (const Parent * owner, unsigned int childIndex,
      const GridMap * operand1, const GridMap * operand2, BinaryOperator binaryOperator)
{
   return new SerialGridMap (owner, childIndex, operand1, operand2, binaryOperator);
}

GridMap * ObjectFactory::produceGridMap (const Parent * owner, unsigned int childIndex,
      const GridMap * operand1, const GridMap * operand2, BinaryFunctor& binaryFunctor )
{
   return new SerialGridMap (owner, childIndex, operand1, operand2, binaryFunctor);
}

GridMap * ObjectFactory::produceGridMap (const Parent * owner, unsigned int childIndex,
      const GridMap * operand, UnaryOperator unaryOperator)
{
   return new SerialGridMap (owner, childIndex, operand, unaryOperator);
}

GridMap * ObjectFactory::produceGridMap (const Parent * owner, unsigned int childIndex,
      const GridMap * operand, UnaryFunctor& unaryFunctor)
{
   return new SerialGridMap (owner, childIndex, operand, unaryFunctor);
}

Grid * ObjectFactory::produceGrid (double minI, double minJ, double maxI, double maxJ, unsigned int numI, unsigned int numJ, unsigned int lowResNumI, unsigned int lowResNumJ)
{
   return new SerialGrid (minI, minJ, maxI, maxJ, numI, numJ);
}

Grid * ObjectFactory::produceGrid (const Grid * referenceGrid, double minI, double minJ, double maxI, double maxJ, unsigned int numI, unsigned int numJ)
{
   return new SerialGrid (referenceGrid, minI, minJ, maxI, maxJ, numI, numJ);
}

MapWriter* ObjectFactory::produceMapWriter (void)
{
   return new SerialMapWriter ( );
}
