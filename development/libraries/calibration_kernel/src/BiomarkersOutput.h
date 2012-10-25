#ifndef BIOMARKERSOUTPUT_H
#define BIOMARKERSOUTPUT_H


#include<iostream>
#include <fstream>
#include<iomanip>

#include <map>
#include <string>
namespace Calibration
{

class BiomarkersOutput
{
public:
	class LessThanPointer
	{
	public:
		bool operator()(const BiomarkersOutput *Left, const BiomarkersOutput *Right)
		{
			return (Left->getReferenceTime() < Right->getReferenceTime());
		}
	};    
   BiomarkersOutput(const double inReferenceTime = 0.0):
	m_referenceTime(inReferenceTime),
	m_SteraneAromatisation(0.0),
	m_SteraneIsomerisation(0.0),
	m_HopaneIsomerisation(0.0)	
	{

	}
   ~BiomarkersOutput()
	{

	}
   double getHopaneIsomerisation() const
	{
		return m_HopaneIsomerisation;
	}
	double getSteraneAromatisation() const
	{
		return m_SteraneAromatisation;
	}
	double getSteraneIsomerisation() const
	{
		return m_SteraneIsomerisation;
	}
	void setHopaneIsomerisation(const double &HopaneIsomerisation)
	{
		m_HopaneIsomerisation = HopaneIsomerisation;
	}
	void setSteraneAromatisation(const double &SteraneAromatisation)
	{
		m_SteraneAromatisation = SteraneAromatisation;
	}
	void setSteraneIsomerisation(const double &SteraneIsomerisation)
	{
		m_SteraneIsomerisation = SteraneIsomerisation;
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
   double m_SteraneAromatisation;
	double m_SteraneIsomerisation;
	double m_HopaneIsomerisation;
};

}
#endif
