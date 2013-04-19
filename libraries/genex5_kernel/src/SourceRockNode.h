#ifndef SOURCEROCKNODE_H
#define SOURCEROCKNODE_H

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <iomanip>
using namespace std;

#include "NodeAdsorptionHistory.h"

namespace Genex5
{
class SourceRockNodeInput;
class SourceRockNodeOutput;
class UnitTestDataCreator;
class SimulatorState;
class Simulator;

//!A particular point of a Source Rock where a simulation is performed
class SourceRockNode
{
public:
   SourceRockNode(const double &in_thickness,
                  const double &in_TOCi,
                  const double &in_InorganicDensity, 
                  const double &in_sgMeanBulkDensity, 
                  const int &in_I=0,
                  const int &in_J=0);

   virtual ~SourceRockNode();

   void ClearInputHistory();
   void ClearOutputHistory();
   
   void AddOuput(SourceRockNodeOutput* in_theOuput);
   void AddInput(SourceRockNodeInput* in_theInput);

   const SourceRockNodeOutput *GetTheLastOutput() const;
   const SourceRockNodeOutput *GetOutputByIndex(const int &index) const;


   unsigned int GetOutputSize() const;

   const SourceRockNodeInput* getTheLastInput() const;

   const SimulatorState* getSimulatorState () const;

   SimulatorState* getSimulatorState ();

   unsigned int getInputSize() const;

   double getInorganicDensity () const;

   double getSGMeanBulkDensity () const;

   unsigned int GetI()const;
   unsigned int GetJ()const;

   double getThickness () const;

   //interface
   int GetInputHistorySize() const;
   
   enum {FAIL = 0, SUCCESS = 1};
   //!Computation using a Simulator object and the avaible Input History
   /*!
   * RequestCompuation(Simulator & ), handles a simulation request from a Simulator given the input history available at the moment of the call
   */
   void RequestComputation(Simulator & theSimulator,
                           const bool isSnapshot = false );
   int RequestComputation1D(Simulator *theSimulator, double snapshots[], const int &numberOfSnapshots, const double &depositionAge);

   //General I/O
   void PrintBenchmarkOutput(const std::string &in_FullPathBenchmarkName, Simulator &theSimulator);
   void PrintInputHistoryHeader(ofstream &outputfile) const;
   void PrintInputHistory(ofstream &outputfile) const;
   void PrintInputHistory(std::string &outputFullPathFileName) const;

   void PrintBenchmarkModelConcTableHeader(ofstream &outputfile) const;
   void PrintBenchmarkModelConcTable(ofstream &outputfile) const;

   void PrintBenchmarkModelFluxHeader(ofstream &outputfile) const;
   void PrintBenchmarkModelFluxTable(ofstream &outputfile) const;

   void PrintBenchmarkModelCumExpHeader(ofstream &outputfile) const;
   void PrintBenchmarkModelCumExpTable(ofstream &outputfile) const;

   void CreateTestingPTHistory(const UnitTestDataCreator &theUnitTestDataCreator);
   void LoadTestingPTHistory(std::string in_FullPathFileName);
   void NormalizePTHistory(const double &dt, bool linearTemperature=false);
   bool FindTimeInstance(const double &timeCounter, std::vector<SourceRockNodeInput*>::iterator &Current);
   void FindInterpolationPair (const double &timeCounter, std::vector<SourceRockNodeInput*>::iterator &Previous,std::vector<SourceRockNodeInput*>::iterator &Next);

   void PrintSensitivityOutputHeader(ofstream &outputfile) const;
   void PrintSensitivityOutput(std::string &sensitivityOutputFullPathFileName) const;
   
   void addNodeAdsorptionHistory ( NodeAdsorptionHistory* adsorptionHistory );


private:

   void updateOutput (       Simulator&            simulator,
                       const SourceRockNodeInput*  input,
                             SimulatorState*       state,
                             SourceRockNodeOutput* output );

   void collectHistory ();

   void computeCurrentToc ();

   const double m_thickness;
   const double m_TOCi;               //Source Rock Node Total Organic Content, source rock property value from GUI, in the VBA from GEO.ROC
   const double m_InorganicDensity;   //source rock property,  DensIO
   const double m_sgMeanBulkDensity;
   const unsigned int m_I;
   const unsigned int m_J;
   double m_ConcKi;
   double m_currenToc;
   
   SimulatorState *m_currentState;
   std::vector<SourceRockNodeInput*> m_theInput;
   std::vector<SourceRockNodeOutput*> m_theOutput;

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

inline double SourceRockNode::getInorganicDensity () const {
   return m_InorganicDensity;
}

inline double SourceRockNode::getSGMeanBulkDensity () const {
   return m_sgMeanBulkDensity;
}

}
#endif
