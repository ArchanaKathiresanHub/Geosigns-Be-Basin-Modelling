#ifndef _GENEX6_LIBRARY__CONSTANT_GRID_INTERPOLATOR_H_
#define _GENEX6_LIBRARY__CONSTANT_GRID_INTERPOLATOR_H_

#include "Interface/GridMap.h"

#include "LocalGridInterpolator.h"

namespace Genex6 {

   class ConstantGridInterpolator : public LocalGridInterpolator {

   public :

      ConstantGridInterpolator ( const DataAccess::Interface::GridMap*  data );
   
      ~ConstantGridInterpolator ();
   

      double evaluateProperty ( const int &I, const int &J ) const;


   private :

      const DataAccess::Interface::GridMap* m_data;

   };

}

#endif // _GENEX6_LIBRARY__CONSTANT_GRID_INTERPOLATOR_H_
