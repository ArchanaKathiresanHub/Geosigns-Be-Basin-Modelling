#ifndef _GENEX6_KERNEL__SOURCE_ROCK_ADSORPTION_HISTORY_H_
#define _GENEX6_KERNEL__SOURCE_ROCK_ADSORPTION_HISTORY_H_

#include <vector>

#include "NodeAdsorptionHistory.h"

#include "Interface.h"
#include "ProjectHandle.h"
#include "PointAdsorptionHistory.h"

namespace Genex6 {

   class SourceRockAdsorptionHistory {

   public :

      SourceRockAdsorptionHistory ( DataAccess::Interface::ProjectHandle*                projectHandle, 
                                    const DataAccess::Interface::PointAdsorptionHistory* record );


      // This object now take control over the life-time of the NodeAdsorptionHistory object.
      void setNodeAdsorptionHistory ( Genex6::NodeAdsorptionHistory* adsorptionhistory );
      void setNodeGenexHistory ( Genex6::NodeAdsorptionHistory* genexhistory );

      /// \brief 
      Genex6::NodeAdsorptionHistory* getNodeAdsorptionHistory ();
      Genex6::NodeAdsorptionHistory* getNodeGenexHistory ();
   

      // Save all (if any) collected results.
      void save ();


   private :

      DataAccess::Interface::ProjectHandle*                m_projectHandle;
      const DataAccess::Interface::PointAdsorptionHistory* m_historyRecord;
      Genex6::NodeAdsorptionHistory*                            m_adsorptionHistory;

      Genex6::NodeAdsorptionHistory*                            m_genexHistory;

   };

   typedef std::vector<SourceRockAdsorptionHistory*> SourceRockAdsorptionHistoryList;

}

#endif // _GENEX6_KERNEL__SOURCE_ROCK_ADSORPTION_HISTORY_H_
