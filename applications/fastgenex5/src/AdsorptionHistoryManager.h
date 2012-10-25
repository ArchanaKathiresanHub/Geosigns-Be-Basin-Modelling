#ifndef _GENEX5_ADSORPTION_HISTORY_MANAGER_H_
#define _GENEX5_ADSORPTION_HISTORY_MANAGER_H_

#include <map>

#include "AdsorptionProjectHandle."
#include "GenexSimulator.h"
#include "SourceRock.h"

class AdsorptionHistoryManager {

public :

   AdsorptionHistoryManager ( GenexSimulation::GenexSimulator* genexProjectHandle,
                              AdsorptionProjectHandle*         adsorptionProjectHandle );

   ~AdsorptionHistoryManager ();

   void collectHistory ( const SourceRock* sourceRock );

private :

   GenexSimulation::GenexSimulator* m_genexProjectHandle;

};


#endif // _GENEX5_ADSORPTION_HISTORY_MANAGER_H_
