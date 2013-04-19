#ifndef _PROFILE_H_
#define _PROFILE_H_

#include <map>
#include <string>

#include "petsc.h"

using namespace std;

/* #define FASTCAULDRON_PROFILE */

struct Stage{
  string Name;
  PetscLogDouble StartTime;
  PetscLogDouble EndTime;
  PetscLogDouble CalculationTime;
  Stage();
  ~Stage();
};

typedef map<string, Stage*, less<string> > ProfileManager;

static ProfileManager::iterator profile_it;

void   LogStageBegin(ProfileManager& pProfileManager, const string& EventName);
void   LogStageEnd(ProfileManager& pProfileManager, const string& EventName);
void   LogClear(ProfileManager& pProfileManager);
void   LogOutput(ProfileManager& pProfileManager, int rank);

inline void startLogStage(ProfileManager& pProfileManager, const string& EventName) {

#if defined (FASTCAULDRON_PROFILE)
  LogStageBegin(pProfileManager,EventName);
#endif

}

inline void endLogStage(ProfileManager& pProfileManager, const string& EventName) {

#if defined (FASTCAULDRON_PROFILE)
  LogStageEnd(pProfileManager,EventName);
#endif

}

inline void clearLog(ProfileManager& pProfileManager) {

#if defined (FASTCAULDRON_PROFILE)
  LogClear(pProfileManager);
#endif

}

inline void outputLog(ProfileManager& pProfileManager, int rank) {

#if defined (FASTCAULDRON_PROFILE)
  LogOutput(pProfileManager,rank);
#endif

}

/* #define outputLog(pm,r) { \ */
/*   #ifdef(FASTCAULDRON_PROFILE) \ */
/*     LogOutput(pm,r); \ */
/*   #endif \ */
/* } */

#endif /* _PROFILE_H_ */
