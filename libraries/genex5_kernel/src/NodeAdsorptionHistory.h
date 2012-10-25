#ifndef _GENEX5_NODE_ADSORPTION_HISTORY_H_
#define _GENEX5_NODE_ADSORPTION_HISTORY_H_

#include <ostream>
#include <vector>

namespace Genex5 {
   class SourceRockNode;
}

namespace Genex5 {

   class NodeAdsorptionHistory {

   public :

      virtual ~NodeAdsorptionHistory () {}

      /// Collect adsorption results.
      virtual void collect ( SourceRockNode* node ) = 0;

      /// Write collected results to the stream.
      virtual void write ( std::ostream& str ) = 0;


   }; 

   typedef std::vector<NodeAdsorptionHistory*> NodeAdsorptionHistoryList;

}

#endif // _GENEX5_NODE_ADSORPTION_HISTORY_H_
