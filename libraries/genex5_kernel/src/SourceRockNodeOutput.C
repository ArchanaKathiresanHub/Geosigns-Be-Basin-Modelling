#include "SourceRockNodeOutput.h"
#include "SpeciesResult.h"
#include "ChemicalModel.h"
#include "Constants.h"

#include "GenexResultManager.h"
namespace Genex5
{

SourceRockNodeOutput::SourceRockNodeOutput(const double &in_currentTime):
m_currentTime(in_currentTime)
{
   m_ExmTot = m_TotalRetainedOM = m_MobilOMConc = 0.0;
   
   using namespace CBMGenerics;
   GenexResultManager & theResultManager = GenexResultManager::getInstance();
   
   int i;
   for(i = 0; i < GenexResultManager::NumberOfResults; ++i )
   {
      if(theResultManager.IsResultRequired( i ))
      {
         m_ResultsByResultId[i] = 0.0;
      }
   }
}
SourceRockNodeOutput::~SourceRockNodeOutput()
{
   std::map<std::string,SpeciesResult*>::iterator it;
   for(it = m_SpeciesResultsBySpeciesName.begin(); it != m_SpeciesResultsBySpeciesName.end(); ++it)
   {
      delete (it->second);
   }
   m_SpeciesResultsBySpeciesName.clear();
}
void SourceRockNodeOutput::AddSpeciesResult(const std::string &SpeciesName, SpeciesResult* theResult)
{
   std::map<std::string,SpeciesResult*>::iterator it = m_SpeciesResultsBySpeciesName.find(SpeciesName);
   if(it == m_SpeciesResultsBySpeciesName.end()) //if it does not exist, add
   {
       m_SpeciesResultsBySpeciesName[SpeciesName] = theResult;
   }
   else//just in case
   {
      SpeciesResult* result2Delete = it->second;
      it->second = theResult;
      delete (result2Delete);  
   }
}
void SourceRockNodeOutput::PrintBenchmarkModelFluxData(ofstream &outputfile) const
{
   int numberOfSpecies = ChemicalModel::GetNumberOfSpecies();
   outputfile<<0.0<<","<<m_currentTime<<",";

   for(int id = 1;id <= numberOfSpecies; id++)
   {
      const std::string SpeciesName=ChemicalModel::GetSpeciesNameById(id);
      std::map<std::string,SpeciesResult*>::const_iterator it=m_SpeciesResultsBySpeciesName.find(SpeciesName);
      if(it != m_SpeciesResultsBySpeciesName.end())
      {
         outputfile<< it->second->GetFlux() <<",";
      }
      else
      {
         outputfile<< "0.0,";//for the first timeinstance
      }
   }

   using namespace CBMGenerics;
   GenexResultManager::ResultId theID = GenexResultManager::ExpulsionApiInst;

   std::map<int,double>::const_iterator resIt =  m_ResultsByResultId.find(theID);
   double ValueExpulsionApiInst = 0.0;
   if(resIt != m_ResultsByResultId.end() )
   {
      ValueExpulsionApiInst = resIt->second;
   }

   double ValueExpulsionGasOilRatioInst = 0.0;
   theID = GenexResultManager::ExpulsionGasOilRatioInst;
   resIt =  m_ResultsByResultId.find(theID);
   if(resIt != m_ResultsByResultId.end() )
   {
      ValueExpulsionGasOilRatioInst = resIt->second;
   }
   outputfile<<ValueExpulsionApiInst<<","<<ValueExpulsionGasOilRatioInst<<",";
   outputfile<<endl;
}
void SourceRockNodeOutput::PrintBenchmarkModelCumExpData(ofstream &outputfile) const
{
   int numberOfSpecies = ChemicalModel::GetNumberOfSpecies();
   outputfile<<0.0<<","<<m_currentTime<<",";

   for(int id = 1;id <= numberOfSpecies; id++)
   {
      const std::string SpeciesName = ChemicalModel::GetSpeciesNameById(id);
      std::map<std::string,SpeciesResult*>::const_iterator it = m_SpeciesResultsBySpeciesName.find(SpeciesName);
      if(it!=m_SpeciesResultsBySpeciesName.end())
      {
         outputfile<< it->second->GetExpelledMass()<<",";
      }
      else
      {
         outputfile<< "0.0,";//for the first timeinstance
      }
   }
   using namespace CBMGenerics;
   GenexResultManager::ResultId theID = GenexResultManager::ExpulsionApiCum;

   std::map<int,double>::const_iterator resIt =  m_ResultsByResultId.find(theID);

   double ValueExpulsionApiCum = 0.0;

   if(resIt != m_ResultsByResultId.end() )
   {
      ValueExpulsionApiCum = resIt->second;
   }

   double ValueExpulsionGasOilRatioCum = 0.0;

   theID = GenexResultManager::ExpulsionGasOilRatioCum;

   resIt =  m_ResultsByResultId.find(theID);

   if(resIt != m_ResultsByResultId.end() )
   {
      ValueExpulsionGasOilRatioCum = resIt->second;
   }
   outputfile<<0.0<<","<<ValueExpulsionApiCum<<","<<ValueExpulsionGasOilRatioCum<<",";
   outputfile<<endl;
}
void SourceRockNodeOutput::PrintSensitivityResultsOnFile(ofstream &outputfile) const
{
   using namespace CBMGenerics;
   GenexResultManager::ResultId theID = GenexResultManager::OilExpelledCum;
   std::map<int,double>::const_iterator resIt =  m_ResultsByResultId.find(theID);
   double ValueOilExpelledCum = 0.0;
   if(resIt != m_ResultsByResultId.end() )
   {
      ValueOilExpelledCum = resIt->second;
   }
   outputfile<<m_currentTime<<","<<GetTotalMass()<<","<<ValueOilExpelledCum<<","<<m_MobilOMConc<<endl;
}
SpeciesResult *SourceRockNodeOutput::GetSpeciesResultByName(const std::string &SpeciesName) const
{
   SpeciesResult *functionReturn = 0;
   std::map<std::string,SpeciesResult*>::const_iterator it = m_SpeciesResultsBySpeciesName.find(SpeciesName);
   if(it != m_SpeciesResultsBySpeciesName.end())
   {
      functionReturn = it->second;
   }
   return functionReturn;
}
void SourceRockNodeOutput::PrintBenchmarkModelConcData(ofstream &outputfile)  const
{
   int numberOfSpecies=ChemicalModel::GetNumberOfSpecies();
   outputfile<<0.0<<","<<m_currentTime<<",";             

   for(int id = 1;id <= numberOfSpecies; id++)
   {
      const std::string SpeciesName=ChemicalModel::GetSpeciesNameById(id);
      std::map<std::string,SpeciesResult*>::const_iterator it=m_SpeciesResultsBySpeciesName.find(SpeciesName);
      if(it!=m_SpeciesResultsBySpeciesName.end())
      {
         outputfile<< it->second->GetConcentration() <<",";
      }
      else
      {
         outputfile<< "0.0,";//for the first timeinstance
      }
   }
   outputfile<<0.0<<","<<0.0<<",";    
   outputfile<<endl;

}

double SourceRockNodeOutput::GetResult(const int &theId) const
{
   std::map<int,double>::const_iterator resIt =  m_ResultsByResultId.find(theId);
   double Value = Genex5::Constants::s_undefinedValue;
   if(resIt != m_ResultsByResultId.end() )
   {
      Value = resIt->second;
   }
   return Value;
}
void SourceRockNodeOutput::SetResult(const int &theId, const double &theResult)
{
   m_ResultsByResultId[theId] = theResult;
}

  

}
