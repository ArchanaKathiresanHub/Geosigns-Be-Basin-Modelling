/// File global_typedefs.h holds typedefs used globally

#ifndef __global_typedefs__
#define __global_typedefs__

#include <map>
#include <string>
using namespace std;

#include "traplist.h"
using namespace TrapListNameSpace;

#include "linedata.h"

/// namespace FileRequestData holds typedefs used to 
/// build an stl::map type type that holds details about 
/// user selections
/// It allows for multiple selection of Reservoirs which may
/// be useful in the future
namespace FileRequestData
{
   // transient id list : age-> transId
   typedef map<double, int> TransIdMap;
   typedef TransIdMap::iterator TransIdMapIT;
   typedef TransIdMap::const_iterator const_TransIdMapIT;
   
   // trap data: persis trap Id -> transient id list
   typedef map<int, TransIdMap> TrapDataMap; 
   typedef TrapDataMap::iterator TrapDataMapIT;
   typedef TrapDataMap::const_iterator const_TrapDataMapIT;
   
   // reservoir trap data : reservoir name -> trap data
   typedef map<string, TrapDataMap> ReservoirTrapDataMap;
   typedef ReservoirTrapDataMap::iterator ReservoirTrapDataMapIT;
   typedef ReservoirTrapDataMap::const_iterator const_ReservoirTrapDataMapIT;
}

/// namespace TrapIdData holds typedefs relating to 
/// trap id's for each reservoir and age
namespace TrapIdData
{
   // trap info map : age -> trap list
   typedef map<double, TrapList*> TrapInfoMap;
   typedef TrapInfoMap::const_iterator const_TrapInfoMapIT;
   typedef TrapInfoMap::iterator TrapInfoMapIT;
   
   // trap info history : reservoir name -> trap info map
   typedef map<string, TrapInfoMap> TrapInfoHistory;
   typedef TrapInfoHistory::iterator TrapInfoHistoryIT;
   typedef TrapInfoHistory::const_iterator const_TrapInfoHistoryIT;
}

/// namespace PlotData holds typedefs and map containers
/// to handle graph legend data
namespace PlotData
{
   struct LegendType
   {
      LegendType (const string& n, int o = -1)
         : name (n), order (o) {}
      
      LegendType (const char *n, int o = -1)
         : name (n), order (o) {}
      
      // add less than for stl::map based on order number or name
      bool lessThan (const LegendType& rhs) const 
      { 
         return (order == rhs.order) ? name < rhs.name : order < rhs.order; 
      }
      
      const char* c_str() const { return name.c_str(); }
      
      string name;
      int order;
   };
 
   // define sort type for Line Group map
   class SortByOrder : public binary_function <LegendType, LegendType, bool>
   {
   public:
      bool operator () (const LegendType& lhs, const LegendType& rhs) const
      {
         return lhs.lessThan (rhs);
      }
   };
   
   typedef map<LegendType, LineData, SortByOrder> LineGroup;
   typedef LineGroup::iterator LineGroupIT;
   typedef LineGroup::const_iterator const_LineGroupIT;
   
   typedef map<int, LineGroup> TrapLineGroup;
   typedef TrapLineGroup::const_iterator const_TrapLineGroupIT;
   
   typedef map<string, TrapLineGroup> ReservoirLineGroup;
   typedef ReservoirLineGroup::const_iterator const_ReservoirLineGroupIT;
   
   typedef vector<string> PropertyList;
   typedef PropertyList::iterator PropertyListIT;
   typedef PropertyList::const_iterator const_PropertyListIT;
}

#endif
