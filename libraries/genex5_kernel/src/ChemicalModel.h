#ifndef CHEMICALMODEL_H
#define CHEMICALMODEL_H


#include <vector>
#include <string>
#include <map>
#include<iostream>
#include <fstream>
#include<iomanip>

namespace Genex5
{

using namespace std;

class Element;
class Species;
class Reaction;
class SimulatorState;
class SourceRockNodeOutput;

 //!Encapsulates the chemical properties of a source rock.
/*!
* ChemicalModel contains a full description of the chemical properties of a particular Source rock which includes the chemical composition, the species characterization and the associated chemical reactions. It's main input is the corresponding configuration file. Provides also low level compuational functionality invoked during simulation of generation and expulsion.
*/
class ChemicalModel
{

public:
   ///Constructor
   ChemicalModel();
   
   virtual ~ChemicalModel();
   
   void UpdateSpeciesCompositionsByElementName(const std::string &in_Species,const std::string &in_Element, const double &in_CompositionFactor); 
   void UpdateSpeciesDiffusionEnergy1(const std::string &SpeciesName,const double &diffEnergy1);
   
   //! Sanity check of the model 
   /*!
    * Check if species declared in CBMGenerics::ComponentManager exist in the ChemicalModel and if the associated stoichiometric factors are positive. 
   */
   bool Validate() const;
   void ComputeFirstTimeInstance(SimulatorState &theState,
                                 SourceRockNodeOutput &theOutput,
                                 const double &thickness);
   void ComputeTimeStep(SimulatorState &theSimulatorState,
                        SourceRockNodeOutput &theOutput,
                        const double &thickness, 
                        const double &ConcKi,
                        const double &in_dT,
                        const double &s_Peff,
                        const double &s_TK,
                        const double &s_FrequencyFactor,
                        const double &s_kerogenTransformationRatio,
                        const double &s_DiffusionConcDependence,
                        const double &s_VogelFulcherTemperature,
                        const bool &in_OpenSourceRockConditions );
   void UpdateSpeciesPositiveGenRateByName(const std::string &SpeciesName, const double &in_GenRateToSubract);
   double GetElemAtomWeightByName(const std::string &ElemName) const;
   double GetSpeciesMolWeightByName(const std::string &SpeciesName) const;
   double GetSpeciesDensityByName(const std::string &SpeciesName) const; 
   double GetSpeciesCompositionByElement(const std::string &SpeciesName, const std::string &ElementName); 
   
   //!Preprocess chemical composition of species.
/*!
* Changes the chemical composition of a sequence of species. It is invoked by setting the configuration file parameter PreprocessSpeciesCompostion
*/
   void CompEarlySpecies(); 
   void KineticsEarlySpecies(const double &Emean);
   void ComputeFactors();
   void UpdateSpeciesProperties();
   static int GetNumberOfElements();
   static int GetNumberOfSpecies();
   static int GetNumberOfReactions();  

   Species *GetSpeciesById(const int &in_Id) const;

   static std::string GetSpeciesNameById(const int &id);//only for printing out results according to the VBA format
   
   //Output Functionality for Benchmarking
   void PrintBenchmarkSpeciesPropertiesHeader(ofstream &outfile);
   void PrintBenchmarkSpeciesProperties(ofstream &outfile);
   void PrintBenchmarkStoichiometryHeader(ofstream &outfile);
   void PrintBenchmarkStoichiometry(ofstream &outfile);

   //New Configuration File I/O
   void LoadElements(ifstream &ConfigurationFile);
   void LoadSpecies(ifstream &ConfigurationFile);
   void LoadSpeciesComposition(ifstream &ConfigurationFile);
   void LoadSpeciesProperties(ifstream &ConfigurationFile);
   void LoadReactions(ifstream &ConfigurationFile);
   void LoadReactionRatios(ifstream &ConfigurationFile);
   void PrintConfigurationFileEntities(ofstream &outfile);
   
   ///Support for sch file format
   ChemicalModel(const std::string &in_FulPathConfigurationFileName_SCH_Format); 

   Species* GetSpeciesByName (const std::string &in_name) const;

protected:

   void AddElement(Element *theElement);
   void AddSpecies(Species *theSpecies);
   void AddReaction(Reaction *theReaction);
   Element *GetElementByName(const std::string &in_name) const;
   Reaction *GetReactionByMotherName(const std::string &theMotherName) const;

   void PostProcessTimeStepComputation(SimulatorState &theSimulatorState, 
                                       SourceRockNodeOutput &theOutput,
                                       const double &thickness);
   void SetTheOutputSpecies();
   void InitializeSpeciesTimeStepVariables();
   void ComputeProductRatios(const double &preasphalteneAromaticity);
   void ComputeMassProductRatios();

   void clearElements();
   void clearSpecies();
   void clearReactions();

   ///Support for sch file format
   void SetTheElements(std::ifstream &theStream);  
   void SetTheSpecies(std::ifstream &theStream);   
   void SetTheReactions(std::ifstream &theStream); 
   
   ///Debug/Unit Test Functionality
   static void AddPairInSpeciesId2Name(const int &id, const std::string &name);//only for printing out results according to the VBA format
   
private:
   std::string m_name;                                                               //Stoichiometry "name", value from sch file 
   ///Element container
   std::map<std::string,Element*>   m_theElements;
   ///Species Container          
   std::vector<Species*>   m_theSpecies;
   ///Reactions Container              
   std::map<std::string,Reaction*>  m_theReactions;            //Reactions by mother name, values from sch file

   /// Data structure to support queries "Get Species By Name" 
   std::map<std::string,std::vector<Species*>::size_type >     m_mapSpeciesName2Index;               //Species by name,
   /// Data structure to support queries "Get Species By Id" 
   std::map<int, std::vector<Species*>::size_type >            m_mapSpeciesId2Index; 

   /// Debug/Unit Test Data members
   static std::map<int,std::string> s_mapSpeciesId2Name; 
   /// Debug/Unit Test Data members    
   static int s_numberOfElements;   
   /// Debug/Unit Test Data members
   static int s_numberOfSpecies;   
   /// Debug/Unit Test Data members 
   static int s_numberOfReactions;  

   ///Non-copyable
   ChemicalModel & operator=(const ChemicalModel &);
   ///Non-copyable
   ChemicalModel(const ChemicalModel &);
};


}
#endif  //CHEMICALMODEL_H
