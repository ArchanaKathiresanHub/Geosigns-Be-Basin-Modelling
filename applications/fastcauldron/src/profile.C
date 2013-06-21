#include "profile.h"

#include <iostream>
#include <iomanip>
using namespace std;


void LogStageBegin(ProfileManager& pProfileManager, const string& EventName)
{
  Stage* pStage;
  profile_it = pProfileManager.find(EventName);
  if (profile_it == pProfileManager.end()) {
    pStage = new Stage;
    pStage->Name = EventName;
  } else {
    pStage = profile_it->second;
  }
  PetscTime(&pStage->StartTime);
  pProfileManager[EventName] = pStage; 
}

void LogOutput(ProfileManager& pProfileManager, int rank)
{

  if (rank) {
    clearLog(pProfileManager);
    return;
  }
  cout << endl << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
  Stage* pStage;
  for (profile_it = pProfileManager.begin(); 
       profile_it != pProfileManager.end(); profile_it++) {
    pStage = profile_it->second;
    cout << setw(50) << pStage->Name << setw(25) << pStage->CalculationTime << endl;
  }
  cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl << endl;
  clearLog(pProfileManager);
}

void LogClear(ProfileManager& pProfileManager)
{
  ProfileManager::iterator pm_iter;
  for (pm_iter = pProfileManager.begin(); pm_iter != pProfileManager.end(); pm_iter++) {
    delete  pm_iter->second;
  }
  pProfileManager.clear();
}

Stage::Stage()
{
  Name = "";
  StartTime = 0.0;
  EndTime = 0.0;
  CalculationTime = 0.0;
} 

Stage::~Stage()
{

}

void LogStageEnd(ProfileManager& pProfileManager, const string& EventName)
{
  Stage* pStage;
  profile_it = pProfileManager.find(EventName);
  if (profile_it == pProfileManager.end()) {
    cout << "Error Trying to Log Stage: " << EventName << endl;
    exit(1);
  } else {
    pStage = profile_it->second;
    PetscTime(&pStage->EndTime); 
    pStage->CalculationTime += pStage->EndTime - pStage->StartTime;
  }

}
