#ifndef INVERSION_CASE_H
#define INVERSION_CASE_H

#include <string>
#include <vector>

class Parameter;
class DatadrillerProperty;

class Case
{
public:

   Case();
   Case(const std::vector< Parameter* > & values); 

   void addParameter(Parameter * parameter);

   void addVariableToDrill(const DatadrillerProperty & oneVariableDef);

   void setProjectFile(const std::string & filename1);

   void setResultsFile(const std::string & filename2);

   void createProjectFile(const std::string & input, const std::string & output);

   void defineLocationToDrill(const DatadrillerProperty & propertyDrilled);

   void readOnePropertyProjectFile(const std::string & propertyName = "Temperature", int propertyIterator = 1, double snapshotTime = 0.0, double x = 460001, double y = 6750001);

   void readProjectFile();

   void displayResults() const;

   void displayParameters() const;

private:

   std::string m_workingProjectFile;
   std::vector<Parameter*> m_values;

private:

   std::string m_outputDataFile;
   std::vector< std::vector<double> > m_results;
   std::vector<double> m_zs;
   double m_xs;
   double m_ys;
   double m_ts;

private:

   std::vector< std::string > m_variablesToDrill;
};

#endif
