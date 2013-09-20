#ifndef FASTCAULDRON__SOURCE_ROCKS_TEMPORAL_INTERPOLATOR__H
#define FASTCAULDRON__SOURCE_ROCKS_TEMPORAL_INTERPOLATOR__H

#include "SourceTermTemporalInterpolator.h"

class SourceRocksTemporalInterpolator {

public :

   SourceRocksTemporalInterpolator ( Subdomain&  subdomain );

   ~SourceRocksTemporalInterpolator ();

   const SourceTermTemporalInterpolator * accessSourceRock ( const FormationSubdomainElementGrid * formationGrid ) const;

private :

   std::map<const FormationSubdomainElementGrid *, SourceTermTemporalInterpolator*> m_sourceTermInterpolators;  

}; 


#endif // FASTCAULDRON__SOURCE_ROCKS_TEMPORAL_INTERPOLATOR__H
