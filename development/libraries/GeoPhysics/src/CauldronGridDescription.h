#ifndef _GEOPHYSICS__CAULDRON_GRID_DESCRIPTION_H_
#define _GEOPHYSICS__CAULDRON_GRID_DESCRIPTION_H_

#include <iostream>
#include <string>

namespace GeoPhysics {

   class CauldronGridDescription {
   public:
      CauldronGridDescription ();

      unsigned int nrI;
      unsigned int nrJ;

      double originI;
      double originJ;

      double deltaI;
      double deltaJ;

      void print ( std::ostream& o = std::cout ) const;

      /// Return the string representation of the cauldron-grid-description.
      std::string image () const;

   };

}

#endif // _GEOPHYSICS__CAULDRON_GRID_DESCRIPTION_H_
