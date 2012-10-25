/// Class CreateUserRequest holds the details of user selected options
/// from the main user interface
/// From details of a selected trap (resevoir name, age, trap id), 
/// it establishes a trap history so can provide the trap's persistent Id
/// and its age-specific id at any age.
/// Eg: Trap Persistent id = 20;
/// age 0, id = 1
/// age 35, id = 6
/// age 70, id = 10
/// age 100, id = 1

#include "createuserrequest.h"
#include "globalnumbers.h"
using namespace Null_Values;

#include "includestreams.h"
#include "environmentsettings.h"

//
// PUBLIC METHODS
//
/// CreateUserRequest::setUserRequestData creates a map of trap history
/// with persistent id and transient id's over all the ages it exists at

void CreateUserRequest::setUserRequestData (const TrapInfoHistory &resTrapInfo, 
                                            const string &reservoirName,
                                            const double userAge,
                                            const int userTrapId)
{
   const_TrapInfoHistoryIT resInfoIt = resTrapInfo.find (reservoirName);
   if ( resInfoIt != resTrapInfo.end() ) 
   {
      // get persistent id for reservoir, user age and transId
      int persisTrapId = getPersisId (resInfoIt, userAge, userTrapId);
      if ( persisTrapId != Cauldron_Null_Trap )
      {
         // create transient Id's for reservoir name and persistent id
         TransIdMap &transIdMap = m_resTrapMap[reservoirName][persisTrapId];

         // loop snapshots for this reservoir, getting transient id at each snapshot
         const TrapInfoMap& trapsPerSnapshot = resInfoIt->second;
         const_TrapInfoMapIT snapIt, endSnaps = trapsPerSnapshot.end();
         TrapInfo *trapInfo;
         for ( snapIt = trapsPerSnapshot.begin(); snapIt != endSnaps; ++snapIt )
         {
            trapInfo = (snapIt->second)->getTrapWithPersisId (persisTrapId);
            if ( trapInfo ) transIdMap[snapIt->first] = trapInfo->transId;
         }
      }
   }

   if ( EnvironmentSettings::debugOn () )
   {
      printDebugInfo ();
   }
}

void CreateUserRequest::clearUserRequest (void)
{
   m_resTrapMap.erase (m_resTrapMap.begin(), m_resTrapMap.end());                                        
}

//
// PRIVATE METHODS
//
/// CreateUserRequest::getPersisId gets a persistent trap id for any trap
/// based on the traps reservoir name, age and transient trap id
int CreateUserRequest::getPersisId (const_TrapInfoHistoryIT &resIt, 
                                    const double userAge, 
                                    const int userTrapId) const
{
   int persisId = Cauldron_Null_Trap;

   const TrapInfoMap& trapsPerSnapshot = resIt->second;
   const_TrapInfoMapIT trapListIt = trapsPerSnapshot.find (userAge);
   
   if ( trapListIt != trapsPerSnapshot.end() )
   {
      TrapInfo *trapInfo = (trapListIt->second)->getTrapWithTransId (userTrapId);
      if ( trapInfo ) persisId = trapInfo->persisId;
   }
 
   return persisId;
}

void CreateUserRequest::printDebugInfo ()
{
   cout << endl << "User Request Created:"<< endl;
   const_ReservoirTrapDataMapIT resIt, endRes = m_resTrapMap.end();
   const_TrapDataMapIT trapIt, endTraps;
   const_TransIdMapIT transIt, endTrans;

   for ( resIt = m_resTrapMap.begin(); resIt != endRes; ++resIt )
   {
      cout << endl << "Reservoir: " << resIt->first << endl;
      const TrapDataMap& trapData = (resIt->second );
      endTraps = trapData.end();
      
      for ( trapIt = trapData.begin(); trapIt != endTraps; ++trapIt )
      {
         cout << "PersisId: " << (trapIt->first) << endl;
         const TransIdMap &transData = (trapIt->second);
         endTrans = transData.end();
         for ( transIt = transData.begin(); transIt != endTrans; ++transIt )
         {
            cout << "Age: " << transIt->first << " TransId: " << transIt->second << endl;
         }
         cout << endl;
      }
   }
}
