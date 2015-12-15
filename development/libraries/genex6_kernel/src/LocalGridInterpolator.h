#ifndef _GENEX6_LIBRARY__LOCAL_GRID_INTERPOLATOR_H_
#define _GENEX6_LIBRARY__LOCAL_GRID_INTERPOLATOR_H_


namespace Genex6
{

   class LocalGridInterpolator
   {
   public:

      ///constructor
      LocalGridInterpolator() {}
   
      ///destructor
      virtual ~LocalGridInterpolator(){}
   
      virtual double evaluateProperty(const int &I, const int &J, const double &in_timeInstance) const = 0;

   };


}

#endif // _GENEX6_LIBRARY__LOCAL_GRID_INTERPOLATOR_H_

