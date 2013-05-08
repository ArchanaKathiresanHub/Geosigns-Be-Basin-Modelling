#ifndef INVERSION_CASE_H
#define INVERSION_CASE_H

#include <string>
#include <vector>
#include <iosfwd>

#include <boost/shared_ptr.hpp>

class Parameter;
class DatadrillerProperty;

/// A Case represents a set of several Parameter settings. These can be applied on a Cauldron project. 
class Case
{
public:
   /// Add a Parameter setting to this case.
   void addParameter(Parameter * parameter);

   /// Apply the Parameter settings on the template Cauldron project file 'originalProjectFile'
   /// and save the resulting file as 'workingProjectFile'.
   void createProjectFile(const std::string & originalProjectFile, const std::string & workingProjectFile) const;

   /// For debugging purposes: print the parameter settings.
   void printParameters(std::ostream & output) const;

private:
   std::vector< boost::shared_ptr<Parameter> > m_values;
};

#endif
