#ifndef SMECTITEILLITEOUTPUT_H
#define SMECTITEILLITEOUTPUT_H


#include<iostream>
#include <fstream>
#include<iomanip>

#include <map>
#include <string>
namespace Calibration
{

class SmectiteIlliteOutput
{
public:
   SmectiteIlliteOutput(const double inReferenceTime = 0.0, const double inTransfRatio = 0.0):
	m_referenceTime(inReferenceTime),
   m_illiteTransfRatio(inTransfRatio)	
	{

	}
   ~SmectiteIlliteOutput()
	{

	}
   double getIlliteTransfRatio() const
	{
		return m_illiteTransfRatio;
	}
	void setIlliteTransfRatio(const double &inIlliteTransfRatio)
	{
		m_illiteTransfRatio = inIlliteTransfRatio;
	}
   void setReferenceTime(const double &inReferenceTime)
	{
		m_referenceTime = inReferenceTime;
	}
	double getReferenceTime() const
	{
		return m_referenceTime;
	}  
private:  
   double m_referenceTime;
   double m_illiteTransfRatio; 

	//friend ostream &
	//operator<<(ostream & os, const SmectiteIlliteOutput &inOutput);
   
};
class LessThanPointer
{
public:
	bool operator()(const SmectiteIlliteOutput *Left, const SmectiteIlliteOutput *Right)
	{
		return (Left->getReferenceTime() < Right->getReferenceTime());
	}

};    

}
#endif
