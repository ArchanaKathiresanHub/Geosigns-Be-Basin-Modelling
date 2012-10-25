
#include "SGDensitySample.h"

#include "cauldronschemafuncs.h"
#include "AdsorptionProjectHandle.h"

SGDensitySample::SGDensitySample ( AdsorptionProjectHandle* adsorptionProjectHandle,
                                   database::Record*        record ) {

   
   m_density = database::getSGMeanBulkDensity ( record );
}
