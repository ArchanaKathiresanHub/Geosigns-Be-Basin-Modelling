/// Class EventNode holds all the attributes of an event at a specific time plus
/// the functionality needed to search the sources of that event

#include "eventnode.h"
#include "includestreams.h"

//
// EVENT NODE PUBLIC METHODS
//
void EventNode::init (const string& srcName, const string& key ) 
{
   m_nodeAttributes.layerName = srcName;
   m_nodeAttributes.trapTotalAmt = 0;   
   m_nodeAttributes.key = key;   
   m_initialised = true;
}

/// EventNode::addSourceNode creates or finds a SrcNode (based on source info from a row in the MigrationIoTbl)
/// that points to an EventNode which is contributing to this EventNode object. 
/// The SrcNode is added to this EventNode objects List of SrcNodes.
void EventNode::addSourceNode (const string &key, EventNode* srcEventNode, double srcAmt)
{  
   // create or find source node
   SrcNode *srcNode = &(m_srcNodes[key]);
   
   // if src node is new, set up pointer to source event node
   if ( ! srcNode->srcNodePtr ) srcNode->srcNodePtr = srcEventNode;
   
   // accumulate total amt coming from this source event node
   srcNode->srcAmt += srcAmt;
   
   // accumulate total amt in this dest node
   m_nodeAttributes.trapTotalAmt += srcAmt;
}

/// EventNode::calculatePercents calculates percentage contributions of all
/// SrcNodes once all SrcNodes have been found and the total contribution to this
/// EventNode has been established
void EventNode::calculatePercents ()
{
   SourceNodeVector_IT nodeIt, endNode (m_srcNodes.end());
   for ( nodeIt = m_srcNodes.begin(); nodeIt != endNode; ++nodeIt )
   {
      (nodeIt->second).percentOfTotal =
            ((nodeIt->second).srcAmt > 0) 
            ? (nodeIt->second).srcAmt / m_nodeAttributes.trapTotalAmt
         :  0;
   }
}

void EventNode::printContents () const
{
   cout << "key: " << key() << endl;
   cout << "Src Nodes:"<< endl;
   const_SourceNodeVector_IT nodeIt, endNode (m_srcNodes.end());
   for ( nodeIt = m_srcNodes.begin(); nodeIt != endNode; ++nodeIt )
   {
      cout << "    Src name : " << ((nodeIt->second).srcNodePtr)->m_nodeAttributes.layerName 
           <<  "("  << ((nodeIt->second).srcNodePtr)->m_nodeAttributes.key << ")" << endl 
           << "    contributes " << (nodeIt->second).srcAmt
           << " which is " << (nodeIt->second).percentOfTotal  << " percent" << endl;
   }
   cout << endl;
}

/// EventNode::recursiveSearchForSource recursively searches the node tree
/// until reaching a first source or sources and then passes the orignal source
/// name and percent contribution back up the tree so each Event Node will have
/// a list of its original sources and their contributions. Copying the info back up the tree
/// means all nodes only have to be traversed once
void EventNode::recursiveSearchForSource () 
{
   // only do search if this node does not already know
   // its source contributions
   if ( m_srcContribs.empty() )
   {
      // if this node has source nodes, then search them first
      if ( m_srcNodes.size() > 0 )
      {
         searchSourceNodes ();
      }
      else
      {
         // this node has no source nodes, so must be a source itself
         createSourceContribution ();
         //  printContents();
      }
   }
}

/// EventNode::searchSourceNodes goes through this EventNodes list of 
/// SrcNodes and calls each SrcNodes EventNode::recursiveSearchForSource method
void EventNode::searchSourceNodes ()  
{
   // loop all source nodes belonging to this destination node
   SourceNodeVector_IT nodeIt;
   SourceNodeVector_IT endNode (m_srcNodes.end());
   for ( nodeIt = m_srcNodes.begin(); nodeIt != endNode; ++nodeIt )
   {     
      // call search recursively on the src node's src nodes
      ((nodeIt->second).srcNodePtr)->recursiveSearchForSource ();
      
      // now that this nodes sources have been found, save the source names
      // and percent contributions
      copyContributionsFromSource (&(nodeIt->second));
   }
}

/// EventNode::createSourceContribution is called if this EventNode is an original
/// source contribution, Eg: a source rock. Its contribution will be 100%
void EventNode::createSourceContribution ()  
{
   // create initial source rock contribution, (100 percent)
   m_srcContribs[layerName()] = 1.0;
}

/// EventNode::copyContributionsFromSource copies the list of contributions (source name + percent)
/// from all its sources to its own list of contribution. 
void EventNode::copyContributionsFromSource (SrcNode* srcNode)  
{
   // for each node contributing to this node, record its source name and percentage contribution
   EventNode *src = srcNode->srcNodePtr;
   const_SourcePercentContributions_IT contribIt, endContrib = src->m_srcContribs.end();
   for ( contribIt = src->m_srcContribs.begin(); contribIt != endContrib; ++contribIt )
   {
      (m_srcContribs[contribIt->first]).percent += (srcNode->percentOfTotal * (contribIt->second).percent);
   }
}

