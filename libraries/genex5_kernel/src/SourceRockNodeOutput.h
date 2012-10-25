#ifndef SOURCEROCKNODEOUTPUT_H
#define SOURCEROCKNODEOUTPUT_H


#ifdef sgi
   #ifdef _STANDARD_C_PLUS_PLUS
      #include<iostream>
      #include<fstream>
      #include<iomanip>
      using namespace std;
   #else // !_STANDARD_C_PLUS_PLUS
      #include<fstream.h>
      #include<iostream.h>
      #include<iomanip.h>
   #endif // _STANDARD_C_PLUS_PLUS
#else // !sgi
   #include<iostream>
   #include <fstream>
   #include<iomanip>
   using namespace std;
#endif // sgi



#include <map>
#include <string>
namespace Genex5
{
class SpeciesResult;

//!Output of a Genex5 simulation
class SourceRockNodeOutput
{
public:
   SourceRockNodeOutput(const double &in_currentTime);
   ~SourceRockNodeOutput();

   void AddSpeciesResult(const std::string &SpeciesName, SpeciesResult* theResult);
   SpeciesResult *GetSpeciesResultByName(const std::string &SpeciesName) const;
   double GetCurrentTime() const;
   
   double GetResult(const int &theId) const;
   void SetResult(const int &theId, const double &theResult);
   
 
   double GetExmTot() const;
   void SetExmTot(const double &in_ExmTot);
   void SetTotalRetainedOM(const double &in_TotalRetainedOM);
   void SetMobileOMConc(const double &in_MobileOMConc);
   double GetTotalMass() const; 
   

   void PrintBenchmarkModelConcData(ofstream &outputfile) const;
   void PrintBenchmarkModelFluxData(ofstream &outputfile) const;
   void PrintBenchmarkModelCumExpData(ofstream &outputfile) const;
   void PrintSensitivityResultsOnFile(ofstream &outputfile) const;

private:
   std::map<std::string,SpeciesResult*> m_SpeciesResultsBySpeciesName;
   std::map<int,double>    m_ResultsByResultId;

   
   double m_currentTime;
             
   double m_ExmTot;
   double m_TotalRetainedOM;
   double m_MobilOMConc;
 
};
inline double SourceRockNodeOutput::GetCurrentTime() const
{
   return m_currentTime;
}
//Sensitivity
inline void SourceRockNodeOutput::SetExmTot(const double &in_ExmTot)
{
  m_ExmTot = in_ExmTot;
}
inline double SourceRockNodeOutput::GetExmTot() const
{
  return m_ExmTot;
}
inline void SourceRockNodeOutput::SetMobileOMConc(const double &in_MobileOMConc)
{
  m_MobilOMConc = in_MobileOMConc;
}
inline void SourceRockNodeOutput::SetTotalRetainedOM(const double &in_TotalRetainedOM)
{
    m_TotalRetainedOM = in_TotalRetainedOM;
}
inline double SourceRockNodeOutput::GetTotalMass() const
{
    return m_TotalRetainedOM + m_ExmTot;
}

}
#endif
