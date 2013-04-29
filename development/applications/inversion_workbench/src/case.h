#ifndef INVERSION_CASE_H
#define INVERSION_CASE_H

#include "parameter.h"
#include <string>
#include <vector>
#include "project.h"

#include "generalexception.h"
#include "ProbeException.h"
#include "Interface/ProjectHandle.h"
#include "DataMiningProjectHandle.h"
#include "CauldronDomain.h"
#include "DatadrillerProperty.h"

/*class InlineVector
{
public:

  InlineVector & add(double x)
  {
    m_vector.push_back(x);
    return *this;
  }

  operator std::vector<double> () const
  {
    return m_vector;
  }

private:
  std::vector<double> m_vector;
};*/



class Case
{
public:

   Case();
   Case(const std::vector< Parameter* > & values); //const std::string & templateProjectFilename,

   void addParameter(Parameter * parameter);

   void addVariableToDrill(const DatadrillerProperty & OneVariableDef);

   void set_ProjectFile(const std::string & filename1);

   void set_ResultsFile(const std::string & filename2);

   void create_project_file(const std::string & input, const std::string & output);

   void Define_location_to_drill(const DatadrillerProperty & PropertyDrilled);

   void readOnePropertyProjectFile(const std::string & propertyName = "Temperature", int Property_Iterator = 1, double snapshotTime = 0.0, double x = 460001, double y = 6750001);

   void readProjectFile();

// InlineVector().add(0).add(500).add(1000));

   void display_results() const;

   void display_Parameters() const;

private:

   std::string m_workingProjectFile;
   std::vector<Parameter*> m_values;

private:

   std::string m_outputDataFile;
   std::vector< vector<double> > results;
   std::vector<double> zs;
   double xs;
   double ys;
   double ts;

private:

   std::vector<string> m_VariablesToDrill;

};

#endif
