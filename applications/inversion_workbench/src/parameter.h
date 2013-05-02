#ifndef INVERSION_PARAMETER_H
#define INVERSION_PARAMETER_H

#include <string>
#include <vector>
#include "project.h"

typedef double Time; // in mA (million of years ago)
typedef double Thickness; // in meters.
typedef std::pair<Time, Thickness> ThicknessAtTime;

// Abstract class
class Parameter
{
public:
  virtual ~Parameter() {};
  virtual void print()=0;
  virtual void Change_parameter(Project & project)=0;

private:
};



class BasementParameter : public Parameter
{
public:
  BasementParameter( const std::string & name, double value);
  virtual ~BasementParameter() {};
  virtual void print();
  virtual void Change_parameter(Project & project);

private:
  std::string m_name;
  double m_value;
};



class CrustalThinningParameter : public Parameter
{
public:
  CrustalThinningParameter(std::vector< ThicknessAtTime > & series);
  virtual ~CrustalThinningParameter() {};
  virtual void print();
  virtual void Change_parameter(Project & project);

private:
  std::vector< ThicknessAtTime > m_series;
};



 // double m_t0;
 // double m_Dt;
 // double m_thickness;






/*class Parameter
{
public:
  Parameter( const std::string & name, double value);
//  Parameter():m_name("A"),m_value(0) {};
  virtual ~Parameter() {};
  //void updateProjectFile(DataAccess::Interface::ProjectHandle * project) ;
  void print();
  virtual void Change_parameter(Project & project);


private:
  std::string m_name;
  double m_value;
};*/




#endif

