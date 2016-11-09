//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

#ifndef GENEX6__CHEMICALMODEL_H
#define GENEX6__CHEMICALMODEL_H

#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <iomanip>

#include "SpeciesManager.h"
#include "ConstantsGenex.h"
#include "ImmobileSpecies.h"

namespace Genex6
{

using namespace std;

class Element;
class Species;
class Reaction;
class SimulatorStateBase;

class ChemicalModel
{
public:
   ChemicalModel(const int in_simulationType);
   ChemicalModel(const std::string &in_FulPathConfigurationFileName, const int in_simulationType); 
   
   virtual ~ChemicalModel();
   
   void UpdateSpeciesCompositionsByElementName(const int in_Species,
                                               const int in_Element,
                                               const double in_CompositionFactor); 
   void UpdateSpeciesDiffusionEnergy1(const int SpeciesName,const double diffEnergy1);//public
   
   bool Validate() const;

   void SetSCratio(const double in_SC);
   void SetHCratio(const double in_HC);

   Species *GetByNameSpecies(const std::string & in_name) const;
   Species *GetSpeciesById(const int in_Id) const;
   const Species** GetSpecies() const;

   int  GetSimulationType() const;
   bool isGX5() const;
   bool isOTGC5() const;
   bool isSim5() const;
   bool isGenex() const;
   bool isTSR() const;

   double GetElemAtomWeightByName(const int ElemName) const;
   double GetSpeciesMolWeightByName(const int SpeciesName) const;
   double GetSpeciesDensityByName(const int SpeciesName) const; 
   double GetSpeciesCompositionByElement(const int SpeciesName, const int ElementName); 
   double GetSpeciesB0ByName(const int SpeciesId) const; 
   const ImmobileSpecies & getImmobileSpecies() const;

   void SetSpeciesReactionOrder();
   void setImmobileDenisty( ImmobileSpecies::SpeciesId anId, double aDensity );
   void ComputeSpeciesUltimateMasses(SimulatorStateBase *theState);
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
                        const bool in_OpenSourceRockConditions );
    
//!Preprocess chemical composition of species.
/*!
* Changes the chemical composition of a sequence of species. It is invoked by setting the configuration file parameter 
*/
   void InitializeSpeciesTimeStepVariables();
   void CompEarlySpecies(); 
   void KineticsEarlySpecies(const double Emean);
   void ComputeStoichiometry();
   void UpdateSpeciesProperties();
   void ComputePseudoActEnergyRadical();
   void ComputeB0();

   int GetNumberOfElements() const;
   int GetNumberOfSpecies() const;
   int GetNumberOfReactions() const;

   double getHC() const;

   // static int GetNumberOfElements();
   // static int GetNumberOfSpecies();
   // static int GetNumberOfReactions();  

   //only for printing out results according to the VBA format
   void AddPairInSpeciesId2Name(const int id, const std::string &name);
   // static void AddPairInSpeciesId2Name(const int id, const std::string &name);

   //only for printing out results according to the VBA format
   std::string GetSpeciesNameById(const int id) const;
   // static std::string GetSpeciesNameById(const int id);

   int GetSpeciesIdByName(const string &name) const;//for OTGC interface
   // static int GetSpeciesIdByName(const string &name);//for OTGC interface
   
   //Output Functionality for Benchmarking
   void PrintBenchmarkSpeciesPropertiesHeader(ofstream &outfile) const;
   void PrintBenchmarkSpeciesProperties(ofstream &outfile) const;
   void PrintBenchmarkStoichiometryHeader(ofstream &outfile) const;
   void PrintBenchmarkStoichiometry(ofstream &outfile) const;
   //Testing method
   //void ChemicalModel::LoadBenchmarkStoichiometry(string & namefile);

   //New Configuration File I/O
   void LoadElements(ifstream &ConfigurationFile);
   void LoadSpecies(ifstream &ConfigurationFile);
   void LoadSpeciesComposition(ifstream &ConfigurationFile);
   void LoadSpeciesProperties(ifstream &ConfigurationFile);
   void LoadReactions(ifstream &ConfigurationFile);
   void LoadReactionRatios(ifstream &ConfigurationFile);
   void PrintConfigurationFileEntities(ofstream &outfile);
   void DebugOutput();

   /// \brief Return const reference to species-manager.
   const SpeciesManager& getSpeciesManager () const;

protected:
   void ComputeProductRatios(const double preasphalteneAromaticity);
   void ComputeMassProductRatios();

   void clearElements();
   void clearSpecies();
   void clearReactions();

   void SetTheElements(ifstream &theStream); 
   void SetTheSpecies(ifstream &theStream);  
   void SetTheReactions(ifstream &theStream);
   void AddElement(Element *theElement);
   void AddSpecies(Species *theSpecies);
   void AddReaction(Reaction *theReaction);
   Element *GetElementByName(const int in_name) const;
   Reaction *GetReactionByMotherName(const int theMotherId) const;
   void SetTheOutputSpecies();
   
   void setSpeciesNum( string speciesName, int index );

private:
   int m_simulationType; // type of simulation (OTGC, SourceRock, Genex5-like simulation)

   double m_SC;   // S/C ratio
   double m_HC;   // H/C initial ratio

   std::string m_name;    //Stoichiometry "name", value from sch file
   // Element container
   std::vector<Element*> m_theElements; 
   // Species Container  
   Species * m_theSpecies[Genex6::SpeciesManager::numberOfSpecies];
 
   // Reactions Container              
   std::map<int,Reaction*> m_theReactions;  //Reactions by mother name, values from sch file

   // query support 
   std::map<std::string, int> s_mapSpeciesId2Name;     //need for benchmarking output

   int s_numberOfElements;   //no need for this in the production code,only for the C++ prototype
   int s_numberOfSpecies;    //no need for this in the production code,only for the C++ prototype
   int s_numberOfReactions;  //no need for this in the production code,only for the C++ prototype

   SpeciesManager  m_speciesManager;
   ImmobileSpecies m_immobileSpecies;

};

inline const ImmobileSpecies & ChemicalModel::getImmobileSpecies() const
{
   return m_immobileSpecies;
}

inline void ChemicalModel::setImmobileDenisty( ImmobileSpecies::SpeciesId anId, double aDensity )
{
   m_immobileSpecies.setDensity ( anId, aDensity );
}

inline Species *ChemicalModel::GetSpeciesById(const int in_Id) const
{
   return m_theSpecies[in_Id - 1];
}

inline const Genex6::SpeciesManager& Genex6::ChemicalModel::getSpeciesManager () const {
   return m_speciesManager;
}
inline void Genex6::ChemicalModel::SetSCratio(const double in_SC) 
{
   m_SC = in_SC;
}
inline void Genex6::ChemicalModel::SetHCratio(const double in_HC) 
{
   m_HC = in_HC;
}
inline int Genex6::ChemicalModel::GetSimulationType() const
{
   return m_simulationType;
}
inline bool Genex6::ChemicalModel::isGX5() const
{
   return (m_simulationType & Genex6::Constants::SIMGENEX5) ? true : false;
}
inline bool Genex6::ChemicalModel::isOTGC5() const
{
   return (m_simulationType & Genex6::Constants::SIMOTGC5) ? true : false;
}

inline bool Genex6::ChemicalModel::isTSR() const
{
   return (m_simulationType & Genex6::Constants::SIMTSR) ? true : false;
}

inline bool Genex6::ChemicalModel::isSim5() const
{
   return isGX5() || isOTGC5();
}
inline bool Genex6::ChemicalModel::isGenex() const 
{
   return (m_simulationType & Genex6::Constants::SIMGENEX) ? true : false;
}
inline double Genex6::ChemicalModel::getHC () const {
   return m_HC;
}

}
#endif // _GENEX6__CHEMICALMODEL_H_
