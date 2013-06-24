#ifndef INVERSION_SCENARIO_H
#define INVERSION_SCENARIO_H

#include <string>
#include <vector>
#include <iosfwd>

#include <boost/shared_ptr.hpp>

class Parameter;
class DatadrillerProperty;

/// A Scenario represents a set of several Parameter settings. These can be applied on a Cauldron project. 
class Scenario
{
public:
   Scenario ();

   /// Check whether scenario is valid
   bool isValid() const;

   /// Add a Parameter setting to this scenario.
   void addParameter(Parameter * parameter);

   /// Apply the Parameter settings on the template Cauldron project file 'originalProjectFile'
   /// and save the resulting file as 'workingProjectFile'.
   void createProjectFile(const std::string & originalProjectFile, const std::string & workingProjectFile) const;

   /// For debugging purposes: print the parameter settings.
   void printParameters(std::ostream & output) const;

private:
   std::vector< boost::shared_ptr<Parameter> > m_parameters;
   mutable bool m_isValid;
};

#endif
