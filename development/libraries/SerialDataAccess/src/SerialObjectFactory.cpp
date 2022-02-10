#include <iostream>
using namespace std;


#include "ObjectFactory.h"
#include "SerialGrid.h"
#include "SerialGridMap.h"
#include "SerialMapWriter.h"

using namespace DataAccess;
using namespace Interface;

GridMap * ObjectFactory::produceGridMap (const Parent * owner, unsigned int childIndex,
      const Grid * grid, double undefinedValue, unsigned int depth, float *** values) const
{
   return new SerialGridMap (owner, childIndex, grid, undefinedValue, depth, values);
}
GridMap * ObjectFactory::produceGridMap (const Parent * owner, unsigned int childIndex,
      const Grid * grid, double value, unsigned int depth) const
{
   return new SerialGridMap (owner, childIndex, grid, value, depth);
}

GridMap * ObjectFactory::produceGridMap (const Parent * owner, unsigned int childIndex,
      const GridMap * operand1, const GridMap * operand2, BinaryOperator binaryOperator) const
{
   return new SerialGridMap (owner, childIndex, operand1, operand2, binaryOperator);
}

GridMap * ObjectFactory::produceGridMap (const Parent * owner, unsigned int childIndex,
      const GridMap * operand1, const GridMap * operand2, BinaryFunctor& binaryFunctor ) const
{
   return new SerialGridMap (owner, childIndex, operand1, operand2, binaryFunctor);
}

GridMap * ObjectFactory::produceGridMap (const Parent * owner, unsigned int childIndex,
      const GridMap * operand, UnaryOperator unaryOperator) const
{
   return new SerialGridMap (owner, childIndex, operand, unaryOperator);
}

GridMap * ObjectFactory::produceGridMap (const Parent * owner, unsigned int childIndex,
      const GridMap * operand, UnaryFunctor& unaryFunctor) const
{
   return new SerialGridMap (owner, childIndex, operand, unaryFunctor);
}

Grid * ObjectFactory::produceGrid (double minI, double minJ, double maxI, double maxJ, unsigned int numI, unsigned int numJ, unsigned int ,
                                   unsigned int, const std::vector<std::vector<int>>&) const
{
   return new SerialGrid (minI, minJ, maxI, maxJ, numI, numJ);
}

Grid * ObjectFactory::produceGrid (const Grid * referenceGrid, double minI, double minJ, double maxI, double maxJ, unsigned int numI, unsigned int numJ) const
{
   return new SerialGrid (referenceGrid, minI, minJ, maxI, maxJ, numI, numJ);
}

MapWriter* ObjectFactory::produceMapWriter (void) const
{
   return new SerialMapWriter ( );
}
