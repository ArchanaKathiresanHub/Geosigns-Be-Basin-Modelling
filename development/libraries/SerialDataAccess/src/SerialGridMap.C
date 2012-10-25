#include <math.h>

#if (defined (_WIN32) || defined (_WIN64))
#define NOMINMAX
#include <stdafx.h>
#else
#include <values.h>
#endif

#include <assert.h>

#include "Interface/SerialGridMap.h"

#include "Interface/SerialGrid.h"

#include "array.h"

#include "hdf5funcs.h"
#include "hdf5.h"

using namespace DataAccess;
using namespace Interface;

#define Min(a,b)        (a < b ? a : b)
#define Max(a,b)        (a > b ? a : b)


SerialGridMap::SerialGridMap (const Parent * owner, unsigned int childIndex, const Grid * grid, double undefinedValue, unsigned int depth, float *** values) :
	    GridMap (owner, childIndex), m_grid (grid),	m_undefinedValue (undefinedValue), m_averageValue (m_undefinedValue), m_depth (depth)
{
   m_values = Array < double >::create3d (grid->numI (), grid->numJ (), m_depth, m_undefinedValue);
   m_singleValue = m_undefinedValue;

   unsigned int numI = grid->numI ();
   unsigned int numJ = grid->numJ ();

   bool first = true;
   bool isConstant = true;
   double constantValue;

   for (unsigned int i = 0; i < numI; ++i)
   {
      for (unsigned int j = 0; j < numJ; ++j)
      {
         for (unsigned int k = 0; k < m_depth; ++k)
         {
            m_values[i][j][k] = (double) values[i][j][m_depth -1 - k];
            if (first) 
            {
               constantValue = m_values[i][j][k];
               first = false;
            }
            else if (isConstant && constantValue != m_values[i][j][k])
            {
               isConstant = false;
            }
         }
      }
   }
   if (isConstant)
   {
      Array<double>::delete3d (m_values);
      m_values = 0;
      m_singleValue = constantValue;
      m_averageValue = m_singleValue;
   }
}

SerialGridMap::SerialGridMap (const Parent * owner, unsigned int childIndex, const Grid * grid, double value, unsigned int depth) :
      GridMap (owner, childIndex), m_grid (grid), m_undefinedValue (99999), m_averageValue (value), m_depth (depth)
{
   m_values = 0;
   m_singleValue = value;
}

SerialGridMap::SerialGridMap (const Parent * owner, unsigned int childIndex, const GridMap * operand1, const GridMap * operand2, BinaryOperator binaryOperator) :
      GridMap (owner, childIndex), m_grid ((Grid *) operand1->getGrid ()),
      m_undefinedValue (operand1->getUndefinedValue ()), m_averageValue (operand1->getUndefinedValue ()),
      m_depth (operand1->getDepth ())
{
   unsigned int numI = m_grid->numI ();
   unsigned int numJ = m_grid->numJ ();

   assert (numI == operand2->getGrid ()->numI ());
   assert (numJ == operand2->getGrid ()->numJ ());

   assert (m_depth == operand2->getDepth ());

   if (operand1->isConstant () && operand2->isConstant ())
   {
      m_singleValue = (* binaryOperator) (operand1->getConstantValue (), operand2->getConstantValue ());
      m_averageValue = m_singleValue;
      m_values = 0;
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
      GridMap (owner, childIndex), m_grid ((Grid *) operand->getGrid ()),
      m_undefinedValue (operand->getUndefinedValue ()), m_averageValue (operand->getUndefinedValue ()),
      m_depth (operand->getDepth ())
{
   unsigned int numI = m_grid->numI ();
   unsigned int numJ = m_grid->numJ ();

   if (operand->isConstant ())
   {
      m_singleValue = (* unaryOperator) (operand->getConstantValue ());
      m_averageValue = m_singleValue;
      m_values = 0;
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
      GridMap (owner, childIndex), m_grid ((Grid *) operand1->getGrid ()),
      m_undefinedValue (operand1->getUndefinedValue ()), m_averageValue (operand1->getUndefinedValue ()),
      m_depth (operand1->getDepth ())
{
   unsigned int numI = m_grid->numI ();
   unsigned int numJ = m_grid->numJ ();

   assert (numI == operand2->getGrid ()->numI ());
   assert (numJ == operand2->getGrid ()->numJ ());

   assert (m_depth == operand2->getDepth ());

   if (operand1->isConstant () && operand2->isConstant ())
   {
      m_singleValue = binaryFunctor (operand1->getConstantValue (), operand2->getConstantValue ());
      m_averageValue = m_singleValue;
      m_values = 0;
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
      GridMap (owner, childIndex), m_grid ((Grid *) operand->getGrid ()),
      m_undefinedValue (operand->getUndefinedValue ()), m_averageValue (operand->getUndefinedValue ()),
      m_depth (operand->getDepth ())
{
   unsigned int numI = m_grid->numI ();
   unsigned int numJ = m_grid->numJ ();

   if (operand->isConstant ())
   {
      m_singleValue = unaryFunctor (operand->getConstantValue ());
      m_averageValue = m_singleValue;
      m_values = 0;
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




SerialGridMap::~SerialGridMap (void)
{
   if (m_values) Array<double>::delete3d (m_values);
}

bool SerialGridMap::retrieveGhostedData (void) const
{
   return true;
}

bool SerialGridMap::retrieveData (bool withGhosts) const
{
   return true;
}

bool SerialGridMap::restoreData (bool save, bool withGhosts) const
{
   return true;
}

#if 0
// Added by V.R. Ambati (13/07/2011):
// getVec(...) is defined only for Distrubuted version and therefore, serial version
// simply throws an error
Vec & GridMap::getVec (void)
{
   throw "Vec & GridMap::getVec (void) is not defined for serial version.";
}

// Added by V.R. Ambati (13/07/2011):
// getDA(...) is defined only for Distrubuted version and therefore, serial version
// simply throws an error
DA & GridMap::getDA (void) const
{
   throw "DA & GridMap::getDA (void) const is not defined for serial version.";
}
#endif

double SerialGridMap::getUndefinedValue (void) const
{
   return m_undefinedValue;
}

/// Check if GridMap is constant
bool SerialGridMap::isConstant (void) const
{
   return m_values == 0 && m_singleValue != m_undefinedValue;
}

/// Return the constant value. returns the undefined value if not constant
double SerialGridMap::getConstantValue (void) const
{
   return m_singleValue;
}

const Grid * SerialGridMap::getGrid (void) const
{
   return m_grid;
}

double SerialGridMap::getValue (unsigned int i, unsigned int j) const
{
    return getValue(i, j, (unsigned int) 0);
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

double SerialGridMap::getValue (unsigned int i, unsigned int j, double k) const
{
   return getValue ( (double) i, (double) j, k);
}

double SerialGridMap::getValue (double i, double j, double k) const
{
   const double MinOffset = 1e-6;

   unsigned int baseI = (unsigned int) i;
   unsigned int baseJ = (unsigned int) j;
   unsigned int baseK = (unsigned int) k;

   double fractionI = i - (double) baseI;
   double fractionJ = j - (double) baseJ;
   double fractionK = k - (double) baseK;

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

   double value = 
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

double SerialGridMap::getAverageValue (void) const
{
   unsigned int numValues = 0;
   double total = 0;

   if (m_averageValue != getUndefinedValue ())
   {
      return m_averageValue;
   }

   for (unsigned int i = 0; i < m_grid->numI (); i++)
   {
      for (unsigned int j = 0; j < m_grid->numJ (); j++)
      {
	 for (unsigned int k = 0; k < m_depth; k++)
	 {
	    double value = getValue (i, j, k);

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
   max = std::numeric_limits< double >::min(); 

   for (unsigned int i = 0; i < m_grid->numI (); i++)
   {
      for (unsigned int j = 0; j < m_grid->numJ (); j++)
      {
	 for (unsigned int k = 0; k < m_depth; k++)
	 {
	    double value = getValue (i, j, k);

	    if (value != getUndefinedValue ())
	    {
	       max = Max (value, max);
	       min = Min (value, min);
	    }
	 }
      }
   }
}

unsigned int SerialGridMap::getDepth (void) const
{
   return m_depth;
}

void SerialGridMap::release (void) const
{
   Child::release ();
}

double const * const * const * SerialGridMap::getValues (void) const
{
   if (!m_values)
   {
      m_values = Array < double >::create3d (m_grid->numI (), m_grid->numJ (), m_depth, m_singleValue);
   }

   return m_values;
}

bool SerialGridMap::saveHDF5 (const string & fileName) const
{
   hid_t fileHandle = H5Fcreate (fileName.c_str (), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

   if (fileHandle < 0)
      return false;

   int version = 0;
   HDF5::writeData1D (fileHandle, 1, "/gioHDFfile version", H5T_NATIVE_INT, &version);

   int numI = getGrid ()->numI ();
   int numJ = getGrid ()->numJ ();
   HDF5::writeData1D (fileHandle, 1, "/number in I dimension", H5T_NATIVE_INT, &numI);
   HDF5::writeData1D (fileHandle, 1, "/number in J dimension", H5T_NATIVE_INT, &numJ);

   float origI = getGrid ()->minI ();
   float origJ = getGrid ()->minJ ();
   HDF5::writeData1D (fileHandle, 1, "/origin in I dimension", H5T_NATIVE_FLOAT, &origI);
   HDF5::writeData1D (fileHandle, 1, "/origin in J dimension", H5T_NATIVE_FLOAT, &origJ);

   float deltaI = getGrid ()->deltaI ();
   float deltaJ = getGrid ()->deltaJ ();
   HDF5::writeData1D (fileHandle, 1, "/delta in I dimension", H5T_NATIVE_FLOAT, &deltaI);
   HDF5::writeData1D (fileHandle, 1, "/delta in J dimension", H5T_NATIVE_FLOAT, &deltaJ);

   float nullValue = (float) getUndefinedValue ();
   HDF5::writeData1D (fileHandle, 1, "/null value", H5T_NATIVE_FLOAT, &nullValue);

   int maxKsize = 1;
   HDF5::writeData1D (fileHandle, 1, "maxKsize", H5T_NATIVE_INT, &maxKsize);

   float * dataArray = new float[getGrid ()->numI () * getGrid ()->numJ ()];

   for (unsigned int i = 0; i < numI; ++i)
   {
      for (unsigned int j = 0; j < numJ; ++j)
      {
	 dataArray[i * numJ + j] = getValue (i, j, (unsigned int) 0); 
      }
   }

   HDF5::writeData2D (fileHandle, numI, numJ, "/Layer=0", H5T_NATIVE_FLOAT, dataArray);

   HDF5::writeAttribute (fileHandle, "/Layer=0", "PropertyName", H5T_C_S1, 3, (void*)"RD0");
   HDF5::writeAttribute (fileHandle, "/Layer=0", "GridName", H5T_C_S1, 5, (void*)"RD0_1");
   HDF5::writeAttribute (fileHandle, "/Layer=0", "CLASS", H5T_C_S1, 5, (void*)"IMAGE");

   float age = 0;
   HDF5::writeAttribute (fileHandle, "/Layer=0", "StratTopAge", H5T_NATIVE_FLOAT, 1, &age);
   HDF5::writeData1D (fileHandle, 1, "/StratTopAge", H5T_NATIVE_FLOAT, &age);

   HDF5::writeAttribute (fileHandle, "/Layer=0", "StratTopProp", H5T_C_S1, 3, (void*)"DEP");
   HDF5::writeAttribute (fileHandle, "/Layer=0", "StratTopName", H5T_C_S1, 12, (void*)"Water bottom");

   H5Fclose (fileHandle);
   return true;
}

void SerialGridMap::printOn (ostream & ostr) const
{
   unsigned int depth = getDepth ();
   unsigned int numI = getGrid ()->numI ();
   unsigned int numJ = getGrid ()->numJ ();

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
	    if (j != 0)
	       ostr << ", ";
	    double value = getValue (i, j, k);

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
   bool ret = false;

   const Grid *gridA = (Grid *) this->getGrid();
   const Grid *gridB = (Grid *) mapB->getGrid();
   
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
   const Grid *gridB = (Grid *) mapB->getGrid ();

   bool ret = true;

   unsigned int indexImapA, indexJmapA;
   unsigned int indexImapB, indexJmapB;

   unsigned int k;

   indexImapA = indexJmapA = indexImapB = indexJmapB = 0;

   unsigned int depthA = this->getDepth ();

   const Grid *highResGridA = (Grid *) this->getGrid();
   
   for (indexImapB = 0; indexImapB < gridB->numI (); ++indexImapB)
   {
      for (indexJmapB = 0; indexJmapB < gridB->numJ (); ++indexJmapB)
      {
	 if (gridB->convertToGrid ( (*highResGridA), indexImapB, indexJmapB, indexImapA, indexJmapA) )
	 {
	    for (k = 0; k < depthA; k++)
	    {
               mapB->setValue (indexImapB, indexJmapB, k, this->getValue (indexImapA, indexJmapA, k));
	       
#if 0
	       cerr << ddd::GetRankString () << ": converting value " << this->getValue (indexImapA, indexJmapA, k) <<
		  " from lowres (" << indexImapB << ", " << indexJmapB << ", " << k <<
		  ") to highres (" << indexImapA << ", "  << indexJmapA << ", " << k << ")" << endl;
#endif
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

bool SerialGridMap::transformLowRes2HighRes(GridMap *mapB) const
{
   const GridMap *mapA = this;
   const Grid *gridA = (Grid *) mapA->getGrid ();
   const Grid *gridB = (Grid *) mapB->getGrid ();

   bool ret = true;


   unsigned int highResI, highResJ, k;
   unsigned int depthB = mapB->getDepth ();

   for (highResI = 0; highResI < gridB->numI (); ++highResI)
   {
      for (highResJ = 0; highResJ < gridB->numJ (); ++highResJ)
      {
         double doubleLowResI, doubleLowResJ;

         if (!gridB->convertToGrid (* gridA, highResI, highResJ, doubleLowResI, doubleLowResJ))
         {
	    // one of the four surrounding lowres grid points is outside the highres grid
#if 0
	    cerr << "mapping of (" << highResI << ", " << highResJ << ") to " <<
	       "(" << doubleLowResI << ", " << doubleLowResJ << ") is out of bounds " << endl;
#endif

            for (k = 0; k < depthB; k++)
	    {
               mapB->setValue(highResI, highResJ, k, DefaultUndefinedMapValue); 
	    }
	    continue;
         }

#if 0
	    cerr << "mapping of (" << highResI << ", " << highResJ << ") to " <<
	       "(" << doubleLowResI << ", " << doubleLowResJ << ") is within bounds " << endl;
#endif
         unsigned int intLowResI = (int) doubleLowResI;
         unsigned int intLowResJ = (int) doubleLowResJ;

	 const double errorMargin = 1e-4;

         double fractionI = doubleLowResI - intLowResI;
	 if (fractionI < errorMargin) fractionI = 0;
	 if (fractionI > 1 - errorMargin) fractionI = 1;

         double fractionJ = doubleLowResJ - intLowResJ;
	 if (fractionJ < errorMargin) fractionJ = 0;
	 if (fractionJ > 1 - errorMargin) fractionJ = 1;

#if 0
	 cerr << "Fractions (" << highResI << ", " << highResJ << ") = " << "(" << fractionI << ", " << fractionJ << ")" << endl;
#endif

         for (k = 0; k < depthB; k++)
         {
            double lowResMapValues[2][2];

            lowResMapValues[0][0] = mapA->getValue (intLowResI, intLowResJ, k);
            lowResMapValues[0][1] = mapA->getValue (intLowResI, intLowResJ + 1, k);
            lowResMapValues[1][0] = mapA->getValue (intLowResI + 1, intLowResJ, k);
            lowResMapValues[1][1] = mapA->getValue (intLowResI + 1, intLowResJ + 1, k);

            double highResMapValue = 0;

	    if ((lowResMapValues[0][0] == mapA->getUndefinedValue () && fractionI != 1 && fractionJ != 1) ||
		  (lowResMapValues[0][1] == mapA->getUndefinedValue () && fractionI != 1 && fractionJ != 0) ||
		  (lowResMapValues[1][0] == mapA->getUndefinedValue () && fractionI != 0 && fractionJ != 1) ||
		  (lowResMapValues[1][1] == mapA->getUndefinedValue () && fractionI != 0 && fractionJ != 0))
	    {
#if 0
	       cerr << ddd::GetRank () << ": " << endl;
	       cerr << "highResMapValue (" << highResI << ", " << highResJ << ") = undefined" << endl;
	       cerr << "fractionI = " << fractionI << ", fractionJ = " << fractionJ << endl;
	       cerr << "intLowResI = " << intLowResI << ", intLowResJ = " << intLowResJ << endl;
	       cerr << "doubleLowResI = " << doubleLowResI << ", doubleLowResJ = " << doubleLowResJ << endl;
	       cerr << "lowResMapValues[0][0] = " << lowResMapValues[0][0] << endl;
	       cerr << "lowResMapValues[0][1] = " << lowResMapValues[0][1] << endl;
	       cerr << "lowResMapValues[1][0] = " << lowResMapValues[1][0] << endl;
	       cerr << "lowResMapValues[1][1] = " << lowResMapValues[1][1] << endl;
#endif
	       highResMapValue = mapB->getUndefinedValue ();

	    }
	    else
	    {
	       highResMapValue += lowResMapValues[0][0] * (1 - fractionI) * (1 - fractionJ);
	       highResMapValue += lowResMapValues[0][1] * (1 - fractionI) * (fractionJ);
	       highResMapValue += lowResMapValues[1][0] * (fractionI) * (1 - fractionJ);
	       highResMapValue += lowResMapValues[1][1] * (fractionI) * (fractionJ);
	    }

            //set the value
            mapB->setValue (highResI, highResJ, k, highResMapValue);
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
