/// Class EventNode holds all the attributes of an event at a specific time plus
/// the functionality needed to search the sources of that event

#ifndef __eventnode__
#define __eventnode__

#include <vector>
#include <set>
#include <map>
#include <string>
using namespace std;

class EventNode;

/// Struct SrcNode holds a pointer to a node, which is a partial source of another node's volume,
/// the src amount provided by that source and the percentage that source amount is of the overall node's volume
typedef struct SrcNode
{
   SrcNode () : srcAmt (0), percentOfTotal (0), srcNodePtr (0) {}
   
   double srcAmt;
   double percentOfTotal;
   EventNode* srcNodePtr;
} SrcNode;

typedef map<string, SrcNode> SourceNodeVector;
typedef SourceNodeVector::iterator SourceNodeVector_IT;
typedef SourceNodeVector::const_iterator const_SourceNodeVector_IT;

/// Struct NodeAttributes keeps together all the attributes of a node, 
/// which represents an event at a specific time
typedef struct NodeAttributes
{  
   NodeAttributes () : trapTotalAmt (0) {}

   string layerName;
   string key;

   double trapTotalAmt;
} NodeAttributes;

// typedefs for list of source rocks and their percentage contribution
// to a node
typedef struct SrcPercent
{
   SrcPercent () : percent (0) {}
   SrcPercent (double p) : percent (p) {}
   SrcPercent (const SrcPercent& rhs) : percent (rhs.percent) {}
   double percent;
} SrcPercent;

typedef map <string, SrcPercent> SourcePercentContributions;
typedef SourcePercentContributions::iterator SourcePercentContributions_IT;
typedef SourcePercentContributions::const_iterator const_SourcePercentContributions_IT;

// the event node class holds all the attributes of an event at a specific time plus
// the functionality needed to search the sources of that event
class EventNode
{
public:
   EventNode () : m_initialised (false) {}
   ~EventNode () {}
    
   double totalAmt () const { return m_nodeAttributes.trapTotalAmt; }
   void setTotalAmt (double amt) { m_nodeAttributes.trapTotalAmt = amt; }
   SourceNodeVector& sourceNodeVector() { return m_srcNodes; }
   int numSourceNodes () const { return m_srcNodes.size(); }
   void addSourceNode (const string& key, EventNode* srcNode, double srcAmt);   
   void calculatePercents ();
   const string& layerName () const { return m_nodeAttributes.layerName; }
   const string& key () const { return m_nodeAttributes.key; }
   void recursiveSearchForSource ();
   bool initialised () const { return m_initialised; }
   void init (const string& srcName, const string &key); 
   void printContents () const;   
   const SourcePercentContributions& getSourceContributions () const { return m_srcContribs; }

private:
   NodeAttributes m_nodeAttributes;
   SourceNodeVector m_srcNodes;   
   SourcePercentContributions m_srcContribs;
   bool m_initialised;
   
   void searchSourceNodes ();
   void createSourceContribution ();
   void copyContributionsFromSource (SrcNode* srcNode);
};

#endif



