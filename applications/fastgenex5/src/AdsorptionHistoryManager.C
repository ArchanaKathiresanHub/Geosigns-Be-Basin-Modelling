#include "AdsorptionHistoryManager.h"

#include "PointAdsorptionHistory.h"

AdsorptionHistoryManager::AdsorptionHistoryManager ( GenexSimulation::GenexSimulator* genexProjectHandle,
                                                     AdsorptionProjectHandle*         adsorptionProjectHandle ) :
   
   m_genexProjectHandle ( genexProjectHandle ),
   m_adsorptionProjectHandle ( adsorptionProjectHandle )
{
}
