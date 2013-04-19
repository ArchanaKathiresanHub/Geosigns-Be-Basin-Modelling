#ifndef SIMULATOR_H
#define SIMULATOR_H


#include <string>
#include <vector>
#include<iostream>
#include <fstream>
#include<iomanip>
using namespace std;

/*! \mainpage genex5_kernel library
 * \section intro Introduction
 * The genex5_kernel library is a library that provides the core computational functionality for the simulation of GENeration and 
 * EXpulsion in source rocks.  
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

#include "AdsorptionSimulator.h"

namespace Genex5
{
class ChemicalModel;
class SourceRockNodeInput;
class SourceRockNodeOutput;
class SimulatorState;

//!Provides the main computational functionality for the generation and expulsion simulation.
/*!
* Each Simulator object is associated with a particular source rock type and with a corresponding chemical model. Once a Simulator object has been constructed, its main public interfaces ComputeSourceRockNodeTimeInstance and InitializeSourceRockNode can be used for the computation of a particular SourceRockNode.
*/
class Simulator
{
public:

   //energies in Joule
   Simulator(const std::string &in_fullPathConfigurationFileName,
             const std::string &in_type="TypeI",
             const double &in_HC = 1.56,
             const double &in_Emean = 216000.0,
             const double &in_VRE = 0.5,
             const double &in_asphalteneDiffusionEnergy = 88000.0,
             const double &in_resinDiffusionEnergy = 85000.0,
             const double &in_C15AroDiffusionEnergy = 80000.0,
             const double &in_C15SatDiffusionEnergy = 75000.0);

   //Unit Test, sch file format support
   Simulator(const std::string &in_fullPathConfigurationFileName,
             const double &in_HC,
             const double &in_Emean, 
             bool PreprocessChemicalModel);
  
   virtual ~Simulator();

   bool Validate()const;

   //!Returns the maximum admissible time step size 
   /*!
      \param depositionTime the deposition time of a source rock
      Returns the time step size that should be used in a simulation given the corresponding declarations in the configuration file
   */
   double GetMaximumTimeStepSize(const double &depositionTime) const;
   
   //!Computes source rock node for any time instance
   /*!
         \param theInput a valid SourceRockNodeInput that corresponds to the end of the interval 
         \param NodeSimulatorState a valid initial SimulatorState
         \param theOutput a SourceRockNodeOutput to store the derived result quantities
         \param thickness the thickness of the SourceRockNode
         \param ConcKi the organic density 

         Essentially computes a single interval between t1 and t2 and the output results at t2. The Simulator class
         interfaces with clent code through the SimulatorState object which on entry refers to t1.
         ComputSourceRockNodeTimeInstance advances the state from its original time instance t1(accesible through
         SimulatorState::GetReferenceTime ) to the final time instance t2 which is defined by the reference time of
         the SourceRockNodeInput object(accessible through SourceRockNodeInput::GetCurrentTime). It also updates the
         SourceRockNodeoutput object with the results corresponding to the end reference time t2. 

         On exit SimulatorState object contains the absolutely necessary data so that its state and a pair of
         SourceRockNodeInput and SourceRockNodeOutput can be used to perform a computation for a subsequent time instance. 

         The timestep size control is given to the caller and within the algorithm is always t2 - t1. Nevertheless it is
         advisable that the actual timestep size that is employed in any simulation is taken from the function
         Simulator::GetMaximumTimeStepSize 

         
  
   */
   void ComputeSourceRockNodeTimeInstance(const SourceRockNodeInput &theInput,
                                          SimulatorState *const NodeSimulatorState,
                                          SourceRockNodeOutput &theOutput,
                                          const double &thickness,
                                          const double &ConcKi);
   
 //!Initializes source rock node, computes output for the starting time instance 
   /*!
         \param theInput a valid SourceRockNodeInput that corresponds to the initialization time instance  
         \param NodeSimulatorState a reference to a pointer of type SimulatorState
         \param theOutput a SourceRockNodeOutput to store the derived result quantities
         \param thickness the thickness of the SourceRockNode a SourceRockNodeOutput to store the derived result quantities
         \param TOC the Total Organic Carbon of the SourceRockNode
         \param InorganicDensity the inorganic density of the SourceRockNode
         \param ConcKi a reference to the organic density

         The initialization of a source rock node essentially implies an initiliazation of the corresponding SimulatorState,
         a computation of the organic density and an update of the initial output for the very first time instance.

         InitializeSourceRockNode expects the incoming NodeSimulatorState to point to 0 and in principle it works as a source
         function by constructing the initial SimulatorState and returning the ownership back to the caller(SourceRockNode). . 

         On exit SimulatorState object contains the absolutely necessary data so that its state and a pair of
         SourceRockNodeInput and SourceRockNodeOutput can be used to perform a computation for a subsequent time instance.
         theOutput contains all the necessary output data at deposition age.  
  
   */
   void InitializeSourceRockNode(const SourceRockNodeInput &theInput,
                                 SimulatorState * &NodeSimulatorState,
                                 SourceRockNodeOutput &theOutput,
                                 const double &thickness,
                                 const double &TOC,
                                 const double &InorganicDensity,
                                 const double &sgMeanBulkDensity,
                                 double &ConcKi);
  

   double GetHCFinal()const;
   double GetOCFinal()const;
   double GetNCFinal()const;

   /// Set the adsorption-simulator.
   ///
   /// The Simulator object takes control of the adsorption-simulator. It is deleted in this class.
   void setAdsorptionSimulator ( AdsorptionSimulator* adsorptionSimulator );
  
   const AdsorptionSimulator* getAdsorptionSimulator () const;


   void PrintBenchmarkOutput(ofstream &outputTestingSetFile);
   void PrintConfigurationFile(const std::string &FullPathConfigurationFileName);

protected:  
   void PreprocessTimeStepComputation(const SourceRockNodeInput &theSourceRockInput);

   void ProcessTimeStepComputation ( const SourceRockNodeInput&  theInput,
                                           SourceRockNodeOutput& theOutput,
                                     const double &thickness,
                                     const double &ConcKi);

   /// Compute the toc after time-step.
   void computeToc ( const SourceRockNodeInput &theInput );

   //-------------computational functionality-------------------------------
   double ComputeTimestepSize(const SourceRockNodeInput &theInput)const;

   double ComputeVogelFulcherTemperature(const double &in_Waso);
   double ComputeDiffusionConcDependence(const double &in_Waso);
   double ComputeKerogenTransformatioRatio();

   void ComputeFirstTimeInstance(SourceRockNodeOutput &theOutput, const double &thickness);

   double ComputeNodeInitialOrganicMatterDensity(const double &TOC, const double &InorganicDensity);

   void CreateInstance();

   void Preprocess(const double &in_Emean,
                   const double &in_VRE,
                   const double &in_asphalteneDiffusionEnergy,
                   const double &in_resinDiffusionEnergy,
                   const double &in_C15AroDiffusionEnergy,
                   const double &in_C15SatDiffusionEnergy);

   // Also set the immobile species densities.
   void LoadDataFromConfigurationFile();
   void LoadSimulatorProperties(ifstream &ConfigurationFile);
   void LoadGeneralParameters(ifstream &ConfigurationFile); 

   void CheckInitialHC(const double &in_VRE);
   void TransformHC(const double &in_VRE);
   
   
private:
   std::string m_fullPathToConfigurationFileDirectory;
   std::string m_type;
   double m_HC;
   
   ChemicalModel   *m_theChemicalModel;
   SimulatorState  *m_currentState;          

   AdsorptionSimulator* m_adsorptionSimulator;

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
   static double s_kerogenTransformationRatio; 
   static double s_Waso;
   static double s_DiffusionConcDependence;
   static double s_VogelFulcherTemperature;

   static double s_lithostaticPressure;
   static double s_hydrostaticPressure;
   static double s_porePressure;
   static double s_porosity;


   static std::string s_cfgFileExtension; 

   //Non-copyable
   Simulator &operator=(const Simulator &);
   //Non-copyable
   Simulator (const Simulator &);
   

};

}
#endif
