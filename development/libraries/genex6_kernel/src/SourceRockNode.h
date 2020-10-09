#ifndef SOURCEROCKNODE_H
#define SOURCEROCKNODE_H

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <iomanip>
using namespace std;

#include "ChemicalModel.h"
#include "NodeAdsorptionHistory.h"

#include "AdsorptionSimulator.h"

namespace Genex6
{
class Input;
class UnitTestDataCreator;
class SimulatorState;
class Simulator;


//!A particular point of a Source Rock where a simulation is performed
class SourceRockNode
{
public:
   SourceRockNode(const double in_thickness, const double in_TOCi, const double in_InorganicDensity, 
                  const double in_f1, const double in_f2,
                  const int in_I = 0, const int in_J = 0);
   virtual ~SourceRockNode();

   /// \brief Sets the source-rock-node to an initial state.
   void initialise ();

   void ClearInputHistory();
   void ClearOutputHistory();
   void ClearSimulatorStates();
   
   void AddInput(Input* in_theInput);
   void AddOuput(SimulatorState* in_theOuput);
   void AddSimulatorState(SimulatorState* in_theOuput);

   const Input* getLastInput() const;

   /// \brief Get the thickness at the node.
   double getThickness () const;


   unsigned int GetI()const;
   unsigned int GetJ()const;

   //  SimulatorState &GetSimulatorState() const;
   SimulatorState* GetSimulatorState( int id ) const;
   SimulatorState &GetMixedSimulatorState() const;

   /// \brief Returns simulator state of the mixed source rocks in the case of source-rock mixing otherwise returns the state of the single source-rock.
   SimulatorState& getPrincipleSimulatorState () const;
  
   double GetF1() const;
   double GetF2() const;

   //  void CreateSimulatorState(const double currentTime,Simulator & theSimulator);
   int CreateSimulatorState(int numberOfSourceRock, const double currentTime, Simulator & theSimulator);

   //interface
   int GetInputHistorySize() const;
   
   enum {FAIL = 0, SUCCESS = 1};
   //!Computation using a Simulator object and the avaible Input History
   /*!
   * RequestCompuation(Simulator & ), handles a simulation request from a Simulator given the input history available at the moment of the call
   */
  
   bool RequestComputation(int numOfSourceRock, Simulator & theSimulator );
   void RequestComputation(Simulator & theSimulator);
   int  RequestComputation1D(int numOfSourceRock, Simulator *theSimulator, double snapshots[], const int numberOfSnapshots, 
                            const double depositionAge);
   bool RequestMixing( ChemicalModel * aModel );

   //General I/O
   void PrintBenchmarkOutput(const std::string & in_FullPathBenchmarkName, const Simulator & theSimulator) const;
   void PrintInputHistoryHeader(ofstream &outputfile) const;
   void PrintInputHistory(ofstream &outputfile) const;
   void PrintInputHistory(std::string &outputFullPathFileName) const;

   void PrintBenchmarkModelConcTableHeader(const ChemicalModel& chemicalModel, 
                                           ofstream &outputfile) const;

   void PrintBenchmarkModelConcTable(const ChemicalModel& chemicalModel, 
                                     ofstream &outputfile) const;

   void PrintBenchmarkModelFluxHeader(const ChemicalModel& chemicalModel, 
                                      ofstream &outputfile) const;

   void PrintBenchmarkModelFluxTable(const ChemicalModel& chemicalModel, 
                                     ofstream &outputfile) const;

   void PrintBenchmarkModelCumExpHeader(const ChemicalModel& chemicalModel, 
                                        ofstream &outputfile) const;

   void PrintBenchmarkModelCumExpTable(const ChemicalModel& chemicalModel, 
                                       ofstream &outputfile) const;

   void CreateInputPTHistory(const UnitTestDataCreator &theUnitTestDataCreator);
   void CreateInputPTHistory(FILE * fp);
   void CreateInputPTHistory(const std::vector<double>& time, const std::vector<double>& temperature, const std::vector<double>& pressure);
   void LoadTestingPTHistory(std::string in_FullPathFileName);
   void NormalizePTHistory(const double dt, bool linearTemperature = false);
   bool FindTimeInstance(const double timeCounter, std::vector<Input*>::iterator &Current);
   void FindInterpolationPair (const double timeCounter, std::vector<Input*>::iterator &Previous,
                               std::vector<Input*>::iterator &Next);

   void PrintSensitivityOutputHeader(ofstream &outputfile) const;
   void PrintSensitivityOutput(std::string &sensitivityOutputFullPathFileName) const;

   /// \brief Add a history.
   void addNodeAdsorptionHistory ( NodeAdsorptionHistory* adsorptionHistory );
   double getSGMeanBulkDensity () const;

   void updateAdsorptionOutput ( const AdsorptionSimulator& adsorptionSimulator );


   /// \brief Computes the volume of oil and gas per volume of rock.
   ///
   /// Since pvt is being called here several other properties are calclated.
   /// like api of both oil and condensate, gor and cgr.
   void computeHcVolumes ( double& gasVolume,
                           double& oilVolume,
                           double& gasExpansionRatio,
                           double& gor,
                           double& cgr,
                           double& oilApi,
                           double& condensateApi ) const;

   /// \brief Compute the charge-factor assuming no expulsion.
   void computeOverChargeFactor ( double& overChargeFactor ) const;

   void collectHistory ();

   /// \brief Set all the accumulations for the time-step to be zero.
   void zeroTimeStepAccumulations ();

   const SimulatorState& getState () const;
   
private:



   const double m_thickness;

   //Source Rock Node Total Organic Content, source rock property value from GUI, in the VBA from GEO.ROC
   const double m_TOCi;   
   const double m_InorganicDensity;   //source rock property,  DensIO
   const unsigned int m_I;
   const unsigned int m_J;
   
   // SR mixing fractions
   const double m_f1;
   const double m_f2;

   // double m_ConcKi;
   // double m_ConcKi2; // concki for Source rock type II
  
   SimulatorState *m_currentState;
   // SimulatorState *m_subProcessSimulatorState;

   std::vector<SimulatorState*> m_theSimulatorStates;
   std::vector<double> m_ConcKi;

   // SimulatorState where results of Source Rocks mixig are collected
   SimulatorState *m_mixedSimulatorState;

   std::vector<Input*> m_theInput;
   std::vector<SimulatorState*> m_theOutput;

   NodeAdsorptionHistoryList m_adsorptionHistoryList;

};
inline unsigned int SourceRockNode::GetI()const
{
   return m_I;
}
inline unsigned int SourceRockNode::GetJ()const
{
   return m_J;
}

inline double SourceRockNode::getThickness () const {
   return m_thickness;
}

inline double SourceRockNode::GetF1() const {
   return m_f1;
}

inline double SourceRockNode::GetF2() const {
   return m_f2;
}

}
#endif
