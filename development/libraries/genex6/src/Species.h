#ifndef SPECIES_H
#define SPECIES_H

#include <vector>
#include <string>
#include <map>
#include<iostream>
#include <fstream>
#include<iomanip>

#include "SpeciesManager.h"
#include "ChemicalModel.h"

using namespace std;

namespace Genex6
{
class SimulatorStateBase;
class SpeciesProperties;
class Element;

class Species
{
public:
   Species();
   ~Species();
   Species(const std::string &in_Name, const int in_id, ChemicalModel *const in_theModel);
   

   void ComputeTimeStep(SimulatorStateBase &theSimulatorState,                           
                        const double in_dT,
                        const double s_Peff,
                        const double s_TK,
                        const double s_FrequencyFactor,
                        const double s_kerogenTransformationRatio,
                        const double s_precokeTransformationRatio,
                        const double s_coke2TransformationRatio,
                        const double s_DiffusionConcDependence,
                        const double s_VogelFulcherTemperature,
                        const bool in_OpenSourceRockConditions);
   void SetProperties(SpeciesProperties *const in_Props);

   //structure formation
   void UpdateCompositionByElement(Element *const theElem,const double inComposition);
   void UpdateMassFactorBySpeciesName(const int SpeciesId, const double Factor);
   void UpdateDiffusionEnergy1(const double in_diffEnergy);

   void SetPositiveGenRate(const double in_positiveGenRate);
   double GetPositiveGenRate() const;
   void UpdatePositiveGenRate(const double in_positiveGenRate);
   void AddPositiveGenRate(const double generationRateToAdd);
   bool validate();

   double GetMassFactorBySpecies(const int productId) const ;
   const double * GetProductMassFactors() const;
   void setMassFactorBySpecies(const int productId, const double in_factor);

   const std::string& GetName() const;
   int GetId() const;
   int GetNumberOfElements();
   double GetCompositionByElement(const int elemName) const;
   const std::string& GetCompositionCode() const;
   SpeciesProperties *GetSpeciesProperties();
   int GetCompositionCodeLength() const;
   int * GetCompositionCodeIds();

   bool isGX5() const;
   bool isGenex() const;

   void OutputCompositionOnScreen();
   void OutputPropertiesOnScreen() const;

   void OutputCompositionOnFile(ofstream &outfile);
   void OutputMassFactorsOnFile(ofstream &outfile) const;
   void OutputMassFactorsOnScreen() const;
   void OutputPropertiesOnFile(ofstream &outfile) const;
   void OutputOnFile(ofstream &outfile) const;
   void PrintBenchmarkProperties(ofstream &outfile) const;
   void PrintBenchmarkStoichiometry(ofstream &outfile) const;
   void OutputResults(const bool value);
   bool IsSpeciesOutputRequired() const;
   //void LoadStoichiometry(ifstream &infile);


   //General, properties computation ,
   //The Species class provides this computational functionality, probably should change to 
   //function objects in order to increase flexibility.
   //Results stored in the  object m_theProps
   //class SpeciesPropertiesprovides the Get-Set  interface
   double ComputeMolWeight() const;   //called every time there is an update in the Composition,
                                      //which happens often in preprocessing
   double ComputeAromaticity() const;
   double ComputeReactionOrder() const;
   void   ComputeB0() const;
   double ComputeDensity() const;     //necessary only in the case of immobile species

   double GetMolWeight() const;
   double GetDensity() const;
   double GetAromaticity() const;
   double GetTheta() const;

   void UpdateProperties();          //Calls SpeciesProperties::Update(), 
   void UpdatePositiveGenerationRatesOfDaughters(const double NegativeGenerationRate);

   void SetApproximateFlag(const bool in_approximateFlag); 
   //utilities
   double ComputeHCCorrector() const;//HC correction according to Van Krevelen

   double FunDiffusivityHybrid(const double s_FrequencyFactor, const double s_Peff, 
                               const double s_TK, const double s_VogelFulcherTemperature);

   double ComputeArrheniusReactionRate2a(const SimulatorStateBase &theSimulatorState, 
                                         const double s_FrequencyFactor, 
                                         const double s_Peff, 
                                         const double s_TK,  
                                         const double s_VogelFulcherTemperature,
                                         const double s_kerogenTransformationRatio, 
                                         const double s_precokeTransformationRatio, 
                                         const double s_coke2TransformationRatio);

   /// \brief Return constant reference to chemical-model.
   const ChemicalModel& getChemicalModel () const;

   /// \brief return const reference to species-manager.
   const SpeciesManager& getSpeciesManager () const;
 
private:       
   std::string m_name;
   int m_id;
   double m_theta;
   bool m_outputResults;
   bool m_approximate;
   //Composition
   std::string m_compositionCode;    //e.g species which consist of C, O, N have a compositionCode CON
   
   // array of element's Ids correspondent to compositionCode
   int m_compositionCodeIds [Genex6::SpeciesManager::numberOfElements]; 
   int m_compositionCodeLength;  // number of elements in composition

   Element* m_theElements[Genex6::SpeciesManager::numberOfElements];        //Elements that constitute each species
   double m_compositionByElement[Genex6::SpeciesManager::numberOfElements]; //Composition factor by element id

   // SFmass. Computed by  Reaction::ComputeMassProductRatios()  
   double m_massFactorsBySpecies[Genex6::SpeciesManager::numberOfSpecies]; 

   //physical, chemical properties
   SpeciesProperties *m_theProps; //species properties

   ChemicalModel *const m_theChemicalModel;
   
   double m_positiveGenRate;//cross link between the species reactions

   void ComputeMassTransportCoeff(const double s_Peff,
                                  const double s_TK,
                                  const double s_FrequencyFactor,
                                  const double s_DiffusionConcDependence,
                                  const double s_VogelFulcherTemperature,
                                  const bool in_OpenSourceRockConditions);
};

inline SpeciesProperties *Species::GetSpeciesProperties()
{
   return m_theProps;
}
inline int Species::GetNumberOfElements()
{
   return  m_compositionCodeLength; 
}
inline void Species::SetProperties(SpeciesProperties *const in_Props)
{
   m_theProps = in_Props;
}
inline void Species::SetPositiveGenRate(const double in_positiveGenRate)
{
   m_positiveGenRate = in_positiveGenRate;
}
inline double Species::GetPositiveGenRate() const
{
  return  m_positiveGenRate;
}
inline void Species::UpdatePositiveGenRate(const double in_positiveGenRate)
{
  m_positiveGenRate -= in_positiveGenRate;
}
inline const std::string& Species::GetName() const
{
   return m_name;
}
inline int Species::GetId() const
{
   return m_id;
}
inline double Species::GetTheta() const
{
   return m_theta;
}
inline void Species::OutputResults(const bool value)
{
  m_outputResults = value; 
}
inline bool Species::IsSpeciesOutputRequired()const
{
   return m_outputResults;
}
inline const std::string& Species::GetCompositionCode()const 
{
   return m_compositionCode;
}
inline int Species::GetCompositionCodeLength() const 
{ 
   return m_compositionCodeLength;
}
inline int * Species::GetCompositionCodeIds() 
{ 
   return m_compositionCodeIds;
}
inline void Species::SetApproximateFlag(const bool in_approximateFlag) 
{
   m_approximate = in_approximateFlag;
}
inline bool Species::isGenex() const
{
   return m_theChemicalModel->isGenex();
}

}

inline const Genex6::ChemicalModel& Genex6::Species::getChemicalModel () const {
   return *m_theChemicalModel;
}

inline const Genex6::SpeciesManager& Genex6::Species::getSpeciesManager () const {
   return m_theChemicalModel->getSpeciesManager ();
}

#endif

