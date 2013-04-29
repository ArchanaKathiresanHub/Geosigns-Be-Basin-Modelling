#ifndef INVERSION_EXPERIMENT_H
#define INVERSION_EXPERIMENT_H

#include <string>
#include <vector>
#include "case.h"
#include "DatadrillerProperty.h"
#include <sstream>

class Experiment
{
public:
//  Experiment(const std::string & configurationFile);
  Experiment(std::vector< Property * > params, std::vector<DatadrillerProperty> & DatadrillerDefinitions, RuntimeConfiguration & datainfo);

  void sample(std::vector<Property *> parameterDefinitions, std::vector< Case > & allProjects );
//  void Case :: addCase(const Case & case);

  void define_datamining( std::vector<DatadrillerProperty> & DatadrillerDefinitions, std::vector< Case > & allProjects );

  std::vector<std::string> create_projects_set();

  void runProjectSet( const std::vector< std::string > & fileList);

  void ReadExperimentResults();

  void ReadExperimentCases();

  void display_Cases() const;


private:
  std::vector< Case > m_cases;
  RuntimeConfiguration m_experiment_info;
};

#endif
