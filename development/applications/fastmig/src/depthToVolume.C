#include "depthToVolume.h"

#include "functions/src/Tuple2.h"

#include <map>
#include <limits>
#include <math.h>

using functions::Tuple2;
using functions::tuple;
using std::map;
using std::numeric_limits;
using std::min;

namespace migration { namespace depthToVolume {

// I have seen 1.2815e-6 here:
const double TOLERANCE = 1.0e-4;

template <typename ITERATOR>
const MonotonicIncreasingPiecewiseLinearInvertableFunction* compute(const double& origin, 
   ITERATOR begin, ITERATOR end, double spillDepth)
{
   // For each column the following assumptions are valid:
   // - there is one top and one base 
   // - the capacity is linear between top and base.
   // The capacity of the trap is therefore determined by the capacities of the 
   // individual columns, and the top and base of each column.  Each column contributes 
   // from top to base with a certain capacity.  What we don't know here is, where the 
   // individual column starts, and how much they contribute tot the overall capacity. 
   // Therefore we determin first a sorted map with for each depth the capacity increase 
   // (i.e. the gradient of the gradient):
   map<double,double> capacityGradientIncreases;

   // In order to solve a problem with the case when the capacities of all columns are zero,
   // we determine also the lowest and highest depths of the columns:
   Tuple2<double> limits; 

#ifdef _MSC_VER
#undef max
#endif
   limits[0] =  numeric_limits<double>::max();
   limits[1] = -numeric_limits<double>::max();

   for (ITERATOR it1 = begin; it1 != end; ++it1)
   {
      limits[0] = min(limits[0],Min (spillDepth, (*it1)->getTopDepth()));
      limits[1] = max(limits[1],Min (spillDepth, (*it1)->getBottomDepth()));

      double columnCapacity = (*it1)->getCapacity(spillDepth);
      assert(columnCapacity >= 0.0);

      double columnHeight = Min (spillDepth, (*it1)->getBottomDepth()) - Min (spillDepth, (*it1)->getTopDepth());
      assert(columnHeight >= 0.0);

#if 0
      if (columnHeight == 0)
      {
	 cerr << "WARNING from depthToVolume::compute (): Trying to use zero thickness column " << (*it1) << ", spill depth = " << spillDepth << endl;
      }
#endif
	 
      if (columnCapacity == 0) continue;

      double increase = columnCapacity / columnHeight;

      // In case there is no increase (because the capacity of this column is zero) we may 
      // ignore this column:
      if (increase == 0.0) continue;

      // Add the capacity of this column from depth column->getTopDepth() and subtract 
      // the capacity from depth column->getBottomDepth().  If there is already another column 
      // with the same top or base, add the increase to the one of that column, hence use +=:

      // TODO: the algorithm is a bit inefficient when the iterators are sorted on top depth 
      // or bottom depth. Unfortunately, there seems not to be a:
      // pair<iterator, bool> map::insert(iterator hint, const value_type& x) 
      // method in map where we can provide a hint of the position, and get returned a  
      // Boolean indicating whether such an element was already present.  With such a  
      // method present, making it more efficient would have been easy, but now making this 
      // more efficient is a bit more involved (and probably beyond what the Solaris compiler 
      // can compile).
      double topDepth = Min (spillDepth,(*it1)->getTopDepth()) - origin;
      capacityGradientIncreases[topDepth] += increase;
      double baseDepth = Min (spillDepth,(*it1)->getBottomDepth()) - origin;
      capacityGradientIncreases[baseDepth] -= increase;
   }

   // We know now at what depths the capacity gradient is discontinuous.  Use this information 
   // to calculate the depth to capacity function:
   vector<MonotonicIncreasingPiecewiseLinearInvertableFunction::element_type> depthToCapacity;

   Tuple2<double> capacity; 
   capacity[0] = -numeric_limits<double>::max(); capacity[1] = 0.0;
   double capacityGradient = 0.0;

   for (map<double,double>::const_iterator it2 = capacityGradientIncreases.begin(); 
      it2 != capacityGradientIncreases.end(); ++it2)
   {
      double extraCapacity = capacityGradient * ((*it2).first - capacity[0]);
      capacity[0] = (*it2).first;
      capacity[1] += extraCapacity;

      depthToCapacity.push_back(capacity);

      capacityGradient += (*it2).second;

      // Because we add and subtract a lot of numbers, we may because of the limited 
      // numeric precision end up with capacityGradient a tiny bit below 0.0, and this 
      // should be prevented:
#ifdef DEBUG_DEPTHTOVOLUME
      if (capacityGradient < 0.0)
	 cerr << "WARNING: capacity gradient = " << capacityGradient << endl;
#endif
      assert(capacityGradient > -TOLERANCE);
      capacityGradient = max(0.0, capacityGradient);
   }

   // All columns have a top and a base, so in the end the capacityGradient should have gone 
   // back to zero:
#ifdef DEBUG_DEPTHTOVOLUME
   if (fabs(capacityGradient) > 0.0)
      cerr << "WARNING: Remaining capacity gradient = " << capacityGradient << endl;
#endif
   assert(fabs(capacityGradient) < TOLERANCE);

   // Traps with an empty levelToVolume function are more difficult to deal with than a 
   // trap with a zero levelToVolume function: 
   if (capacity[1] == 0.0) {
      depthToCapacity.push_back(functions::tuple(limits[0],0.0));
      depthToCapacity.push_back(functions::tuple(limits[1],0.0));
   }
   
   assert(depthToCapacity.size() > 1);

   // Create a MonotonicIncreasingPiecewiseLinearInvertableFunction function.  This functions 
   // interpolates between the values given vector<XY> depthToCapacity:
   MonotonicIncreasingPiecewiseLinearInvertableFunction* result = new 
      MonotonicIncreasingPiecewiseLinearInvertableFunction(depthToCapacity);
   return result;
}

} } // namespace migration::depthToVolume
