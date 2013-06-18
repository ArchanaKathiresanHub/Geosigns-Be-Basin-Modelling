#ifndef INVERSION_EXPERIMENT_H
#define INVERSION_EXPERIMENT_H

#include <string>
#include <vector>
#include <iosfwd>

#include <boost/shared_ptr.hpp>

#include "Scenario.h"
#include "RuntimeConfiguration.h"

class DatadrillerProperty;
class Property;

/// An Experiment consists of several scenarios (see Scenario) that are generated / sampled
/// from a set of properties (see Property). This experiment can be executed and 
/// the results can be collected.
class Experiment
{
public:
   Experiment( const std::vector< boost::shared_ptr<Property> > & params, const std::vector<DatadrillerProperty> & DatadrillerDefinitions, const RuntimeConfiguration & datainfo);

   /// Generate the set of Cauldron project files from the scenarios: one for each scenario.
   void createProjectsSet() const;

   /// Run fastcauldron on each generated project file
   void runProjectSet(const std::string & cauldronVersion);

   /// Collect the results into .dat files.
   void collectResults() const;

   /// For debugging purposes: show which scenarios have been generated.
   void printScenarios(std::ostream & output) const;

private:
   static std::vector< Scenario > sample(const std::vector< boost::shared_ptr<Property> > & parameterDefinitions );
   std::string workingProjectFileName(unsigned scenarioNumber) const;
   std::string workingLogFileName(unsigned scenarioNumber) const;
   std::string resultsFileName(unsigned scenarioNumber) const;

   std::vector< Scenario > m_scenarios;
   std::vector< DatadrillerProperty > m_probes;
   RuntimeConfiguration m_experimentInfo;
};

#endif
