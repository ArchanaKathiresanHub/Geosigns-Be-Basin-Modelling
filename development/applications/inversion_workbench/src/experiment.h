#ifndef INVERSION_EXPERIMENT_H
#define INVERSION_EXPERIMENT_H

#include <string>
#include <vector>
#include <iosfwd>

#include <boost/shared_ptr.hpp>

#include "case.h"
#include "RuntimeConfiguration.h"

class DatadrillerProperty;
class Property;

/// An Experiment consists of several cases (see Case) that are generated / sampled
/// from a set of properties (see Property). This experiment can be executed and 
/// the results can be collected.
class Experiment
{
public:
   Experiment( const std::vector< boost::shared_ptr<Property> > & params, const std::vector<DatadrillerProperty> & DatadrillerDefinitions, const RuntimeConfiguration & datainfo);

   /// Generate the set of Cauldron project files from the cases: one for each case.
   void createProjectsSet() const;

   /// Run fastcauldron on each generated project file
   void runProjectSet();

   /// Collect the results into .dat files.
   void collectResults() const;

   /// For debugging purposes: show which cases have been generated.
   void printCases(std::ostream & output) const;

private:
   void sample(std::vector< boost::shared_ptr<Property> > & parameterDefinitions, std::vector< Case > & allProjects );
   std::string workingProjectFileName(unsigned caseNumber) const;
   std::string resultsFileName(unsigned caseNumber) const;

   std::vector< Case > m_cases;
   std::vector< DatadrillerProperty > m_probes;
   RuntimeConfiguration m_experimentInfo;
};

#endif
