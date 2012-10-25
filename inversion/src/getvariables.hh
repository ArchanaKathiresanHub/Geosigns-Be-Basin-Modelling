#ifndef GETVARIABLES_HH
#define GETVARIABLES_HH
#include <iostream>   
#include <string>
#include <vector>

//! getting the variable names for the given property
/*!
  \param  piproperty property name
it returns the variables as a vector of strings
*/
std::vector<std::string> getVariableNames(std::string piproperty)
{
  std::vector<std::string> qstrVector;

  if(piproperty=="PI_WELL" || piproperty=="PI_PERFORATION_QT")
	{
	  qstrVector.push_back("QOF");
	  qstrVector.push_back("QWF");
	  qstrVector.push_back("QGF");	  
	}
  else if(piproperty=="PI_PERFORATION_QO")
	{
	  qstrVector.push_back("QOF");
	}
  else if(piproperty=="PI_PERFORATION_QW")
	{
	  qstrVector.push_back("QWF");
	}
  else if(piproperty=="PI_PERFORATION_QG")
	{
	  qstrVector.push_back("QGF");
	}
  else if(piproperty=="PI_PERFORATION_P")
	{
	  qstrVector.push_back("P");
	  qstrVector.push_back("PI");
	  qstrVector.push_back("PPRF");
	  //qstrVector.push_back("STAP0");
	}
  return qstrVector;
}
#endif
