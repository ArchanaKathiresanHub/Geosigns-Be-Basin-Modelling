#ifndef _SG_DENSITY_SAMPLE_H_
#define _SG_DENSITY_SAMPLE_H_

#include "database.h"

class AdsorptionProjectHandle;


class SGDensitySample {

public :

   SGDensitySample ( AdsorptionProjectHandle* adsorptionProjectHandle,
                     database::Record*        record );

   double getDensity () const;

private :

   double m_density;

};

inline double SGDensitySample::getDensity () const {
   return m_density;
}


#endif // _SG_DENSITY_SAMPLE_H_
