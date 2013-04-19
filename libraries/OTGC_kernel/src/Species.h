#ifndef SPECIES_H
#define SPECIES_H



#include <vector>
#include <string>
#include <map>
#include<iostream>
#include <fstream>
#include<iomanip>
using namespace std;

namespace OTGC
{
class ChemicalModel;          
class SimulatorState;
class SpeciesProperties;
class Element;

class Species
{
public:
   Species();
   ~Species();
   Species(const std::string &in_Name, const int &in_id, ChemicalModel *const in_theModel);
   

   void ComputeTimeStep(SimulatorState &theSimulatorState,                           
                        const double &in_dT,
                        const double &s_Peff,
                        const double &s_TK,
                        const double &s_FrequencyFactor,
                        const double &s_kerogenTransformationRatio,
								const double &s_precokeTransformationRatio,
								const double &s_coke2TransformationRatio,
                        const double &s_DiffusionConcDependence,
                        const double &s_VogelFulcherTemperature,
                        const bool &in_OpenSourceRockConditions
                        );
//OTGC
   void SetProperties(SpeciesProperties *const in_Props);

   //structure formation
   void UpdateCompositionByElement(Element *const theElem,const double &inComposition);
   void UpdateMassFactorBySpeciesName(const std::string &SpeciesName, const double &Factor);
   void UpdateDiffusionEnergy1(const double &in_diffEnergy);

   void SetPositiveGenRate(const double &in_positiveGenRate);
   double GetPositiveGenRate() const;
   void AddPositiveGenRate(const double &generationRateToAdd);

   //I/O
   double GetMassFactorBySpecies(const std::string &productName) const ;
   //OTGC
   const std::map<std::string,double> & GetProductMassFactors() const;
   //OTGC
   std::string GetName() const;
   int GetId() const ;
   int GetNumberOfElements();
   double GetCompositionByElement(const std::string &elemName) const;
   const std::string GetCompositionCode()const {return m_compositionCode;}
   SpeciesProperties *GetSpeciesProperties();

   void OutputCompositionOnScreen() const;
   void OutputPropertiesOnScreen() const;

   void OutputCompositionOnFile(ofstream &outfile) const;
   void OutputMassFactorsOnFile(ofstream &outfile) const;
   void OutputMassFactorsOnScreen() const;
   void OutputPropertiesOnFile(ofstream &outfile) const;
   void OutputOnFile(ofstream &outfile) const;
   void PrintBenchmarkProperties(ofstream &outfile, const int &id) const;
   void PrintBenchmarkStoichiometry(ofstream &outfile) const;
   void OutputResults(const bool &value);
   bool IsSpeciesOutputRequired() const;


   //General, properties computation ,
   //The Species class provides this computational functionality, probably should change to function objects in order to increase flexibility
   //Results stored in the  object m_theProps
   //class SpeciesPropertiesprovides the Get-Set  interface
   double ComputeMolWeight() const;        //called every time there is an update in the Composition,which happens often in preprocessing
   double ComputeAromaticity() const;
   double ComputeReactionOrder() const;
   double ComputeB0() const;
   double ComputeDensity() const;      //necessary only in the case of immobile species

   double GetMolWeight() const;
   double GetDensity() const;
   double GetAromaticity() const;

   void UpdateProperties();//Calls SpeciesProperties::Update(), 
   void UpdatePositiveGenerationRatesOfDaughters(const double &NegativeGenerationRate);

   //utilities
   double ComputeHCCorrector() const;          //HC correction according to Van Krevelen

   //OTGC
   double ComputeArrheniusReactionRate2a(const SimulatorState &theSimulatorState, const double &s_FrequencyFactor, 
                                             const double &s_Peff, const double &s_TK,  const double &s_VogelFulcherTemperature,
                                             const double &s_kerogenTransformationRatio, const double &s_precokeTransformationRatio, 
                                             const double &s_coke2TransformationRatio   
                                             );
   //OTGC

private:       
   std::string m_name;
   int m_id;
   bool m_outputResults;
   //Composition
   std::string m_compositionCode;                                  //e.g species which consist of C, O, N have a compositionCode CON
   std::map<std::string,Element*>  m_theElements;                  //elements that constitute each species
   std::map<std::string,double>      m_compositionByElement;       //Composition factor by element name
   std::map<std::string,double>      m_massFactorsBySpecies;       //Computed by  Reaction::ComputeMassProductRatios(), SFmass

   //physical, chemical properties
   SpeciesProperties *m_theProps;                                  //species properties

   ChemicalModel *const m_theChemicalModel;
   
   double m_positiveGenRate;//cross link between the species reactions

   Species(const Species &);
   Species &operator=(const Species &);
   
};
inline SpeciesProperties *Species::GetSpeciesProperties()
{
   return m_theProps;
}
inline int Species::GetNumberOfElements()
{
   return m_theElements.size();
}
inline void Species::SetProperties(SpeciesProperties *const in_Props)
{
   m_theProps = in_Props;
}
inline void Species::SetPositiveGenRate(const double &in_positiveGenRate)
{
   m_positiveGenRate = in_positiveGenRate;
}
inline double Species::GetPositiveGenRate() const
{
  return  m_positiveGenRate;
}
inline std::string Species::GetName() const
{
   return m_name;
}
inline int Species::GetId() const
{
   return m_id;
}
inline void Species::OutputResults(const bool &value)
{
  m_outputResults = true; 
}
inline bool Species::IsSpeciesOutputRequired()const
{
   return m_outputResults;
}


}
#endif

