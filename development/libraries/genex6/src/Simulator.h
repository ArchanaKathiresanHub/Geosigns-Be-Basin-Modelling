#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <iomanip>

/*! \mainpage genex6_kernel library
 * \section intro Introduction
 * The genex6_kernel library is a library that provides the core computational functionality for 
 * the simulation Oil To Gas Cracking in reservoir formations 
*/

#include "SubProcessSimulator.h"
#include "ChemicalModel.h"
      
namespace Genex6
{
class Input;
class SimulatorStateBase;
class Species;

//! Simulator is the main controlling class 

class Simulator 
{
public:
   //! Constructor
   Simulator();
      /*!
         \param in_fullPathConfigurationFileName
         \param in_type the name of the file   
         \param in_HC the H/C ratio for preasphaltene 
         \param in_Emean the mean activation energy for preasphaltene 
      */
   Simulator(const std::string in_fullPathConfigurationFileName,
             const int in_Simulationtype,
             const std::string in_type = "TypeII",
             const double in_HC = 1.15,
             const double in_SC = 0.0,
             const double in_Emean = 210000.0,
             const double in_VRE = 0.5,
             const double in_asphalteneDiffusionEnergy = 88000.0,
             const double in_resinDiffusionEnergy = 85000.0,
             const double in_C15AroDiffusionEnergy = 80000.0,
             const double in_C15SatDiffusionEnergy = 75000.0,
             const bool inApproximateFlag = true);

   //Unit Test, sch file format support
   Simulator(const std::string in_fullPathConfigurationFileName,
             const int in_Simulationtype,
             const double in_HC,
             const double in_SC,
             const double in_Emean, 
             bool PreprocessChemicalModel,
             const bool inApproximateFlag);
  
   virtual ~Simulator();

   /// load ChemicalModel from ConfigFile and Preprocess
   ChemicalModel * loadChemicalModel(const std::string in_fullPathToConfigurationFileDirectory,
                                     const int in_simulationType,
                                     const std::string in_type,
                                     const double in_HC,
                                     const double in_SC,
                                     const double in_Emean,
                                     const double in_VRE,
                                     const double in_asphalteneDiffusionEnergy,
                                     const double in_resinDiffusionEnergy,
                                     const double in_C15AroDiffusionEnergy,
                                     const double in_C15SatDiffusionEnergy,
                                     const bool inApproximateFlag = true);
   /// \brief Add a sub-process simulator to the list of sub-process simulators.
   void addSubProcess ( SubProcessSimulator* subProcess );

   bool Validate()const;

   void setChemicalModel( ChemicalModel * inModel );
   void SetSimulatorState( SimulatorStateBase * theState );
   SimulatorStateBase* GetSimulatorState() const;

   double getHC() const;


   //!Returns the maximum admissible time step size 
   /*!
      \param depositionTime the deposition time of a source rock
      Returns the time step size that should be used in a simulation given the corresponding declarations in the configuration file
   */
   double GetMaximumTimeStepSize(const double depositionTime) const;
   double getMaximumTimeStepSize() const;
   void   setMaximumTimeStepSize( const double aMaxTimestep );

   int    getNumberOfTimesteps() const;
   void   setNumberOfTimesteps( const int aNumberOfTimesteps );

   int GetSpeciesIdByName( const std::string & name );
   const Species ** getSpeciesInChemicalModel(); 

   /// \brief return const reference to species-manager.
   const SpeciesManager& getSpeciesManager () const;

   /// \brief return const reference to chemical-model.
   const ChemicalModel& getChemicalModel () const;

   void advanceSimulatorState(const Input &theInput);

   void initializeSimulatorState( const Input &theInput );
  
   void PrintBenchmarkOutput(std::ofstream &outputTestingSetFile) const;
   void PrintConfigurationFile(const std::string &FullPathConfigurationFileName, const bool PreprocessChemicalModel);

   double ComputeNodeInitialOrganicMatterDensity(const double TOC, const double InorganicDensity); // Genex
   // Fastmig interface
   void computeInterval(SimulatorStateBase &theState,
                        const double tempStart, const double tempEnd,
                        const double pressureStart, const double pressureEnd,
                        const double timeStart, const double timeEnd);


protected:

   void PreprocessTimeStepComputation(const Input &theSourceRockInput);
   void ProcessTimeStepComputation ();

   /// Compute the toc after time-step.
   void computeToc ( const Input &theInput );

   //-------------computational functionality-------------------------------
   double ComputeTimestepSize(const Input &theInput)const;

   double ComputeVogelFulcherTemperature(const double in_Waso);
   void ComputePrecokeTransformatioRatio();
   void ComputeCoke2TransformatioRatio();
   void CreateInstance();
 
   void Preprocess(const double in_SC,
                   const double in_HC,
                   const double in_Emean,
                   const double in_VRE,
                   const double in_asphalteneDiffusionEnergy,
                   const double in_resinDiffusionEnergy,
                   const double in_C15AroDiffusionEnergy,
                   const double in_C15SatDiffusionEnergy);
 	
   void LoadDataFromConfigurationFile();
   void LoadSimulatorProperties(std::ifstream &ConfigurationFile);
   void LoadGeneralParameters(std::ifstream &ConfigurationFile);

   double CheckInitialHC(const double in_VRE, const double in_HC); //Genex
   double TransformHC(const double in_VRE, const double in_HC);//Genex

private:
   int m_simulationType;

   std::string m_fullPathToConfigurationFileDirectory;
   std::string m_type;
   
   ChemicalModel   *m_theChemicalModel;
   SimulatorStateBase  *m_currentState;

   //Simulator Boundary Conditions from configuration file
   bool m_preProcessSpeciesKinetics;
   bool m_preProcessSpeciesComposition;
   bool m_useDefaultGeneralParameters;
   int    m_numberOfTimesteps;
   double m_maximumTimeStepSize;
   bool   m_openConditions;
   double m_massBalancePercentTolerance;

   //Computation state variables 
   double s_dT;   
   double s_Peff; 
   double s_TK;
   double s_FrequencyFactor; 
   double s_kerogenTransformationRatio; 
   double s_Waso;
   double s_DiffusionConcDependence;
   double s_VogelFulcherTemperature;

   std::string s_cfgFileExtension; 

   void SetApproximateFlag(const bool in_approximateFlag);

   //   SubProcessSimulatorList m_subProcesses;

};
   //! Interface providing the core computational functionality for simulating oil to gas cracking for a given time interval,
      /*!
         \param theState an object of type SimulatorState. 
         \param tempStart temperature at the start of the interval  
         \param tempEnd temperature at the end of the interval  
         \param pressureStart pressure at the start of the interval  
         \param pressureEnd pressure at the end of the interval  
         \param timeStart time at the start of the interval  
         \param timeEnd time at the end of the interval 
         
         The main task of the function is to advance the incoming state from timeStart to time End if the SimulatorState object is initialized. If it is not initialized the Simulator initializes it first and then proceeds with advancing until the end of the interval.
 
         The convention for the timeStart and timeEnd used in Cauldron is the following: timeStart > timeEnd.

         The timestep size for the computation is the the maximum between the following two cases:

         1. (timeStart - timeEnd)   

         2. MaximumTimeStepSize as defined in $OTGCDIR/TypeII.cfg
      
      */
   //computeInterval


}

inline const Genex6::ChemicalModel& Genex6::Simulator::getChemicalModel () const {
   return *m_theChemicalModel;
}

inline const Genex6::SpeciesManager& Genex6::Simulator::getSpeciesManager () const {
   return m_theChemicalModel->getSpeciesManager ();
}

inline double Genex6::Simulator::getHC () const {
   return m_theChemicalModel->getHC();
}

inline void Genex6::Simulator::setChemicalModel ( ChemicalModel * inModel) {
   m_theChemicalModel = inModel;
   m_simulationType = inModel->GetSimulationType();
}

inline double Genex6::Simulator::getMaximumTimeStepSize() const {
   return m_maximumTimeStepSize;
}

inline void Genex6::Simulator::setMaximumTimeStepSize( const double aMaxTimeStepSize ) {
  m_maximumTimeStepSize = aMaxTimeStepSize;
}

inline int Genex6::Simulator::getNumberOfTimesteps() const {
   return m_numberOfTimesteps;
}

inline void Genex6::Simulator::setNumberOfTimesteps( const int aNumberOfTimeSteps ) {
   m_numberOfTimesteps = aNumberOfTimeSteps;
}

#endif
