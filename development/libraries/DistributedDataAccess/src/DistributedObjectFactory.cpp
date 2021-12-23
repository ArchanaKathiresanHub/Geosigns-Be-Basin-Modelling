#include <iostream>
using namespace std;

#include "ObjectFactory.h"
#include "DistributedGrid.h"
#include "DistributedGridMap.h"
#include "DistributedMapWriter.h"

using namespace DataAccess;
using namespace Interface;

GridMap * ObjectFactory::produceGridMap (const Parent * owner, unsigned int childIndex,
      const Grid * grid, double undefinedValue, unsigned int depth, float *** values) const
{
   return new DistributedGridMap (owner, childIndex, grid, undefinedValue, depth, values);
}
GridMap * ObjectFactory::produceGridMap (const Parent * owner, unsigned int childIndex,
      const Grid * grid, double value, unsigned int depth) const
{
   return new DistributedGridMap (owner, childIndex, grid, value, depth);
}

GridMap * ObjectFactory::produceGridMap (const Parent * owner, unsigned int childIndex,
      const GridMap * operand1, const GridMap * operand2, BinaryOperator binaryOperator) const
{
   return new DistributedGridMap (owner, childIndex, operand1, operand2, binaryOperator);
}

GridMap * ObjectFactory::produceGridMap (const Parent * owner, unsigned int childIndex,
      const GridMap * operand1, const GridMap * operand2, BinaryFunctor& binaryFunctor ) const
{
   return new DistributedGridMap (owner, childIndex, operand1, operand2, binaryFunctor);
}

GridMap * ObjectFactory::produceGridMap (const Parent * owner, unsigned int childIndex,
      const GridMap * operand, UnaryOperator unaryOperator) const
{
   return new DistributedGridMap (owner, childIndex, operand, unaryOperator);
}

GridMap * ObjectFactory::produceGridMap (const Parent * owner, unsigned int childIndex,
      const GridMap * operand, UnaryFunctor& unaryFunctor) const
{
   return new DistributedGridMap (owner, childIndex, operand, unaryFunctor);
}

Grid * ObjectFactory::produceGrid (double minI, double minJ, double maxI, double maxJ, unsigned int numI, unsigned int numJ, unsigned int lowResNumI, unsigned int lowResNumJ,
                                   std::vector<std::vector<int>> domainShape, const double maxDeviation) const
{
   return new DistributedGrid (minI, minJ, maxI, maxJ, numI, numJ, lowResNumI, lowResNumJ, domainShape, maxDeviation);
}

Grid * ObjectFactory::produceGrid (const Grid * referenceGrid, double minI, double minJ, double maxI, double maxJ, unsigned int numI, unsigned int numJ) const
{
   return new DistributedGrid (referenceGrid, minI, minJ, maxI, maxJ, numI, numJ);
}

MapWriter* ObjectFactory::produceMapWriter (void) const
{
   return new DistributedMapWriter ( );
}
