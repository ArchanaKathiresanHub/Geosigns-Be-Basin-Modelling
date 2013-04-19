#ifndef _GENEX5_SOURCE_ROCK_ADSORPTION_HISTORY_H_
#define _GENEX5_SOURCE_ROCK_ADSORPTION_HISTORY_H_

#include <vector>

#include "NodeAdsorptionHistory.h"
#include "AdsorptionProjectHandle.h"
#include "PointAdsorptionHistory.h"

#include "Interface/Interface.h"
#include "Interface/ProjectHandle.h"

using namespace DataAccess;

class SourceRockAdsorptionHistory {

public :

   SourceRockAdsorptionHistory ( Interface::ProjectHandle* projectHandle, 
                                 AdsorptionProjectHandle*       adsorptionProjectHandle,
                                 PointAdsorptionHistory*        record );


   // This object now take control over the life-time of the NodeAdsorptionHistory object.
   void setNodeAdsorptionHistory ( Genex5::NodeAdsorptionHistory* adsorptionhistory );

   Genex5::NodeAdsorptionHistory* getNodeAdsorptionHistory ();
   

   // Save all (if any) collected results.
   void save ();


private :

   Interface::ProjectHandle* m_projectHandle;
   AdsorptionProjectHandle*       m_adsorptionProjectHandle;
   PointAdsorptionHistory*        m_historyRecord;
   Genex5::NodeAdsorptionHistory* m_adsorptionHistory;

};


typedef std::vector<SourceRockAdsorptionHistory*> SourceRockAdsorptionHistoryList;

#endif // _GENEX5_SOURCE_ROCK_ADSORPTION_HISTORY_H_
