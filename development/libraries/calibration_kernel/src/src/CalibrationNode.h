#ifndef CALIBRATIONNODE_H
#define CALIBRATIONNODE_H

#include <vector>
#include <string>
#include "SmectiteIlliteSimulatorState.h"
#include "BiomarkersSimulatorState.h"

namespace Calibration
{
class NodeInput;
class SmectiteIlliteSimulator;
class BiomarkersSimulator;

class SmectiteIlliteOutput;
class BiomarkersOutput;

class CalibrationNode
{
public:
   CalibrationNode(const double in_InitIlliteFraction = 0.0, 
		  				 const int in_I = 0, 
		  				 const int in_J = 0);
   ~CalibrationNode();

	enum InputTimeOrder{ ASCENDING, DESCENDING };

   void clearInputHistory();
   void clearOutputHistory();
	
   void addInput(NodeInput* in_theInput);

	const NodeInput * getInputByIndex(const int index) const;
   const SmectiteIlliteOutput * getSmectiteIlliteOutputByIndex(const int index) const;
	size_t getSmectiteIlliteOutputSize()const;
	const BiomarkersOutput * getBiomarkersOutputByIndex(const int index) const;
	size_t getBiomarkersOutputSize()const;

   unsigned int getI()const;
   unsigned int getJ()const;

   ///Computation using the Input History
   void requestComputation(const SmectiteIlliteSimulator &theSimulator, InputTimeOrder ORDER = DESCENDING);
	void requestComputation(const BiomarkersSimulator &theSimulator, InputTimeOrder ORDER = DESCENDING);

protected:
	void preprocessInput(InputTimeOrder INPUT_ORDER);
	void addOuput(SmectiteIlliteOutput* in_theOuput);
	void addOuput(BiomarkersOutput* in_theOuput);
	void clearSmectiteIlliteOutputHistory();
	void clearBiomarkersOutputHistory();

private:
	SmectiteIlliteSimulatorState m_SmectiteIlliteState;
	BiomarkersSimulatorState 		m_BiomarkersState;

   const unsigned int m_I;
   const unsigned int m_J;
	bool m_initialized;//todo--> initialized for both...@@do not need it anymore, was moved to State. check and remove it
 
   std::vector<NodeInput*> m_theInput;
   std::vector<SmectiteIlliteOutput*> m_SmectiteIlliteOutput;
	std::vector<BiomarkersOutput*> m_BiomarkersOutput;
};
inline unsigned int CalibrationNode::getI()const
{
   return m_I;
}
inline unsigned int CalibrationNode::getJ()const
{
   return m_J;
}
inline size_t CalibrationNode::getSmectiteIlliteOutputSize()const
{
	return m_SmectiteIlliteOutput.size();
}
inline size_t CalibrationNode::getBiomarkersOutputSize()const
{
	return m_BiomarkersOutput.size();
}

}
#endif
