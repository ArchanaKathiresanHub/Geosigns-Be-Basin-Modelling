#ifndef INVERSION_EXPERIMENT_H
#define INVERSION_EXPERIMENT_H

#include <string>
#include <vector>
#include <sstream>

#include "case.h"
#include "RuntimeConfiguration.h"

class DatadrillerProperty;
class Property;

class Experiment
{
public:
//  Experiment(const std::string & configurationFile);
   Experiment(std::vector< Property * > params, std::vector<DatadrillerProperty> & DatadrillerDefinitions, RuntimeConfiguration & datainfo);

   void sample(std::vector<Property *> parameterDefinitions, std::vector< Case > & allProjects );

   void defineDatamining( std::vector<DatadrillerProperty> & DatadrillerDefinitions, std::vector< Case > & allProjects );

   std::vector<std::string> createProjectsSet();

   void runProjectSet( const std::vector< std::string > & fileList);

   void readExperimentResults();

   void readExperimentCases();

   void displayCases() const;


private:
   std::vector< Case > m_cases;
   RuntimeConfiguration m_experimentInfo;
};

#endif
