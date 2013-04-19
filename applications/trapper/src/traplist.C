/// namespace TrapListNameSpace holds a collection of structures and typedefs to handle
/// Trap data

#include "traplist.h"

#include "globalnumbers.h"
using namespace Null_Values;

namespace TrapListNameSpace
{
   //
   // TRAP INFO METHODS
   //
   TrapInfo::TrapInfo (void) 
      : persisId (Cauldron_Null_Trap), transId (Cauldron_Null_Trap) 
   {}
   
   // reinitialise trap info
   void TrapInfo::reinit (void)
   {
      persisId = transId = Cauldron_Null_Trap;
      coordList.erase (coordList.begin(), coordList.end());
   }
   
   //
   // TRAP LIST METHODS
   //
   TrapList::TrapList (int reserve)
   { 
      setReserve (reserve); 
   }
   
   TrapList::~TrapList (void)
   {
      deleteList  ();
   }
   
   /// TrapList::add adds a new TrapInfo object to it list and also 
   /// adds its new iterator to the transient and persistent sorted lists of iterators
   /// so the trapinfo object can be accessed quickly at a latr stage
   void TrapList::add (const TrapInfo &trap)
   {
      // insert new trap at end of vector
      TrapInfoListIT it = trapList.insert (trapList.end(), trap);
      
      // if insert successful, store trap iterator against 
      // transient id and persistent id so it can be 
      // accessed by both quickly, later on
      if ( it != trapList.end() )
      {
         transientList[trap.transId] = it;
         persistentList[trap.persisId] = it;
      }
   }
   
   void TrapList::deleteList (void)
   {
      // empty all lists
      trapList.erase (trapList.begin(), trapList.end());
      transientList.erase (transientList.begin(), transientList.end());
      persistentList.erase (persistentList.begin(), persistentList.end());
   }

   /// TrapList::getTrapWithTransId retrieves a trapInfo object by first gettings
   /// its iterator, based on its id from the transient id list and then deferencing the iterator
   TrapInfo* TrapList::getTrapWithTransId  (TrapId id) const
   {
      const_TrapLocationIT it = transientList.find (id);   
      
      if ( it != transientList.end() )
      {
         TrapInfoListIT trapIt = it->second;
         TrapInfo *retType = &(*trapIt);
         return retType;
      }
      else 
      {
         return NULL;
      }
   }
   
   /// TrapList::getTrapWithPersisId retrieves a trapInfo object by first gettings
   /// its iterator, based on its id from the persistent list and then deferencing the iterator
   TrapInfo* TrapList::getTrapWithPersisId (TrapId id) const
   {
      const_TrapLocationIT it = persistentList.find (id);   
      return (it == persistentList.end()) ? NULL : &(*(it->second));
   }
   
} // end namespace
