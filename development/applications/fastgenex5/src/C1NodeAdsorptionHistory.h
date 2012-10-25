#ifndef _GENEX5_C1_NODE_ADSORPTION_HISTORY_H_
#define _GENEX5_C1_NODE_ADSORPTION_HISTORY_H_

#include <string>
#include <vector>

#include "NodeAdsorptionHistory.h"
#include "SourceRockNode.h"

class C1NodeAdsorptionHistory : public Genex5::NodeAdsorptionHistory {

   struct HistoryItem {
      double m_time;
      double m_temperature;
      double m_pressure;
      double m_porosity;
      double m_c1adsorped;
   };

   typedef std::vector<HistoryItem*> HistoryItemList;

public :

   ~C1NodeAdsorptionHistory ();

   /// Collect adsorption results.
   void collect ( Genex5::SourceRockNode* node );

   /// Write collected results to the stream.
   void write ( std::ostream& str );

private :

   HistoryItemList m_history;

};

Genex5::NodeAdsorptionHistory* allocateC1NodeAdsorptionHistory ();

#endif // _GENEX5_C1_NODE_ADSORPTION_HISTORY_H_
