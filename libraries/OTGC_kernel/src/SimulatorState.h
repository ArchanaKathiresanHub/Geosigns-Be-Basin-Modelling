#ifndef _OTGC_SIMULATORSTATE_H
#define _OTGC_SIMULATORSTATE_H


#include <string>
#include <map>
#include <vector>
#include<iostream>
#include <fstream>
#include<iomanip>
using namespace std;


namespace OTGC
{
class SpeciesState;

//! Simulator is the main controlling class 
/*! 
   Provides the core computational functionality for OTGC to client applications through the public interface computeInterval()
*/
class SimulatorState
{
public:
   SimulatorState();
   SimulatorState(const double &inReferenceTime, const vector<std::string> &SpeciesInChemModel, const std::map<string, double> &initSpeciesConcs);

   virtual ~SimulatorState();

   double GetSpeciesConcentrationByName(const std::string &in_SpeciesName) const ;
   
   void GetSpeciesStateConcentrations(std::map<string, double> &currentSpeciesConcs) const;
   
   SpeciesState *GetSpeciesStateByName(const std::string &in_SpeciesName);
   void AddSpeciesStateByName(const std::string &in_SpeciesName, SpeciesState *theSpeciesState);

   double GetReferenceTime() const;
   void SetReferenceTime(const double in_ReferenceTime);

   bool isInitialized(){return m_isInitialized;}
   void setInitializationStatus(bool status){m_isInitialized = status;}

   void setSpeciesUltimateMassesByName(const std::map<std::string, double> &inUltimateMassesByName);
   double getSpeciesUltimateMassByName(const std::string & name);

   double getMaxPrecokeTransfRatio(){return m_maxprecokeTransformationRatio;}
   void setMaxPrecokeTransfRatio(const double &inMaxPrecokeTransRatios){m_maxprecokeTransformationRatio = inMaxPrecokeTransRatios;}
   double getMaxCoke2TransTransfRatio(){return m_maxcoke2TransformationRatio;}
   void setMaxCoke2TransTransfRatio(const double &intMaxCoke2TransTransfRatio){m_maxcoke2TransformationRatio = intMaxCoke2TransTransfRatio;}

private:
   double m_referenceTime;       
 
   double m_maxprecokeTransformationRatio;
   double m_maxcoke2TransformationRatio;
   bool m_isInitialized;
   
   std::map<std::string, SpeciesState*>   m_SpeciesStateBySpeciesName;
   std::map<std::string, double> m_UltimateMassesBySpeciesName;

   void clearSpeciesState();
   SimulatorState(const SimulatorState &);
   SimulatorState & operator=(const SimulatorState &);
   
};
inline void SimulatorState::SetReferenceTime(const double in_ReferenceTime)
{
  m_referenceTime = in_ReferenceTime; 
}
inline double SimulatorState::GetReferenceTime() const
{
   return m_referenceTime;
}

}
#endif
