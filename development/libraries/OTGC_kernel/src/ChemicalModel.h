#ifndef CHEMICALMODEL_H
#define CHEMICALMODEL_H



#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <iomanip>
using namespace std;

namespace OTGC
{
class Element;
class Species;
class Reaction;
class SimulatorState;
class ChemicalModel
{
public:
   ChemicalModel();
   ChemicalModel(const std::string &in_FulPathConfigurationFileName); 
   
   virtual ~ChemicalModel();
   
   void UpdateSpeciesCompositionsByElementName(const std::string &in_Species,const std::string &in_Element, const double &in_CompositionFactor); 
   void UpdateSpeciesDiffusionEnergy1(const std::string &SpeciesName,const double &diffEnergy1);//public
   
   bool Validate() const;

   void ComputeSpeciesUltimateMasses(SimulatorState &theState);
   Species *GetSpeciesByName(const std::string &in_name) const;
   Species *GetSpeciesById(const int &in_Id) const;

   double GetElemAtomWeightByName(const std::string &ElemName) const;
   double GetSpeciesMolWeightByName(const std::string &SpeciesName) const;
   double GetSpeciesDensityByName(const std::string &SpeciesName) const; 
   double GetSpeciesCompositionByElement(const std::string &SpeciesName, const std::string &ElementName); 
   const vector<std::string> & getSpeciesNames() const;


   void ComputeSpeciesUltimateMasses(SimulatorState &theState, const std::map<std::string,double>  & initialSpeciesConcentrations);

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
                        const bool &in_OpenSourceRockConditions );
public:
   ///Preprocessing
   void CompEarlySpecies(); 
   void KineticsEarlySpecies(const double &Emean);
   void ComputeStoichiometry();
   void UpdateSpeciesProperties();
   static int GetNumberOfElements();
   static int GetNumberOfSpecies();
   static int GetNumberOfReactions();  
   static void AddPairInSpeciesId2Name(const int &id, const std::string &name);//only for printing out results according to the VBA format
   static std::string GetSpeciesNameById(const int &id);//only for printing out results according to the VBA format
   
   //Output Functionality for Benchmarking
   void PrintBenchmarkSpeciesPropertiesHeader(ofstream &outfile) const;
   void PrintBenchmarkSpeciesProperties(ofstream &outfile) const;
   void PrintBenchmarkStoichiometryHeader(ofstream &outfile) const;
   void PrintBenchmarkStoichiometry(ofstream &outfile) const;

   //New Configuration File I/O
   void LoadElements(ifstream &ConfigurationFile);
   void LoadSpecies(ifstream &ConfigurationFile);
   void LoadSpeciesComposition(ifstream &ConfigurationFile);
   void LoadSpeciesProperties(ifstream &ConfigurationFile);
   void LoadReactions(ifstream &ConfigurationFile);
   void LoadReactionRatios(ifstream &ConfigurationFile);
   void PrintConfigurationFileEntities(ofstream &outfile);

   //OTGC
   void SetSpeciesReactionOrder();
   //OTGC
   
protected:
   void ComputeProductRatios(const double &preasphalteneAromaticity);
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
   Element *GetElementByName(const std::string &in_name) const;
   Reaction *GetReactionByMotherName(const std::string &theMotherName) const;//protected
   void SetTheOutputSpecies();//protected
   void InitializeSpeciesTimeStepVariables();
   void UpdateSpeciesPositiveGenRateByName(const std::string &SpeciesName, const double &in_GenRateToSubract);
   
private:
   std::string m_name;                                                               //Stoichiometry "name", value from sch file

   ///Element container
   std::map<std::string,Element*>   m_theElements;
   ///Species Container          
   std::vector<Species*>   m_theSpecies;
   ///Reactions Container              
   std::map<std::string,Reaction*>  m_theReactions;            //Reactions by mother name, values from sch file
   std::map<std::string, double> m_UltimateMassesBySpeciesName;

   /// query support 
   std::map<std::string,std::vector<Species*>::size_type >   m_mapSpeciesName2Index;               //Species by name,
   std::map<int, std::vector<Species*>::size_type >   m_mapSpeciesId2Index; 

   static std::map<int,std::string> s_mapSpeciesId2Name;     //need for benchmarking output
   static int s_numberOfElements;   //no need for this in the production code,only for the C++ prototype
   static int s_numberOfSpecies;    //no need for this in the production code,only for the C++ prototype
   static int s_numberOfReactions;  //no need for this in the production code,only for the C++ prototype

   ChemicalModel(const ChemicalModel &);
   ChemicalModel & operator=(const ChemicalModel &);
};


}
#endif  //CHEMICALMODEL_H
