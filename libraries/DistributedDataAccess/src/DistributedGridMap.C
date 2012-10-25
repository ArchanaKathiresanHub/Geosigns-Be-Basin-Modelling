#include <math.h>
#include <values.h>
#include <assert.h>

#include <iomanip>

#include "petsc.h"

#if 0
#include "Interface/ProjectHandle.h"
#endif

#include "Interface/DistributedGridMap.h"

#include "Interface/DistributedGrid.h"


#include "array.h"

using namespace DataAccess;
using namespace Interface;

#define Min(a,b)        (a < b ? a : b)
#define Max(a,b)        (a > b ? a : b)

DistributedGridMap::~DistributedGridMap (void)
{
   restoreData (false, false);

   VecDestroy (m_vecGlobal);

   if (m_depth != 1)
   {
      DADestroy (m_localInfo.da);
   }
}

/// Create a GridMap from the given grid with the given value to be used for undefined values.
DistributedGridMap::DistributedGridMap (const Grid * grid, unsigned int depth, double undefinedValue) :
       GridMap (0, 0),
      m_grid (grid), m_undefinedValue (undefinedValue), m_averageValue (m_undefinedValue), m_depth (depth), m_retrieved (false)
{
   initialize ();
   PetscScalar initialValue = m_undefinedValue;
   VecSet (m_vecGlobal, initialValue);
}


/// Create a constant GridMap from the given value
DistributedGridMap::DistributedGridMap (const Parent * owner, unsigned int childIndex, const Grid * grid, double value, unsigned int depth) :
      GridMap (owner, childIndex), m_undefinedValue (DefaultUndefinedMapValue), m_averageValue (m_undefinedValue), m_depth (depth), m_grid (grid), m_retrieved (false)
{
   initialize ();
   PetscScalar initialValue = value;
   VecSet (m_vecGlobal, initialValue);
}

DistributedGridMap::DistributedGridMap (const Parent * owner, unsigned int childIndex, const Grid * grid,
      double undefinedValue, unsigned int depth, float *** values) :
      GridMap (owner, childIndex),
      m_grid (grid), m_undefinedValue (undefinedValue), m_averageValue (m_undefinedValue), m_depth (depth), m_retrieved (false)
{
   initialize ();

   retrieveData ();

   for (unsigned int i = firstI (); i <= lastI (); ++i)
   {
      for (unsigned int j = firstJ (); j <= lastJ (); ++j)
      {
         for (unsigned int k = 0; k < getDepth (); ++k)
         {
            {
               setValue (i, j, k, values[i][j][k]);
            }
         }
      }
   }
   restoreData (true);
}

/// Create a GridMap from the two given GridMap objects after elementwise processing by the specified operator function.
DistributedGridMap::DistributedGridMap (const Parent * owner, unsigned int childIndex,
      const GridMap * operand1, const GridMap * operand2, BinaryOperator binaryOperator):
      GridMap (owner, childIndex), m_grid ((DistributedGrid *) operand1->getGrid ()),
      m_undefinedValue (operand1->getUndefinedValue ()), m_averageValue (m_undefinedValue), m_depth (operand1->getDepth ()), m_retrieved (false)
{
   initialize ();

   retrieveData ();

   operand1->retrieveData ();
   operand2->retrieveData ();

   assert (numI () == operand1->numI ());
   assert (numJ () == operand1->numJ ());

   assert (getDepth () == operand1->getDepth ());

   assert (numI () == operand2->numI ());
   assert (numJ () == operand2->numJ ());

   assert (getDepth () == operand2->getDepth ());

   for (unsigned int i = firstI (); i <= lastI (); ++i)
   {
      for (unsigned int j = firstJ (); j <= lastJ (); ++j)
      {
         for (unsigned int k = 0; k < getDepth (); ++k)
         {
            if (!operand1->valueIsDefined (i, j, k) || !operand2->valueIsDefined (i, j, k))
            {
               setValue (i, j, k, m_undefinedValue);
            }
            else
            {
               setValue (i, j, k, (* binaryOperator) (operand1->getValue (i, j, k), operand2->getValue (i, j, k)));
            }
         }
      }
   }

   restoreData (true);
   operand1->restoreData (false);
   operand2->restoreData (false);
}

/// Create a GridMap from the two given GridMap objects after elementwise processing by the specified binary functor.
DistributedGridMap::DistributedGridMap (const Parent * owner, unsigned int childIndex,
      const GridMap * operand1, const GridMap * operand2, BinaryFunctor& binaryFunctor) : 
	GridMap (owner, childIndex), m_grid ((DistributedGrid *) operand1->getGrid ()), m_undefinedValue (operand1->getUndefinedValue ()), 
	m_averageValue (m_undefinedValue), m_depth (operand1->getDepth ()), m_retrieved (false)
{
   initialize ();

   retrieveData ();

   operand1->retrieveData ();
   operand2->retrieveData ();

   assert (numI () == operand1->numI ());
   assert (numJ () == operand1->numJ ());

   assert (getDepth () == operand1->getDepth ());

   assert (numI () == operand2->numI ());
   assert (numJ () == operand2->numJ ());

   assert (getDepth () == operand2->getDepth ());

   for (unsigned int i = firstI (); i <= lastI (); ++i)
   {
      for (unsigned int j = firstJ (); j <= lastJ (); ++j)
      {
         for (unsigned int k = 0; k < getDepth (); ++k)
         {
            if (!operand1->valueIsDefined (i, j, k) || !operand2->valueIsDefined (i, j, k))
            {
               setValue (i, j, k, m_undefinedValue);
            }
            else
            {
               setValue (i, j, k, binaryFunctor (operand1->getValue (i, j, k), operand2->getValue (i, j, k)));
            }
         }
      }
   }

   restoreData (true);
   operand1->restoreData (false);
   operand2->restoreData (false);
}

/// Create a GridMap from the given GridMap after elementwise processing by the specified operator function.
DistributedGridMap::DistributedGridMap (const Parent * owner, unsigned int childIndex, const GridMap * operand, UnaryOperator unaryOperator) :
      GridMap (owner, childIndex), m_grid ((DistributedGrid *) operand->getGrid ()),
      m_undefinedValue (operand->getUndefinedValue ()), m_averageValue (m_undefinedValue), m_depth (operand->getDepth ()), m_retrieved (false)
{
   initialize ();

   retrieveData ();

   operand->retrieveData ();

   assert (numI () == operand->numI ());
   assert (numJ () == operand->numJ ());

   assert (getDepth () == operand->getDepth ());

   for (unsigned int i = firstI (); i <= lastI (); ++i)
   {
      for (unsigned int j = firstJ (); j <= lastJ (); ++j)
      {
         for (unsigned int k = 0; k < getDepth (); ++k)
         {
            if (!operand->valueIsDefined (i, j, k))
            {
               setValue (i, j, k, m_undefinedValue);
            }
            else
            {
               setValue (i, j, k, (*unaryOperator) (operand->getValue (i, j, k)));
            }
         }
      }
   }

   restoreData (true);
   operand->restoreData (false);
}

/// Create a GridMap from the given GridMap after elementwise processing by the specified operator function.
DistributedGridMap::DistributedGridMap (const Parent * owner, unsigned int childIndex, const GridMap * operand, UnaryFunctor& unaryFunctor) :
      GridMap (owner, childIndex), m_grid ((DistributedGrid *) operand->getGrid ()),
      m_undefinedValue (operand->getUndefinedValue ()), m_averageValue (m_undefinedValue), m_depth (operand->getDepth ()), m_retrieved (false)
{
   initialize ();

   retrieveData ();

   operand->retrieveData ();

   assert (numI () == operand->numI ());
   assert (numJ () == operand->numJ ());

   assert (getDepth () == operand->getDepth ());

   for (unsigned int i = firstI (); i <= lastI (); ++i)
   {
      for (unsigned int j = firstJ (); j <= lastJ (); ++j)
      {
         for (unsigned int k = 0; k < getDepth (); ++k)
         {
            if (!operand->valueIsDefined (i, j, k))
            {
               setValue (i, j, k, m_undefinedValue);
            }
            else
            {
               setValue (i, j, k, unaryFunctor (operand->getValue (i, j, k)));
            }
         }
      }
   }

   restoreData (true);
   operand->restoreData (false);
}

void DistributedGridMap::initialize (void)
{
   if (m_depth == 1)
   {
      m_localInfo = dynamic_cast<const DistributedGrid * > (m_grid)->getLocalInfo ();
      VecDuplicate (dynamic_cast<const DistributedGrid * > (m_grid)->getVec (), &m_vecGlobal);
   }
   else
   {
      DACreate3d (PETSC_COMM_WORLD, DA_NONPERIODIC, DA_STENCIL_BOX,
	      dynamic_cast<const DistributedGrid * > (m_grid)->numIGlobal (), dynamic_cast<const DistributedGrid * > (m_grid)->numJGlobal (), m_depth,
	      dynamic_cast<const DistributedGrid * > (m_grid)->numProcsI (), dynamic_cast<const DistributedGrid * > (m_grid)->numProcsJ (), 1,
	      1, 1,
	      dynamic_cast<const DistributedGrid * > (m_grid)->numsI (), dynamic_cast<const DistributedGrid * > (m_grid)->numsJ (), PETSC_NULL,
	      &m_localInfo.da);
      DAGetLocalInfo (m_localInfo.da, &m_localInfo);
      DACreateGlobalVector (m_localInfo.da, &m_vecGlobal);
   }


   m_modified = false;
}

bool DistributedGridMap::retrieveGhostedData (void) const
{
   return retrieveData (true);
}

bool DistributedGridMap::retrieveData (bool withGhosts) const
{
   if (m_retrieved) return false;

   m_withGhosts = withGhosts;

   if (m_withGhosts)
   {
      DAGetLocalVector (m_localInfo.da, &m_vecLocal);
      DAGlobalToLocalBegin (m_localInfo.da, m_vecGlobal, INSERT_VALUES, m_vecLocal);
      DAGlobalToLocalEnd (m_localInfo.da, m_vecGlobal, INSERT_VALUES, m_vecLocal);
   }

   if (m_depth > 1)
   {
      DAVecGetArray (m_localInfo.da, m_withGhosts ? m_vecLocal : m_vecGlobal, (void *) &m_values);
   }
   else
   {
      m_values = Array < double **>::create1d (1);

      DAVecGetArray (m_localInfo.da, m_withGhosts ? m_vecLocal : m_vecGlobal, (void *) &m_values[0]);
   }

   m_retrieved = true;

   m_modified = false;

   return true;
}

bool DistributedGridMap::restoreData (bool save, bool withGhosts) const
{
   if (!m_retrieved)
      return false;

   if (m_depth > 1)
   {
      DAVecRestoreArray (m_localInfo.da, m_withGhosts ? m_vecLocal : m_vecGlobal, (void *) &m_values);
   }
   else
   {
      DAVecRestoreArray (m_localInfo.da, m_withGhosts ? m_vecLocal : m_vecGlobal, (void *) &m_values[0]);
      Array < double **>::delete1d (m_values);
   }

   if (save && m_modified)
   {
      if (m_withGhosts)
      {
         if (withGhosts)
         {
            DALocalToGlobalBegin (m_localInfo.da, m_vecLocal, m_vecGlobal);
            DALocalToGlobalEnd (m_localInfo.da, m_vecLocal, m_vecGlobal);
         }
         else
         {
            DALocalToGlobal (m_localInfo.da, m_vecLocal, INSERT_VALUES, m_vecGlobal);
         }
      }
   }

   if (m_withGhosts)
   {
      DARestoreLocalVector (m_localInfo.da, &m_vecLocal);
   }

   m_retrieved = false;
   m_modified = false;

   return true;
}

Vec & DistributedGridMap::getVec (void)
{
   return m_vecGlobal;
}

DA & DistributedGridMap::getDA (void) const
{
   return m_localInfo.da;
}

double DistributedGridMap::minI (void) const
{
   return m_grid->minIGlobal () + firstI () * deltaI ();
}

double DistributedGridMap::minJ (void) const
{
   return m_grid->minJGlobal () + firstJ () * deltaJ ();
}

double DistributedGridMap::maxI (void) const
{
   return m_grid->minIGlobal () + lastI () * deltaI ();
}

double DistributedGridMap::maxJ (void) const
{
   return m_grid->minJGlobal () + lastJ () * deltaJ ();
}

unsigned int DistributedGridMap::numI (void) const
{
   assert (m_retrieved);
   return m_withGhosts ? m_localInfo.gxm : m_localInfo.xm;
}

unsigned int DistributedGridMap::numJ (void) const
{
   assert (m_retrieved);
   return m_withGhosts ? m_localInfo.gym : m_localInfo.ym;
}

#if 0
unsigned int DistributedGridMap::firstI (void) const
{
   assert (m_retrieved);
   return m_withGhosts ? m_localInfo.gxs : m_localInfo.xs;
}
unsigned int DistributedGridMap::firstJ (void) const
{
   assert (m_retrieved);
   return m_withGhosts ? m_localInfo.gys : m_localInfo.ys;
}
#endif

#if 0
unsigned int DistributedGridMap::firstK (void) const
{
   return 0;
}

unsigned int DistributedGridMap::lastI (void) const
{
   assert (m_retrieved);
   return m_withGhosts ? (m_localInfo.gxs + m_localInfo.gxm - 1) : (m_localInfo.xs + m_localInfo.xm - 1);
}

unsigned int DistributedGridMap::lastJ (void) const
{
   assert (m_retrieved);
   return m_withGhosts ? (m_localInfo.gys + m_localInfo.gym - 1) : (m_localInfo.ys + m_localInfo.ym - 1);
}

unsigned int DistributedGridMap::lastK (void) const
{
   return getDepth () - 1;
}
#endif

double DistributedGridMap::minI (bool withGhosts) const
{
   return m_grid->minIGlobal () + firstI (withGhosts) * deltaI ();
}

double DistributedGridMap::minJ (bool withGhosts) const
{
   return m_grid->minJGlobal () + firstJ (withGhosts) * deltaJ ();
}

double DistributedGridMap::maxI (bool withGhosts) const
{
   return m_grid->minIGlobal () + lastI (withGhosts) * deltaI ();
}

double DistributedGridMap::maxJ (bool withGhosts) const
{
   return m_grid->minJGlobal () + lastJ (withGhosts) * deltaJ ();
}

unsigned int DistributedGridMap::numI (bool withGhosts) const
{
   return withGhosts ? m_localInfo.gxm : m_localInfo.xm;
}

unsigned int DistributedGridMap::numJ (bool withGhosts) const
{
   return withGhosts ? m_localInfo.gym : m_localInfo.ym;
}

unsigned int DistributedGridMap::firstI (bool withGhosts) const
{
   return withGhosts ? m_localInfo.gxs : m_localInfo.xs;
}

unsigned int DistributedGridMap::firstJ (bool withGhosts) const
{
   return withGhosts ? m_localInfo.gys : m_localInfo.ys;
}

unsigned int DistributedGridMap::lastI (bool withGhosts) const
{
   return withGhosts ? (m_localInfo.gxs + m_localInfo.gxm - 1) : (m_localInfo.xs + m_localInfo.xm - 1);
}

unsigned int DistributedGridMap::lastJ (bool withGhosts) const
{
   return withGhosts ? (m_localInfo.gys + m_localInfo.gym - 1) : (m_localInfo.ys + m_localInfo.ym - 1);
}

double DistributedGridMap::deltaI (void) const
{
   return m_grid->deltaI ();
}

double DistributedGridMap::deltaJ (void) const
{
   return m_grid->deltaJ ();
}

bool DistributedGridMap::isGridPoint (unsigned int i,unsigned int j, unsigned int k) const
{
   if (firstI (m_withGhosts) <= i && i <= lastI (m_withGhosts) &&
           firstJ (m_withGhosts) <= j && j <= lastJ (m_withGhosts) &&
	   k < m_depth)
   {
      return true;
   }
   else
   {
#if 0
      cerr << ddd::GetRankString () << ": grid point (" << i << ", " << j << ", " << k
	 << ", " << (m_withGhosts ? "true" : "false") << ") is out of bounds" << endl;
      cerr << "\tfirstI (true) = " << firstI (true) << endl;
      cerr << "\tfirstI (false) = " << firstI (false) << endl;
      cerr << "\tlastI (true) = " << lastI (true) << endl;
      cerr << "\tlastI (false) = " << lastI (false) << endl;
#endif
      return false;
   }

}

double DistributedGridMap::getUndefinedValue (void) const
{
   return m_undefinedValue;
}

/// Check if DistributedGridMap is constant
bool DistributedGridMap::isConstant (void) const
{
   return getConstantValue () != getUndefinedValue ();
}

/// Return the constant value. returns the undefined value if not constant
double DistributedGridMap::getConstantValue (void) const
{
   double min, max;
   getMinMaxValue (min, max);
   if (min == max)
      return min;
   else
      return getUndefinedValue ();
}

const Grid * DistributedGridMap::getGrid (void) const
{
   return m_grid;
}

double DistributedGridMap::getValue (unsigned int i, unsigned int j) const
{
   return getValue (i, j, (unsigned int) 0);
}

double DistributedGridMap::getValue (unsigned int i, unsigned int j, unsigned int k) const
{
   if (isGridPoint (i, j, k) && m_retrieved)
   {
      return m_values[k][j][i];
   }
   else
      return m_undefinedValue;
}

double DistributedGridMap::getValue (unsigned int i, unsigned int j, double k) const
{
   if (k < 0) return getUndefinedValue ();

   unsigned int kLow = (unsigned int) k;
   double fraction = k - kLow;

   if (fraction == 0)
   {
      return getValue (i, j, kLow);
   }

   unsigned int kHigh = kLow + 1;

   double valueLow = getValue (i, j, kLow);
   double valueHigh = getValue (i, j, kHigh);

   if (valueLow == getUndefinedValue () || valueHigh == getUndefinedValue ())
   {
      return getUndefinedValue ();
   }
   else
   {
      return valueLow + fraction * (valueHigh - valueLow);
   }
}

double DistributedGridMap::getValue (double i, double j, double k) const
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

double DistributedGridMap::getFractionalValue (double fraction, unsigned int i, unsigned int j, unsigned int k) const
{
   if (fraction > 0) return fraction * getValue (i, j, k);
   else return 0;
}

double DistributedGridMap::getAverageValue () const
{
   unsigned int numValues = 0;
   double total = 0;

   if (m_averageValue != getUndefinedValue ())
   {
      return m_averageValue;
   }

   for (unsigned int i = firstI (); i <= lastI (); ++i)
   {
      for (unsigned int j = firstJ (); j <= lastJ (); ++j)
      {
	 for (unsigned int k = 0; k < m_depth; ++k)
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

   double allTotal;
   int allNumValues;

   MPI_Allreduce (&total, &allTotal, 1, MPI_DOUBLE, MPI_SUM, PETSC_COMM_WORLD);
   MPI_Allreduce (&numValues, &allNumValues, 1, MPI_INT, MPI_SUM, PETSC_COMM_WORLD);

   if (allNumValues > 0)
      m_averageValue = allTotal / allNumValues;
   else
      m_averageValue = getUndefinedValue ();

   return m_averageValue;
}

/// return the minimum & maximum value calculated over all GridPoints with a defined value
void DistributedGridMap::getMinMaxValue (double & min, double & max) const
{
   double minLocal = MAXDOUBLE;
   double maxLocal = -MAXDOUBLE;
   double minGlobal;
   double maxGlobal;

   for (unsigned int i = firstI (); i <= lastI (); ++i)
   {
      for (unsigned int j = firstJ (); j <= lastJ (); ++j)
      {
	 for (unsigned int k = 0; k < m_depth; k++)
	 {
	    double value = getValue (i, j, k);

	    if (value != getUndefinedValue ())
	    {
	       maxLocal = Max (value, maxLocal);
	       minLocal = Min (value, minLocal);
	    }
	 }
      }
   }

   MPI_Allreduce (&minLocal, &minGlobal, 1, MPI_DOUBLE, MPI_MIN, PETSC_COMM_WORLD);
   MPI_Allreduce (&maxLocal, &maxGlobal, 1, MPI_DOUBLE, MPI_MAX, PETSC_COMM_WORLD);

   min = minGlobal;
   max = maxGlobal;
}

bool DistributedGridMap::valueIsDefined (unsigned int i, unsigned int j, unsigned int k) const
{
   return getValue (i, j, k) != m_undefinedValue;
}

bool DistributedGridMap::setValue (unsigned int i, unsigned int j, double value)
{
   return setValue (i, j, 0, value);
}

bool DistributedGridMap::setValue (unsigned int i, unsigned int j, unsigned int k, double value)
{
   if (isGridPoint (i, j, k) && m_retrieved)
   {
      m_values[k][j][i] = value;

      m_modified = true;
      return true;
   }
   else
   {
      return false;
   }
}

unsigned int DistributedGridMap::getDepth (void) const
{
   return m_depth;
}

void DistributedGridMap::release (void) const
{
   Child::release ();
}

double const * const * const * DistributedGridMap::getValues (void) const
{
   return m_values;
}

// Added by V.R. Ambati (13/07/2011):
// saveHDF5 (.....) is an abstract function and its implementation is missing
// for the distributed version. This implementation is now added which simply
// throws an error when called.
bool DistributedGridMap::saveHDF5 (const string & fileName) const
{
	throw "bool GridMap::saveHDF5 (const string & fileName) const is not implemented in the distributed version.";
}

void DistributedGridMap::printOn (ostream & ostr) const
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
	    double value = getValue (i, j, k);

	    ostr << std::setw ( 16 ) << value;
	    if (j != numJ - 1)
	       ostr << ",";

	 }
	 ostr << endl;
      }
      ostr << endl;
   }
}

void DistributedGridMap::printOn (MPI_Comm comm) const
{
   assert (m_retrieved);
   unsigned int i, j, k;

   const Grid *grid = (Grid *) getGrid ();

   for (k = 0; k < m_depth; k++)
   {
      PetscSynchronizedPrintf (comm, "(%d)\t", k);

      for (i = 0; i < grid->numIGlobal (); i++)
      {
         PetscSynchronizedPrintf (comm, "       %d\t", i);
      }
      PetscSynchronizedPrintf (comm, "\n");

      for (j = 0; j < grid->numJGlobal (); j++)
      {
         PetscSynchronizedPrintf (comm, "%d\t", j);

         for (i = 0; i < grid->numIGlobal (); i++)
         {
            if (i >= firstI () && i <= lastI () &&
                j >= firstJ () && j <= lastJ ())
            {
               PetscSynchronizedPrintf (comm, "%8.2lf\t", getValue (i, j));
            }
            else
            {
               PetscSynchronizedPrintf (comm, " xxxxxxx\t");
            }
         }
         PetscSynchronizedPrintf (comm, "\n");
      }
      PetscSynchronizedPrintf (comm, "\n");
   }
   PetscSynchronizedPrintf (comm, "\n");

   PetscSynchronizedFlush (comm);

}
///map interpolation functionality
bool DistributedGridMap::convertToGridMap(GridMap *mapB) const
{
   bool ret = false;

   const Grid *GridA = (Grid *) this->getGrid();
   const Grid *GridB = (Grid *) mapB->getGrid();
   
   if( GridA->numI() >= GridB->numI()  && GridA->numJ() >= GridB->numJ() )
   {
      ret = transformHighRes2LowRes(mapB);
   }
   else       	
   {
      ret = transformLowRes2HighRes(mapB);
   }

   return ret;	
}

bool DistributedGridMap::transformHighRes2LowRes(GridMap *mapB) const 
{
   const Grid *GridB = (Grid *) mapB->getGrid ();

   bool ret = true;

   unsigned int indexImapA, indexJmapA;
   unsigned int indexImapB, indexJmapB;

   unsigned int k;

   indexImapA = indexJmapA = indexImapB = indexJmapB = 0;

   this->retrieveData ();
   mapB->retrieveData ();

   unsigned int depthA = this->getDepth ();

   const Grid *highResGridA = (Grid *) this->getGrid();
   
   for (indexImapB = mapB->firstI (); indexImapB <= mapB->lastI (); ++indexImapB)
   {
      for (indexJmapB = mapB->firstJ (); indexJmapB <= mapB->lastJ (); ++indexJmapB)
      {
	 if (GridB->convertToGrid ( (*highResGridA), indexImapB, indexJmapB, indexImapA, indexJmapA) )
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

   this->restoreData ();
   mapB->restoreData ();

   return ret;
}

bool DistributedGridMap::transformLowRes2HighRes(GridMap *mapB) const
{
   const GridMap *mapA = this;
   const DistributedGrid *GridA = (DistributedGrid *) mapA->getGrid ();
   const DistributedGrid *GridB = (DistributedGrid *) mapB->getGrid ();

   bool ret = true;

   bool useGhostNodes = true;

   const GlobalGrid & globalGridA = GridA->getGlobalGrid ();
   const GlobalGrid & globalGridB = GridB->getGlobalGrid ();

   mapA->retrieveData (useGhostNodes);
   mapB->retrieveData ();

   unsigned int highResI, highResJ, k;
   unsigned int depthB = mapB->getDepth ();

   for (highResI = mapB->firstI (); highResI <= mapB->lastI (); ++highResI)
   {
      for (highResJ = mapB->firstJ (); highResJ <= mapB->lastJ (); ++highResJ)
      {
         double doubleLowResI, doubleLowResJ;

         if (!globalGridB.convertToGrid (globalGridA, highResI, highResJ, doubleLowResI, doubleLowResJ))
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
bool DistributedGridMap::findLowResElementCoordinates(const unsigned int HighResI, const unsigned int HighResJ, const unsigned int depth, 
                                		 const DistributedGridMap *lowResMap,  const DistributedGrid *lowResGrid,  
                                		 const DistributedGrid *HighResGrid, 
                                		 unsigned int lowResElementCoordinatesInHighRes[],
                                		 double nodalValuesInLowResElement[], bool useGhostNodesInLowRes)
{
   bool ret = false;

   //check first in the cache
   if(isHighResNodeInLowResElement(HighResI, HighResJ, lowResElementCoordinatesInHighRes))
   {
      return true;
   }
   
   //initialize lowResElementCoordinates
   unsigned int lowResElementCoordinates[4] = { 0 };
   
   lowResElementCoordinates[1] = lowResMap->firstJ(useGhostNodesInLowRes);
   lowResElementCoordinates[3] = lowResMap->firstJ(useGhostNodesInLowRes) + 1;

   //and hit the road...
   for( ; lowResElementCoordinates[3] <= lowResMap->lastJ(useGhostNodesInLowRes); ++lowResElementCoordinates[1], ++lowResElementCoordinates[3] )
   {
      lowResElementCoordinates[0] = lowResMap->firstI(useGhostNodesInLowRes);
      lowResElementCoordinates[2] = lowResMap->firstI(useGhostNodesInLowRes) + 1;
     
      for( ; lowResElementCoordinates[2] <= lowResMap->lastI(useGhostNodesInLowRes); ++lowResElementCoordinates[0], ++lowResElementCoordinates[2] )
      {
          
          lowResGrid->convertToGrid( (*HighResGrid), lowResElementCoordinates[0], lowResElementCoordinates[1], 
                                                     lowResElementCoordinatesInHighRes[0], lowResElementCoordinatesInHighRes[1]);
          lowResGrid->convertToGrid( (*HighResGrid), lowResElementCoordinates[2], lowResElementCoordinates[3], 
                                                     lowResElementCoordinatesInHighRes[2], lowResElementCoordinatesInHighRes[3]);

          if(isHighResNodeInLowResElement(HighResI, HighResJ, lowResElementCoordinatesInHighRes))
          {
             nodalValuesInLowResElement[0] = lowResMap->getValue(lowResElementCoordinates[0], lowResElementCoordinates[1], depth );
             nodalValuesInLowResElement[1] = lowResMap->getValue(lowResElementCoordinates[2], lowResElementCoordinates[1], depth );
             nodalValuesInLowResElement[2] = lowResMap->getValue(lowResElementCoordinates[2], lowResElementCoordinates[3], depth );
             nodalValuesInLowResElement[3] = lowResMap->getValue(lowResElementCoordinates[0], lowResElementCoordinates[3], depth );

             return true;
          }
       }
   }

   return ret;
}
bool DistributedGridMap::isHighResNodeInLowResElement(const unsigned int &HighResI, const unsigned int &HighResJ, unsigned int lowResElementCoordinatesInHighRes[])
{
   bool ret = false;

   if(
	HighResI >= lowResElementCoordinatesInHighRes[0] && 
	HighResI <= lowResElementCoordinatesInHighRes[2] &&
        HighResJ >= lowResElementCoordinatesInHighRes[1] && 
	HighResJ <= lowResElementCoordinatesInHighRes[3] 
     )
    {
	ret = true;
    }

    return ret;
}
bool DistributedGridMap::isHighResNodeInLowResGrid(const unsigned int &HighResI, const unsigned int &HighResJ, const GridMap *lowResMap,
                                                           const Grid *lowResGrid, const Grid *highResGrid)
{
   //bool ret = false;

   unsigned int lowResI, lowResJ;

   return (highResGrid->convertToGrid( (*lowResGrid), HighResI, HighResJ, lowResI, lowResJ));
   //cout<<lowResI<<","<<lowResJ<<endl;

   //ret = lowResMap->isGridPoint (lowResI, lowResJ, 0);

   //return ret;
}
