#include "CalibrationNode.h"

#include "NodeInput.h"
#include "SmectiteIlliteSimulator.h"
#include "BiomarkersSimulator.h"

#include "SmectiteIlliteOutput.h"
#include "BiomarkersOutput.h"

#include <math.h>
#include <iostream>
#include <fstream>
#include <iomanip>

#include <algorithm>


namespace Calibration
{

CalibrationNode::CalibrationNode(const double in_InitIlliteFraction ,
		  									const int in_I, 
		  									const int in_J ) :
m_SmectiteIlliteState(in_InitIlliteFraction ),
m_BiomarkersState(),
m_I(in_I),
m_J(in_J),
m_initialized(false)
{

}
CalibrationNode::~CalibrationNode()
{
   clearInputHistory();

   clearOutputHistory();
}
void CalibrationNode::clearInputHistory()
{
   std::vector<NodeInput*>::iterator it = m_theInput.begin();
   std::vector<NodeInput*>::iterator itEnd = m_theInput.end();
   while( it!= itEnd )
   {
      delete  (*(it++));
   }
   m_theInput.clear();
}
void  CalibrationNode::clearOutputHistory()
{
	clearSmectiteIlliteOutputHistory();
	clearBiomarkersOutputHistory();
}
void CalibrationNode::clearSmectiteIlliteOutputHistory()
{
   std::vector<SmectiteIlliteOutput*>::iterator it = m_SmectiteIlliteOutput.begin();
	std::vector<SmectiteIlliteOutput*>::iterator itEnd = m_SmectiteIlliteOutput.end();
   while(it != itEnd)
   {
      delete  (*(it++));
   }
   m_SmectiteIlliteOutput.clear();
}
void CalibrationNode::clearBiomarkersOutputHistory()
{
   std::vector<BiomarkersOutput*>::iterator it = m_BiomarkersOutput.begin();
	std::vector<BiomarkersOutput*>::iterator itEnd = m_BiomarkersOutput.end();
   while(it != itEnd)
   {
      delete  (*(it++));
   }
   m_BiomarkersOutput.clear();
}
void CalibrationNode::addOuput(SmectiteIlliteOutput* in_theOuput)
{
   m_SmectiteIlliteOutput.push_back(in_theOuput);
}
void CalibrationNode::addOuput(BiomarkersOutput* in_theOuput)
{
   m_BiomarkersOutput.push_back(in_theOuput);
}
void CalibrationNode::addInput(NodeInput* in_theInput)
{
   m_theInput.push_back(in_theInput);
}
void CalibrationNode::preprocessInput(InputTimeOrder INPUT_ORDER)
{
	if (ASCENDING == INPUT_ORDER) 
	{
		NodeInput::AscendingTimeOrderInput predicate;
		//if( false == is_sorted( m_theInput.begin(), m_theInput.end(), predicate) )
		//{
			std::sort(m_theInput.begin(), m_theInput.end(), predicate);
		//}

	}
	else if (DESCENDING == INPUT_ORDER) 
	{
      //cout<<"Sort in descending order"<<endl;
		std::vector<NodeInput*>::iterator itInput = m_theInput.begin();
		std::vector<NodeInput*>::iterator itEnd 	= m_theInput.end();

		NodeInput::DescendingTimeOrderInput predicate;
		//if( false == is_sorted( m_theInput.begin(), m_theInput.end(), predicate) )
		//{
			std::sort(m_theInput.begin(), m_theInput.end(), predicate);
		//}
	}
	else
	{
		//throw
	}
}
void CalibrationNode::requestComputation(const SmectiteIlliteSimulator &theSimulator, InputTimeOrder INPUT_ORDER)
{
	preprocessInput(INPUT_ORDER);

	std::vector<NodeInput*>::iterator itInput = m_theInput.begin();
	std::vector<NodeInput*>::iterator itEnd 	= m_theInput.end();

	if( false == m_SmectiteIlliteState.isInitialized() &&  itInput != itEnd)
	{
		theSimulator.initializeState( (*(*itInput)),	m_SmectiteIlliteState);
		SmectiteIlliteOutput *theNodeOutput = new SmectiteIlliteOutput( (*itInput)->getReferenceTime() );
	
		addOuput(theNodeOutput);
		++itInput;//first input in the sequence was used for initialization --> skip it
	}	
	else 
	{
		//throw
		//no input history available
	}

  	while(itInput != itEnd)
   {
     	theSimulator.advanceState((*(*itInput)), m_SmectiteIlliteState);

		SmectiteIlliteOutput *theNodeOutput = new SmectiteIlliteOutput( (*itInput)->getReferenceTime() );

		theSimulator.computeOutput(m_SmectiteIlliteState, *theNodeOutput);

     	addOuput(theNodeOutput);

		++itInput;
  	}
}
void CalibrationNode::requestComputation(const BiomarkersSimulator &theSimulator, InputTimeOrder INPUT_ORDER)
{
	preprocessInput(INPUT_ORDER);

	std::vector<NodeInput*>::iterator itInput = m_theInput.begin();
	std::vector<NodeInput*>::iterator itEnd 	= m_theInput.end();

	if( false == m_BiomarkersState.isInitialized() &&  itInput != itEnd)
	{
		theSimulator.initializeState( (*(*itInput)),	m_BiomarkersState);
		BiomarkersOutput *theNodeOutput = new BiomarkersOutput( (*itInput)->getReferenceTime() );
		addOuput(theNodeOutput);
		++itInput;//first input in the sequence was used for initialization --> skip it
	}	
	else 
	{
		//throw
		//no input history available
	}

  	while(itInput != itEnd)
   {
     	//theSimulator.advanceState((*(*itInput)), m_BiomarkersState);
		theSimulator.advanceState((*(*itInput)), m_BiomarkersState);

		BiomarkersOutput *theNodeOutput = new BiomarkersOutput( (*itInput)->getReferenceTime() );

		theSimulator.computeOutput(m_BiomarkersState, *theNodeOutput);

     	addOuput(theNodeOutput);

		++itInput;
  	}
}
const SmectiteIlliteOutput *CalibrationNode::getSmectiteIlliteOutputByIndex(const int index) const
{
   SmectiteIlliteOutput *ret = 0;
   if(index >= 0 && index < m_SmectiteIlliteOutput.size())
   {
      ret = m_SmectiteIlliteOutput[index];
   }
   return ret;
}
const NodeInput *CalibrationNode::getInputByIndex(const int index) const
{
   NodeInput *ret = 0;
   if(index >= 0 && index < m_theInput.size())
   {
      ret = m_theInput[index];
   }
   return ret;
}
const BiomarkersOutput *CalibrationNode::getBiomarkersOutputByIndex(const int index) const
{
   BiomarkersOutput *ret = 0;
   if(index >= 0 && index < m_BiomarkersOutput.size())
   {
      ret = m_BiomarkersOutput[index];
   }
   return ret;
}

}
