/// namespace TrapListNameSpace holds a collection of structures and typedefs to handle
/// Trap data

#ifndef __traplist__
#define __traplist__

#include <vector>
#include <map>
using namespace std;

namespace TrapListNameSpace
{
   /// struct TrapCoords is a template to hold trap coordinates as any time
   /// usually float or int
   template<typename T> 
   struct TrapCoords
   {
      TrapCoords (void) {}
      TrapCoords (T newI, T newJ) : i (newI), j (newJ) {}
      T i;
      T j;
   }; 
   
   typedef TrapCoords<int> GridCoords;
   typedef TrapCoords<float> LocationCoords;
   typedef vector <GridCoords> CoordList;
   typedef CoordList::iterator CoordIt;
   typedef CoordList::const_iterator const_CoordIt;
   
   /// Struct TrapInfo holds necessary info about a specific trap
   typedef int TrapId;
   typedef struct TrapInfo
   {
      // ctor / dtor
      TrapInfo (void);
      TrapInfo (int p_id, int t_id) : persisId (p_id), transId (t_id) {}
      
      // methods
      void reinit ();
      
      // member variables
      TrapId persisId;
      TrapId transId;
      CoordList coordList;
      LocationCoords locTopDepth;
   } TrapInfo;
   
   typedef vector<TrapInfo> TrapInfoList;
   typedef TrapInfoList::iterator TrapInfoListIT;
   typedef TrapInfoList::const_iterator const_TrapInfoListIT;  
   
    typedef map <TrapId, TrapInfoListIT> TrapLocation;
    typedef TrapLocation::iterator TrapLocationIT;
    typedef TrapLocation::const_iterator const_TrapLocationIT;
   
    /// Class TrapList maintains a list of TrapInfo objects
    /// the list is unsorted but by storing its iterators in 2 sorted lists sortying by 
    /// transient id and persistent id, the trap list can be accessed as if its sorted by
    /// one or the other depending on what is needed at the time
   class TrapList
   {
   public:
      // ctor / dtor   
      TrapList  (void) {} 
      TrapList (int reserve); 
      ~TrapList (void);
      
      // METHODS
      void add (const TrapInfo &trap);
      void deleteList (void);
      inline const TrapInfoList* getTrapList (void) const; 
      TrapInfo* getTrapWithTransId (TrapId id) const;
      TrapInfo* getTrapWithPersisId (TrapId id) const;
      inline int size (void) const;
      inline void setReserve (int size); 
      inline const TrapLocation& getSortedTransIdList () const;
      inline const TrapLocation& getSortedPersisIdList () const;
      
   private:     
      // MEMBERS
      TrapInfoList trapList;
      TrapLocation transientList;
      TrapLocation persistentList;
      
      // dissallow copy and assignment 
      TrapList& operator= (const TrapList& rhs);
      TrapList  (const TrapList& rhs);
   };
   
   // inline TrapList methods
   const TrapInfoList* TrapList::getTrapList (void) const
   {
      return &trapList; 
   }
   
   int TrapList::size (void) const
   { 
      return trapList.size(); 
   }
   
   void TrapList::setReserve (int size)
   {
      trapList.reserve (size); 
   }
   
   const TrapLocation& TrapList::getSortedTransIdList () const
   {
      return transientList;
   }
   
   const TrapLocation& TrapList::getSortedPersisIdList () const
   {
      return persistentList;
   }
   
} // end namespace

#endif
