#ifndef INVERSION_BASEMENT_PROPERTY_H
#define INVERSION_BASEMENT_PROPERTY_H

#include <string>
#include <vector>
#include "InputParameterDataRow.h"
#include "case.h"

//typedef double Time; // in mA (million of years ago)
//typedef double Thickness; // in meters.
//typedef std::pair<Time, Thickness> ThicknessAtTime;

class Property
{

public:
  Property(const std::string & name, double start, double end, double step)
    : m_name(name)
    , m_start(start)
    , m_end(end)
    , m_step(step)
  {};

  virtual ~Property() {}

  virtual void CreateParameter(Case & project, double value)=0;

  const std::string & getName() const
  { return m_name; }

  double getStart() const
  { return m_start; }

  double getEnd() const
  { return m_end; }

  double getStep() const
  { return m_step; }


protected:
  std::string m_name;
  double m_start, m_end, m_step;
};




class BasementProperty : public Property
{

public:
  BasementProperty(const std::string & name, double start, double end, double step)
    : Property(name,start,end,step)
  {}

  virtual void CreateParameter(Case & project, double value);

private:

};




class CrustalThinningProperty : public Property
{

public:
  CrustalThinningProperty(std::vector<InputParameterDataRow> CrustalThinningData);
/*    : m_CrustalThinningData(CrustalThinningData) 
  {
    m_name="Crustal Thinning";
    m_start=1;
    m_step=1;
    m_end=1;

  for (int i=1; i<CrustalThinningData.size(); ++i )
   {
    m_end*=CrustalThinningData[i].calculate_nb_cases();
   }

  }*/

  virtual void CreateParameter(Case & project, double value);

  void FromUserInputParameterToCrustalIoTable();

private:

  std::vector<InputParameterDataRow> m_CrustalThinningData;
  std::vector< std::vector< ThicknessAtTime > > m_CrustalThinningTableforProjectFile;

};


//=====================================================



#endif
