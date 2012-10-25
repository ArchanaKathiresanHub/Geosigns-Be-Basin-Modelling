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

#ifndef __createuserrequest__
#define __createuserrequest__

#include "global_typedefs.h"
using namespace FileRequestData;
using namespace TrapIdData;

#include <string>
using namespace std;

class CreateUserRequest
{
public:
   void setUserRequestData (const TrapInfoHistory &resTrapInfo, 
                            const string &reservoirName,
                            const double userAge,
                            const int userTrapId);
   void clearUserRequest (void);
   bool userRequestEmpty () { return m_resTrapMap.size() == 0; }
   const ReservoirTrapDataMap& getUserRequest () const { return m_resTrapMap; }
   
private:
   void printDebugInfo ();
   int getPersisId (const_TrapInfoHistoryIT &resIt, 
                    const double userAge, 
                    const int userTrapId) const;
   
   // private variables
   ReservoirTrapDataMap m_resTrapMap;
};

#endif
