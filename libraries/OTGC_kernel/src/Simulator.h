#ifndef _OTGC_SIMULATOR_H
#define _OTGC_SIMULATOR_H


#include <string>
#include <vector>
#include <map>
#include<iostream>
#include <fstream>
#include<iomanip>
using namespace std;

/*! \mainpage OTGC_kernel library
 * \section intro Introduction
 * The OTGC_kernel library is a library that provides the core computational functionality for the simulation Oil To Gas Cracking in reservoir formations 
 *   
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 */

   
  
      
namespace OTGC
{
class ChemicalModel;
class Input;
class SimulatorState;

//! Simulator is the main controlling class 
/*! 
   Provides the core computational functionality for OTGC to client applications through the public interface computeInterval()
*/
class Simulator
{
public:
   //! Constructor
      /*!
         \param in_fullPathConfigurationFileName
         \param in_type the name of the file   
         \param in_HC the H/C ratio for preasphaltene 
         \param in_Emean the mean activation energy for preasphaltene 
      */
   Simulator(const std::string &in_fullPathConfigurationFileName,
             const std::string &in_type = "TypeII",
             const double &in_HC = 1.15,
             const double &in_Emean = 210000.0,
             const double &in_VRE = 0.5,
             const double &in_asphalteneDiffusionEnergy = 88000.0,
             const double &in_resinDiffusionEnergy = 85000.0,
             const double &in_C15AroDiffusionEnergy = 80000.0,
             const double &in_C15SatDiffusionEnergy = 75000.0);//energies in Joule
   
   virtual ~Simulator();

   bool Validate()const;
   //! Interface providing the core computational functionality for simulating oil to gas cracking for a given time interval,
      /*!
         \param theState an object of type SimulatorState. 
         \param tempStart temperature at the start of the interval  
         \param tempEnd temperature at the end of the interval  
         \param pressureStart pressure at the start of the interval  
         \param pressureEnd pressure at the end of the interval  
         \param timeStart time at the start of the interval  
         \param timeEnd time at the end of the interval 
         
         The main task of the function is to advance the incoming state from timeStart to time End if the SimulatorState object is initialized. If it is not            initialized the Simulator initializes it first and then proceeds with advancing until the end of the interval.
 
         The convention for the timeStart and timeEnd used in Cauldron is the following: timeStart > timeEnd.

         The timestep size for the computation is the the maximum between the following two cases:

         1. (timeStart - timeEnd)   

         2. MaximumTimeStepSize as defined in $OTGCDIR/TypeII.cfg
      
      */
   void computeInterval(SimulatorState &theState,
							  const double &tempStart, const double &tempEnd,
							  const double &pressureStart, const double &pressureEnd,
							  const double &timeStart, const double &timeEnd
							  );
   const vector<string> & getSpeciesInChemicalModel() const;
  
   Simulator(const std::string &in_fullPathConfigurationFileName,
             const double &in_HC,
             const double &in_Emean, 
             bool PreprocessChemicalModel);
   void PrintBenchmarkOutput(ofstream &outputTestingSetFile) const;
   void PrintConfigurationFile(const std::string &FullPathConfigurationFileName);

protected:
   void CreateInstance();
   void Preprocess(const double &in_Emean,
		             const double &in_VRE,
                   const double &in_asphalteneDiffusionEnergy,
                   const double &in_resinDiffusionEnergy,
                   const double &in_C15AroDiffusionEnergy,
                   const double &in_C15SatDiffusionEnergy);
  
	
   void LoadDataFromConfigurationFile();
   void LoadSimulatorProperties(ifstream &ConfigurationFile);
   void LoadGeneralParameters(ifstream &ConfigurationFile); 


   double ComputeVogelFulcherTemperature();
   double ComputeVogelFulcherTemperature(const double &in_Waso);
   double ComputeDiffusionConcDependence(const double &in_Waso);
   void ComputeKerogenTransformatioRatio();
   void ComputePrecokeTransformatioRatio();
   void ComputeCoke2TransformatioRatio();

   double ComputeTimestepSize(const Input &theInput)const;



	//OTGC interface
   void initializeSimulatorState(SimulatorState &theState);
	void initializeState(const Input &theInput, SimulatorState &NodeSimulatorState);
   void advanceState(const Input &theInput, SimulatorState &NodeSimulatorState);
protected:   
   //OTGC2
   void PreprocessTimeStepComputation(const Input &theSourceRockInput);
	void ProcessTimeStepComputation();
   void advanceSimulatorState(const Input &theInput, SimulatorState & currentState);
	//OTGC2

   
private:
   std::string m_fullPathToConfigurationFileDirectory;
   std::string m_type;
   double m_HC;
   
   ChemicalModel   *m_theChemicalModel;
   SimulatorState  *m_currentState;          

   //Simulator Boundary Conditions from configuration file
   bool m_preProcessSpeciesKinetics;
   bool m_preProcessSpeciesComposition;
   bool m_useDefaultGeneralParameters;
   int    m_numberOfTimesteps;
   double m_maximumTimeStepSize;
   bool m_openConditions;
   double m_massBalancePercentTolerance;

   //Computation state variables 
   static double s_dT;   
   static double s_Peff; 
   static double s_TK;
   static double s_FrequencyFactor; 
   static double s_maxkerogenTransformationRatio; 
   static double s_Waso;
   static double s_DiffusionConcDependence;
   static double s_VogelFulcherTemperature;

   static std::string s_cfgFileExtension; 

   Simulator(const Simulator &);
   Simulator &operator=(const Simulator &);

};

}
#endif
