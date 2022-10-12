//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#if (defined (_WIN32) || defined (_WIN64))
#define NOMINMAX
#endif

#include "SerialGridMap.h"

// std
#include <cassert>
#include <algorithm>
#include <limits>

// SerialDataAccess
#include "SerialGrid.h"

//utilities
#include "array.h"

using namespace DataAccess;
using namespace Interface;
using namespace ibs;
using namespace std;


SerialGridMap::SerialGridMap (const Parent * owner, unsigned int childIndex, const Grid * grid, double undefinedValue, unsigned int depth, float *** values) :
       GridMap (owner, childIndex), m_grid (grid),   m_undefinedValue (undefinedValue), m_averageValue (m_undefinedValue), m_depth (depth)
{
   m_values = Array < double >::create3d( static_cast<size_t>(grid->numI()), static_cast<size_t>(grid->numJ()), static_cast<size_t>( m_depth ), m_undefinedValue);
   m_singleValue = m_undefinedValue;

   const unsigned int numI = grid->numI ();
   const unsigned int numJ = grid->numJ ();

   bool first = true;
   bool isConstant = true;
   double constantValue = DefaultUndefinedMapValue;

   for (unsigned int i = 0; i < numI; ++i)
   {
      for (unsigned int j = 0; j < numJ; ++j)
      {
         for (unsigned int k = 0; k < m_depth; ++k)
         {
            m_values[i][j][k] = static_cast<double>(values[i][j][m_depth - 1 - k]);
            float temp = m_values[i][j][k];
            
            if (first && temp != DefaultUndefinedMapValue)
            {
                constantValue = m_values[i][j][k];
                first = false;
            }
            if (isConstant && !first && temp!= DefaultUndefinedMapValue)
            {
                isConstant = temp == constantValue;
            }
         }
      }
   }
   if (isConstant)
   {
      ibs::Array<double>::delete3d (m_values);
      m_values = nullptr;
      m_singleValue = constantValue;
      m_averageValue = m_singleValue;
   }
}

SerialGridMap::SerialGridMap (const Parent * owner, unsigned int childIndex, const Grid * grid, double value, unsigned int depth) :
      GridMap (owner, childIndex), m_grid (grid), m_undefinedValue (DefaultUndefinedMapValue), m_averageValue (value), m_depth (depth)
{
   m_values = nullptr;
   m_singleValue = value;
}

SerialGridMap::SerialGridMap (const Parent * owner, unsigned int childIndex, const GridMap * operand1, const GridMap * operand2, BinaryOperator binaryOperator) :
      GridMap (owner, childIndex), m_grid (const_cast<Grid *>(operand1->getGrid())),
      m_undefinedValue (operand1->getUndefinedValue ()), m_averageValue (operand1->getUndefinedValue ()),
      m_depth (operand1->getDepth ())
{
   const unsigned int numI = m_grid->numI ();
   const unsigned int numJ = m_grid->numJ ();

   assert (numI == static_cast<unsigned int>(operand2->getGrid ()->numI ()));
   assert (numJ == static_cast<unsigned int>(operand2->getGrid ()->numJ ()));

   assert (m_depth == operand2->getDepth ());

   if (operand1->isConstant () && operand2->isConstant ())
   {
      m_singleValue = (* binaryOperator) (operand1->getConstantValue (), operand2->getConstantValue ());
      m_averageValue = m_singleValue;
      m_values = nullptr;
      return;
   }

   m_values = Array < double >::create3d (numI, numJ, m_depth, m_undefinedValue);
   m_singleValue = m_undefinedValue;

   for (unsigned int i = 0; i < numI; ++i)
   {
      for (unsigned int j = 0; j < numJ; ++j)
      {
         for (unsigned int k = 0; k < m_depth; ++k)
         {
            if (!operand1->valueIsDefined (i, j, k) || !operand2->valueIsDefined (i, j, k))
            {
               m_values[i][j][k] = m_undefinedValue;
            }
            else
            {
               m_values[i][j][k] = (* binaryOperator) (operand1->getValue (i, j, k), operand2->getValue (i, j, k));
            }
         }
      }
   }
}

SerialGridMap::SerialGridMap (const Parent * owner, unsigned int childIndex, const GridMap * operand, UnaryOperator unaryOperator) :
      GridMap (owner, childIndex), m_grid (const_cast<Grid *>(operand->getGrid())),
      m_undefinedValue (operand->getUndefinedValue ()), m_averageValue (operand->getUndefinedValue ()),
      m_depth (operand->getDepth ())
{
   const unsigned int numI = m_grid->numI ();
   const unsigned int numJ = m_grid->numJ ();

   if (operand->isConstant ())
   {
      m_singleValue = (* unaryOperator) (operand->getConstantValue ());
      m_averageValue = m_singleValue;
      m_values = nullptr;
      return;
   }
   m_values = Array < double >::create3d (numI, numJ, m_depth, m_undefinedValue);
   m_singleValue = m_undefinedValue;

   for (unsigned int i = 0; i < numI; ++i)
   {
      for (unsigned int j = 0; j < numJ; ++j)
      {
         for (unsigned int k = 0; k < m_depth; ++k)
         {
            if (!operand->valueIsDefined (i, j, k))
            {
               m_values[i][j][k] = m_undefinedValue;
            }
            else
            {
               m_values[i][j][k] = (* unaryOperator) (operand->getValue (i, j, k));
            }
         }
      }
   }
}


SerialGridMap::SerialGridMap (const Parent * owner, unsigned int childIndex, const GridMap * operand1, const GridMap * operand2, BinaryFunctor& binaryFunctor) :
      GridMap (owner, childIndex), m_grid (const_cast<Grid *>(operand1->getGrid())),
      m_undefinedValue (operand1->getUndefinedValue ()), m_averageValue (operand1->getUndefinedValue ()),
      m_depth (operand1->getDepth ())
{
   const unsigned int numI = m_grid->numI ();
   const unsigned int numJ = m_grid->numJ ();

   assert (numI == static_cast<unsigned int>(operand2->getGrid ()->numI ()));
   assert (numJ == static_cast<unsigned int>(operand2->getGrid ()->numJ ()));

   assert (m_depth == operand2->getDepth ());

   if (operand1->isConstant () && operand2->isConstant ())
   {
      m_singleValue = binaryFunctor (operand1->getConstantValue (), operand2->getConstantValue ());
      m_averageValue = m_singleValue;
      m_values = nullptr;
      return;
   }

   m_values = Array < double >::create3d (numI, numJ, m_depth, m_undefinedValue);
   m_singleValue = m_undefinedValue;

   for (unsigned int i = 0; i < numI; ++i)
   {
      for (unsigned int j = 0; j < numJ; ++j)
      {
         for (unsigned int k = 0; k < m_depth; ++k)
         {
            if (!operand1->valueIsDefined (i, j, k) || !operand2->valueIsDefined (i, j, k))
            {
               m_values[i][j][k] = m_undefinedValue;
            }
            else
            {
               m_values[i][j][k] = binaryFunctor (operand1->getValue (i, j, k), operand2->getValue (i, j, k));
            }
         }
      }
   }
}

SerialGridMap::SerialGridMap (const Parent * owner, unsigned int childIndex, const GridMap * operand, UnaryFunctor& unaryFunctor) :
      GridMap (owner, childIndex), m_grid (const_cast<Grid *>(operand->getGrid())),
      m_undefinedValue (operand->getUndefinedValue ()), m_averageValue (operand->getUndefinedValue ()),
      m_depth (operand->getDepth ())
{
   const unsigned int numI = m_grid->numI ();
   const unsigned int numJ = m_grid->numJ ();

   if (operand->isConstant ())
   {
      m_singleValue = unaryFunctor (operand->getConstantValue ());
      m_averageValue = m_singleValue;
      m_values = nullptr;
      return;
   }
   m_values = Array < double >::create3d (numI, numJ, m_depth, m_undefinedValue);
   m_singleValue = m_undefinedValue;

   for (unsigned int i = 0; i < numI; ++i)
   {
      for (unsigned int j = 0; j < numJ; ++j)
      {
         for (unsigned int k = 0; k < m_depth; ++k)
         {
            if (!operand->valueIsDefined (i, j, k))
            {
               m_values[i][j][k] = m_undefinedValue;
            }
            else
            {
               m_values[i][j][k] = unaryFunctor (operand->getValue (i, j, k));
            }
         }
      }
   }
}




SerialGridMap::~SerialGridMap ()
{
   if (m_values) Array<double>::delete3d (m_values);
}

bool SerialGridMap::retrieveGhostedData () const
{
   return true;
}

bool SerialGridMap::retrieveData (bool ) const
{
   return true;
}

bool SerialGridMap::restoreData (bool , bool ) const
{
   return true;
}

double SerialGridMap::getUndefinedValue () const
{
   return m_undefinedValue;
}

/// Check if GridMap is constant
bool SerialGridMap::isConstant () const
{
   return m_values == nullptr && m_singleValue != m_undefinedValue;
}

/// Return the constant value. returns the undefined value if not constant
double SerialGridMap::getConstantValue () const
{
   return m_singleValue;
}

const Grid * SerialGridMap::getGrid () const
{
   return m_grid;
}

double SerialGridMap::getValue (unsigned int i, unsigned int j) const
{
    return getValue(i, j, static_cast<unsigned int>(0));
}

double SerialGridMap::getValue (unsigned int i, unsigned int j, unsigned int k) const
{
   if (m_grid->isGridPoint (i, j) && k < m_depth)
   {
      if (m_values)
         return m_values[i][j][k];
      else
         return m_singleValue;
   }
   else
      return m_undefinedValue;
}

bool SerialGridMap::isAscendingOrder() const {
   return false;
}

double SerialGridMap::getValue (unsigned int i, unsigned int j, double k) const
{
   return getValue ( static_cast<double>(i), static_cast<double>(j), k);
}

double SerialGridMap::getValue (double i, double j, double k) const
{
   const double MinOffset = 1e-6;

   const auto baseI = static_cast<unsigned int>(i);
   const auto baseJ = static_cast<unsigned int>(j);
   const auto baseK = static_cast<unsigned int>(k);

   double fractionI = i - static_cast<double>(baseI);
   double fractionJ = j - static_cast<double>(baseJ);
   double fractionK = k - static_cast<double>(baseK);

   if (fractionI <= MinOffset) fractionI = 0;
   if (fractionJ <= MinOffset) fractionJ = 0;
   if (fractionK <= MinOffset) fractionK = 0;

   if (fractionI >= 1 - MinOffset) fractionI = 1;
   if (fractionJ >= 1 - MinOffset) fractionJ = 1;
   if (fractionK >= 1 - MinOffset) fractionK = 1;

   if (fractionI == 0 && fractionJ == 0 && fractionK == 0)
   {
      return getValue (baseI, baseJ, baseK);
   }

   if (fractionI < 1 && fractionJ < 1 && fractionK < 1 &&
         !valueIsDefined (baseI,     baseJ,     baseK    )) return m_undefinedValue;

   if (fractionI < 1 && fractionJ < 1 && fractionK > 0 &&
         !valueIsDefined (baseI,     baseJ,     baseK + 1)) return m_undefinedValue;

   if (fractionI < 1 && fractionJ > 0 && fractionK < 1 &&
         !valueIsDefined (baseI,     baseJ + 1, baseK    )) return m_undefinedValue;

   if (fractionI < 1 && fractionJ > 0 && fractionK > 0 &&
         !valueIsDefined (baseI,     baseJ + 1, baseK + 1)) return m_undefinedValue;

   if (fractionI > 0 && fractionJ < 1 && fractionK < 1 &&
         !valueIsDefined (baseI + 1, baseJ,     baseK    )) return m_undefinedValue;

   if (fractionI > 0 && fractionJ < 1 && fractionK > 0 &&
         !valueIsDefined (baseI + 1, baseJ,     baseK + 1)) return m_undefinedValue;

   if (fractionI > 0 && fractionJ > 0 && fractionK < 1 &&
         !valueIsDefined (baseI + 1, baseJ + 1, baseK    )) return m_undefinedValue;

   if (fractionI > 0 && fractionJ > 0 && fractionK > 0 &&
         !valueIsDefined (baseI + 1, baseJ + 1, baseK + 1)) return m_undefinedValue;

   const double value =
      getFractionalValue (fractionI         * fractionJ         * fractionK,         baseI + 1, baseJ + 1, baseK + 1) +
      getFractionalValue (fractionI         * fractionJ         * (1.0 - fractionK), baseI + 1, baseJ + 1, baseK    ) +
      getFractionalValue (fractionI         * (1.0 - fractionJ) * fractionK,         baseI + 1, baseJ,     baseK + 1) +
      getFractionalValue (fractionI         * (1.0 - fractionJ) * (1.0 - fractionK), baseI + 1, baseJ,     baseK    ) +
      getFractionalValue ((1.0 - fractionI) * fractionJ         * fractionK,         baseI,     baseJ + 1, baseK + 1) +
      getFractionalValue ((1.0 - fractionI) * fractionJ         * (1.0 - fractionK), baseI,     baseJ + 1, baseK    ) +
      getFractionalValue ((1.0 - fractionI) * (1.0 - fractionJ) * fractionK,         baseI,     baseJ,     baseK + 1) +
      getFractionalValue ((1.0 - fractionI) * (1.0 - fractionJ) * (1.0 - fractionK), baseI,     baseJ,     baseK    );

      return value;
}

double SerialGridMap::getFractionalValue (double fraction, unsigned int i, unsigned int j, unsigned int k) const
{
   if (fraction > 0) return fraction * getValue (i, j, k);
   else return 0;
}

bool SerialGridMap::valueIsDefined (unsigned int i, unsigned int j, unsigned int k) const
{
   return getValue (i, j, k) != m_undefinedValue;
}

bool SerialGridMap::setValue (unsigned int i, unsigned int j, double value)
{
   return setValue (i, j, 0, value);
}

bool SerialGridMap::setValue (unsigned int i, unsigned int j, unsigned int k, double value)
{
   if (m_grid->isGridPoint (i, j) && k < m_depth)
   {
      if (!m_values && m_singleValue != value)
      {
         m_values = Array < double >::create3d (m_grid->numI (), m_grid->numJ (), m_depth, m_singleValue);
      }
      if (m_values) m_values[i][j][k] = value;
      return true;
   }
   else
   {
      return false;
   }
}

void SerialGridMap::setValues( const double value ){
   if (m_values != nullptr){
      Array < double >::delete3d( m_values );
      m_values = nullptr;
   }
   m_singleValue = value;
}

double SerialGridMap::getAverageValue () const
{
   unsigned int numValues = 0;
   double total = 0;

   if (m_averageValue != getUndefinedValue ())
   {
      return m_averageValue;
   }

   for (unsigned int i = 0; i < static_cast<unsigned int>(m_grid->numI()); i++)
   {
      for (unsigned int j = 0; j < static_cast<unsigned int>(m_grid->numJ()); j++)
      {
         for (unsigned int k = 0; k < m_depth; k++)
         {
            const double value = getValue (i, j, k);
            if (value != getUndefinedValue ())
            {
               numValues++;
               total += value;
            }
         }
      }
   }

   if (numValues > 0)
      m_averageValue = total / numValues;
   else
      m_averageValue = getUndefinedValue ();

   return m_averageValue;
}

/// return the minimum & maximum value calculated over all GridPoints with a defined value
void SerialGridMap::getMinMaxValue (double & min, double & max) const
{
   min = std::numeric_limits< double >::max();
   max = -std::numeric_limits< double >::max();

   for (unsigned int i = 0; i < static_cast<unsigned int>(m_grid->numI ()); i++)
   {
      for (unsigned int j = 0; j < static_cast<unsigned int>(m_grid->numJ ()); j++)
      {
         for (unsigned int k = 0; k < m_depth; k++)
         {
            double value = getValue (i, j, k);
            if (value != getUndefinedValue ())
            {
               max = std::max (value, max);
               min = std::min (value, min);
            }
         }
      }
   }
   if (min == std::numeric_limits< double >::max() )
      min = getUndefinedValue ();
   if (max == -std::numeric_limits< double >::max() )
      max = getUndefinedValue ();
}

double SerialGridMap::getSumOfValues () const
{
   double total = 0;

   for (unsigned int i = 0; i < static_cast<unsigned int>(m_grid->numI ()); i++)
   {
      for (unsigned int j = 0; j < static_cast<unsigned int>(m_grid->numJ ()); j++)
      {
         for (unsigned int k = 0; k < m_depth; k++)
         {
            const double value = getValue (i, j, k);
            if (value != getUndefinedValue ())
            {
               total += value;
            }
         }
      }
   }

   return total;
}

double SerialGridMap::getSumOfSquaredValues () const
{
   double total = 0;

   for (unsigned int i = 0; i < static_cast<unsigned int>(m_grid->numI ()); i++)
   {
      for (unsigned int j = 0; j < static_cast<unsigned int>(m_grid->numJ ()); j++)
      {
         for (unsigned int k = 0; k < m_depth; k++)
         {
            const double value = getValue (i, j, k);

            if (value != getUndefinedValue ())
            {
               total += value * value;
            }
         }
      }
   }

   return total;
}

int SerialGridMap::getNumberOfDefinedValues () const
{
   int numValues = 0;

   for (unsigned int i = 0; i < static_cast<unsigned int>(m_grid->numI ()); i++)
   {
      for (unsigned int j = 0; j < static_cast<unsigned int>(m_grid->numJ ()); j++)
      {
         for (unsigned int k = 0; k < m_depth; k++)
         {
            const double value = getValue (i, j, k);
            if (value != getUndefinedValue ())
            {
               numValues++;
            }
         }
      }
   }

   return numValues;
}

unsigned int SerialGridMap::getDepth () const
{
   return m_depth;
}

double const * const * const * SerialGridMap::getValues () const
{
   if (!m_values)
   {
      m_values = Array < double >::create3d (m_grid->numI (), m_grid->numJ (), m_depth, m_singleValue);
   }

   return m_values;
}

void SerialGridMap::printOn (std::ostream & ostr) const
{
   const unsigned int depth = getDepth ();
   const unsigned int numI = getGrid ()->numI ();
   const unsigned int numJ = getGrid ()->numJ ();

   ostr << "GridMap: ";
   ostr << "depth = " << depth;
   ostr << ", numI = " << numI;
   ostr << ", numJ = " << numJ;
   ostr << ", undefinedValue = " << getUndefinedValue ();
   ostr << endl;

   for (unsigned int k = 0; k < depth; ++k)
   {
      for (unsigned int i = 0; i < numI; ++i)
      {
      for (unsigned int j = 0; j < numJ; ++j)
      {
         if (j != 0) ostr << ", ";
         const double value = getValue (i, j, k);
         ostr << value;
      }
      ostr << endl;
      }
      ostr << endl;
   }
}

///map interpolation functionality
bool SerialGridMap::convertToGridMap(GridMap *mapB) const
{
   bool ret;

   const Grid *gridA = this->getGrid();
   const Grid *gridB = mapB->getGrid();
   if( gridA->numI() >= gridB->numI()  && gridA->numJ() >= gridB->numJ() )
   {
      ret = transformHighRes2LowRes(mapB);
   }
   else
   {
      ret = transformLowRes2HighRes(mapB);
   }

   return ret;
}

bool SerialGridMap::transformHighRes2LowRes(GridMap *mapB) const
{
   const Grid *gridB = mapB->getGrid();

   bool ret = true;

   unsigned int indexJmapA;
   unsigned int indexImapB, indexJmapB;

   unsigned int indexImapA = indexJmapA = indexImapB = indexJmapB = 0;

   const unsigned int depthA = this->getDepth ();

   const Grid *highResGridA = const_cast<Grid *>(this->getGrid());
   for (indexImapB = 0; indexImapB < static_cast<unsigned int>(gridB->numI ()); ++indexImapB)
   {
      for (indexJmapB = 0; indexJmapB < static_cast<unsigned int>(gridB->numJ ()); ++indexJmapB)
      {
         if (gridB->convertToGrid ( (*highResGridA), indexImapB, indexJmapB, indexImapA, indexJmapA) )
         {
            for (unsigned int k = 0; k < depthA; k++)
            {
               mapB->setValue (indexImapB, indexJmapB, k, this->getValue (indexImapA, indexJmapA, k));
            }
         }
         else
         {
            cerr << "conversion from lowres (" << indexImapB << ", " << indexJmapB <<
               ") to highres (" << indexImapA << ", " << indexJmapA << ") failed unexpectedly" << endl;
            ret = false;
            break;
         }
      }
   }


   return ret;
}

bool SerialGridMap::transformLowRes2HighRes(GridMap *mapB,bool extrapolateAOI) const
{
   const GridMap *mapA = this;
   const Grid *gridA = (Grid *)mapA->getGrid();
   const Grid *gridB = (Grid *)mapB->getGrid();

   bool ret = true;

   unsigned int highResI, highResJ, k;
   unsigned int depthB = mapB->getDepth();

   for (highResI = 0; highResI < static_cast<unsigned int>(gridB->numI()); ++highResI)
   {
      for (highResJ = 0; highResJ < static_cast<unsigned int>(gridB->numJ()); ++highResJ)
      {
         double doubleLowResI, doubleLowResJ;
         if (!gridB->convertToGrid(*gridA, highResI, highResJ, doubleLowResI, doubleLowResJ))
         {
            // one of the four surrounding lowres grid points is outside the highres grid
            // when extrapolating, still want to calulcate something (see below)
            if(!extrapolateAOI){
               for (k = 0; k < depthB; k++)
               {
                  mapB->setValue(highResI, highResJ, k, DefaultUndefinedMapValue);
               }
               continue;
            }
         }
         const auto intLowResI = static_cast<unsigned int>(doubleLowResI);
         const auto intLowResJ = static_cast<unsigned int>(doubleLowResJ);

         const double errorMargin = 1e-4;

         double fractionI = doubleLowResI - intLowResI;
         if (fractionI < errorMargin) fractionI = 0;
         if (fractionI > 1 - errorMargin) fractionI = 1;

         double fractionJ = doubleLowResJ - intLowResJ;
         if (fractionJ < errorMargin) fractionJ = 0;
         if (fractionJ > 1 - errorMargin) fractionJ = 1;

         for (k = 0; k < depthB; k++)
         {
            double lowResMapValues[2][2];

            lowResMapValues[0][0] = mapA->getValue(intLowResI, intLowResJ, k);
            lowResMapValues[0][1] = mapA->getValue(intLowResI, intLowResJ + 1, k);
            lowResMapValues[1][0] = mapA->getValue(intLowResI + 1, intLowResJ, k);
            lowResMapValues[1][1] = mapA->getValue(intLowResI + 1, intLowResJ + 1, k);

            double highResMapValue = 0;

            if ((lowResMapValues[0][0] == mapA->getUndefinedValue() && fractionI != 1 && fractionJ != 1) ||
               (lowResMapValues[0][1] == mapA->getUndefinedValue() && fractionI != 1 && fractionJ != 0) ||
                    (lowResMapValues[1][0] == mapA->getUndefinedValue() && fractionI != 0 && fractionJ != 1) ||
                         (lowResMapValues[1][1] == mapA->getUndefinedValue() && fractionI != 0 && fractionJ != 0))
            {
               //inside point with at least one corner point missing!
               highResMapValue = mapB->getUndefinedValue();
               // calculate when extrapolating!
               if(extrapolateAOI ){
                  int cornerPoints= (int) (lowResMapValues[0][0] != mapA->getUndefinedValue ()) +
                                    (int) (lowResMapValues[0][1] != mapA->getUndefinedValue ()) +
                                    (int) (lowResMapValues[1][0] != mapA->getUndefinedValue ()) +
                                    (int) (lowResMapValues[1][1] != mapA->getUndefinedValue ());
                  if( cornerPoints==3){
                      // top-right missing
                      if(lowResMapValues[1][1] == mapA->getUndefinedValue()){
                         //set the top-right
                         lowResMapValues[1][1]=(lowResMapValues[1][0]+lowResMapValues[0][1])*0.5;
                      }
                      // top-left missing
                      if(lowResMapValues[0][1] == mapA->getUndefinedValue()){
                         //set the topleft
                         lowResMapValues[0][1]=(lowResMapValues[0][0]+lowResMapValues[1][1])*0.5;
                      }
                      // bottom-right missing
                      if(lowResMapValues[1][0] == mapA->getUndefinedValue()){
                         //set the bottom-right
                         lowResMapValues[1][0]=(lowResMapValues[0][0]+lowResMapValues[1][1])*0.5;
                      }
                      // bottom-left missing
                      if(lowResMapValues[0][0] == mapA->getUndefinedValue()){
                         //set the bottom-left
                         lowResMapValues[0][0]=(lowResMapValues[1][0]+lowResMapValues[0][1])*0.5;
                      }
                      highResMapValue = 0.0;
                      highResMapValue += lowResMapValues[0][0] * (1 - fractionI) * (1 - fractionJ);
                      highResMapValue += lowResMapValues[0][1] * (1 - fractionI) * (fractionJ);
                      highResMapValue += lowResMapValues[1][0] * (fractionI) * (1 - fractionJ);
                      highResMapValue += lowResMapValues[1][1] * (fractionI) * (fractionJ);
                  }
                  if (cornerPoints==2){
                     // 6 cases of two corner points! bottom, top, left, right, and two diagonal
                     //
                     // bottom
                     if(lowResMapValues[0][0] != mapA->getUndefinedValue () && lowResMapValues[1][0] != mapA->getUndefinedValue ()      ){
                         highResMapValue=(lowResMapValues[0][0]*(1-fractionI)) + (lowResMapValues[1][0] * fractionI);
                     }
                     // top
                     if(lowResMapValues[0][1] != mapA->getUndefinedValue () && lowResMapValues[1][1] != mapA->getUndefinedValue ()      ){
                         highResMapValue=(lowResMapValues[0][1]*(1-fractionI)) + (lowResMapValues[1][1] * fractionI);
                     }
                     // left
                     if(lowResMapValues[0][0] != mapA->getUndefinedValue () && lowResMapValues[0][1] != mapA->getUndefinedValue ()      ){
                         highResMapValue=(lowResMapValues[0][0]*(1-fractionJ)) + (lowResMapValues[0][1] * fractionJ);
                     }
                     // right
                     if(lowResMapValues[1][0] != mapA->getUndefinedValue () && lowResMapValues[1][1] != mapA->getUndefinedValue ()      ){
                         highResMapValue=(lowResMapValues[1][0]*(1-fractionJ)) + (lowResMapValues[1][1] * fractionJ);
                     }
                     // main diagonal (this is a wired geometry AOI, nothing better than set the value to the nearest existing val     ue)
                     if(lowResMapValues[0][0] != mapA->getUndefinedValue () && lowResMapValues[1][1] != mapA->getUndefinedValue ()      ){
                         //choose closest point!
                         if( fractionI + fractionJ < 1 ){
                            highResMapValue=lowResMapValues[0][0];
                         }else{
                            highResMapValue=lowResMapValues[1][1];
                         }
                     }
                     // 2nd diagonal (this is a wired geometry AOI, nothing better than set the value to the nearest existing valu     e)
                     if(lowResMapValues[1][0] != mapA->getUndefinedValue () && lowResMapValues[0][1] != mapA->getUndefinedValue ()      ){
                         //choose closest point!
                         if( fractionI < fractionJ ){
                            highResMapValue=lowResMapValues[1][0];
                         }else{
                            highResMapValue=lowResMapValues[0][1];
                         }
                     }
                  }
                  // if there is only one corner point. Just set the value of that corner point
                  if (cornerPoints==1){
                     if (lowResMapValues[0][0] != mapA->getUndefinedValue () ){
                       highResMapValue=lowResMapValues[0][0];
                     }else if (lowResMapValues[0][1] != mapA->getUndefinedValue () ){
                       highResMapValue=lowResMapValues[0][1];
                     }else if (lowResMapValues[1][0] != mapA->getUndefinedValue () ){
                       highResMapValue=lowResMapValues[1][0];
                     }else if (lowResMapValues[1][1] != mapA->getUndefinedValue () ){
                       highResMapValue=lowResMapValues[1][1];
                     }
                  }
                  // if there are no corner points, undifiend!
               }



            }
            else
            //bi-linear interpolation, 4 corner points present

            {
               highResMapValue += lowResMapValues[0][0] * (1 - fractionI) * (1 - fractionJ);
               highResMapValue += lowResMapValues[0][1] * (1 - fractionI) * (fractionJ);
               highResMapValue += lowResMapValues[1][0] * (fractionI) * (1 - fractionJ);
               highResMapValue += lowResMapValues[1][1] * (fractionI) * (fractionJ);
            }

            //set the value
            mapB->setValue(highResI, highResJ, k, highResMapValue);
            }
      }
   }

   //mapB->printOn (PETSC_COMM_WORLD);

   mapA->restoreData (true, true);
   mapB->restoreData ();

   return ret;
}

unsigned int SerialGridMap::numI (void) const
{
   return m_grid->numI();
}

unsigned int SerialGridMap::numJ (void) const
{
   return m_grid->numJ();
}

double SerialGridMap::minI (void) const
{
   return m_grid->minIGlobal () + firstI () * deltaI ();
}

double SerialGridMap::minJ (void) const
{
   return m_grid->minJGlobal () + firstJ () * deltaJ ();
}

double SerialGridMap::deltaI (void) const
{
       return m_grid->deltaI ();
}

double SerialGridMap::deltaJ (void) const
{
       return m_grid->deltaJ ();
}
