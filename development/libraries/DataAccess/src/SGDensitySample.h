#ifndef _DATA_ACCESS__SG_DENSITY_SAMPLE_H_
#define _DATA_ACCESS__SG_DENSITY_SAMPLE_H_


#include "DAObject.h"
#include "Interface.h"

#include "database.h"

namespace DataAccess {

   namespace Interface {

      /// \brief A density value used in the shale-gas simulation.
      class SGDensitySample : public DAObject {

      public :

         /// \brief Constructor.
         SGDensitySample ( ProjectHandle& projectHandle,
                           database::Record* record );

         virtual ~SGDensitySample ();

         /// \brief Get the density.
         virtual double getDensity () const;

      private :

         /// \brief The density value.
         double m_density;

      };

   }

}

//------------------------------------------------------------//

// Inline functions.

inline double DataAccess::Interface::SGDensitySample::getDensity () const {
      return m_density;
}


#endif // _DATA_ACCESS__SG_DENSITY_SAMPLE_H_
