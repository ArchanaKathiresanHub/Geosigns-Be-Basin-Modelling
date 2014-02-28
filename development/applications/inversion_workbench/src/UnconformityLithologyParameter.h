#ifndef INVERSION_UNCONFORMITY_LITHOLOGY_PARAMETER_H
#define INVERSION_UNCONFORMITY_LITHOLOGY_PARAMETER_H

#include <string>

#include "parameter.h"

class Project;

class UnconformityLithologyParameter : public Parameter
{
public:
   UnconformityLithologyParameter(const std::string & depoFormationName,
	 const std::string lithology1, const double percentage1,
	 const std::string lithology2, const double percentage2,
	 const std::string lithology3, const double percentage3) :
      m_depoFormationName(depoFormationName),
      m_lithology1 (lithology1), m_percentage1 (percentage1),
      m_lithology2 (lithology2), m_percentage2 (percentage2),
      m_lithology3 (lithology3), m_percentage3 (percentage3)
   {
   }

   virtual void print(std::ostream & output);
   virtual void changeParameter(Project & project);

   /// All unconformity lithology parameters are continious now
   virtual bool isContinuous() const { return true; }

   /// Return parameter value as vector of doubles
   virtual std::vector<double> toDblVector() const;

   /// Set value for parameter from array of doubles
   virtual void fromDblVector( const std::vector<double> & prms );


private:
   std::string m_depoFormationName;

   std::string m_lithology1;
   const double m_percentage1;
   std::string m_lithology2;
   const double m_percentage2;
   std::string m_lithology3;
   const double m_percentage3;
};

#endif

