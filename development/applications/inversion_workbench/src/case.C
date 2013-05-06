#include <string>
#include <iostream>

#include "case.h"
#include "DatadrillerProperty.h"
#include "formattingexception.h"
#include "parameter.h"
#include "project.h"

#include "Interface/ProjectHandle.h"
#include "DataMiningProjectHandle.h"
#include "CauldronDomain.h"

Case::Case(const std::vector<Parameter*> & values)
   : m_values(values)
   , m_workingProjectFile("not defined")
   , m_outputDataFile("not defined")
   , m_results(0)
{
}


Case::Case()
   : m_values()
   , m_workingProjectFile()
   , m_outputDataFile()
   , m_results()
{
}


void Case::addParameter(Parameter * pt_parameter)
{
   m_values.push_back( pt_parameter);
}


void Case::addVariableToDrill(const DatadrillerProperty & oneVariableDef )
{
   m_variablesToDrill.push_back( oneVariableDef.getName() );
}


void Case::setProjectFile(const std::string & filename1)
{
   m_workingProjectFile = filename1;
}

void Case::setResultsFile(const std::string & filename2)
{
   m_outputDataFile = filename2;
}

void Case::createProjectFile(const std::string & input, const std::string & output)
{
   Project project( input, output);

   for (unsigned i = 0; i < m_values.size(); ++i)
   {
      m_values[i]->changeParameter(project);
   }

   project.close();
}


void Case::defineLocationToDrill(const DatadrillerProperty & propertyDrilled)
{
   double start = propertyDrilled.getPositionBegZ();
   double end = propertyDrilled.getPositionEndZ();
   double step = propertyDrilled.getStepZ();

   double currentDepth = start; 

   m_zs.clear();

   while(currentDepth <= end)
   {
      m_zs.push_back(currentDepth);
      currentDepth += step;
   }

   m_xs = propertyDrilled.getPositionX();
   m_ys = propertyDrilled.getPositionY();
   m_ts = propertyDrilled.getTime();
}

struct ReadPropertyException : formattingexception::BaseException<ReadPropertyException > {};

/// Probes a Cauldron project3d file. Given:
///  - a physical quantity, as 'propertyName',
///  - a snapshot time,
///  - horizontal coordinates, x and y, and
///  - a series of depths, in zs,
///  this function return the results from the simulation in the output vector 'm_results'.
void Case::readOnePropertyProjectFile(const std::string & propertyName, int Property_Iterator, double snapshotTime, double x, double y)
{
   // clear the result vector
   //  m_results.clear();

   const std::string project = m_workingProjectFile;

   // Open project file
   DataAccess::Mining::DomainPropertyFactory factory;
   DataAccess::Interface::ProjectHandle::UseFactory( &factory );

   DataAccess::Mining::ProjectHandle* projectHandle 
     = dynamic_cast< DataAccess::Mining::ProjectHandle * >(
 	 DataAccess::Interface::OpenCauldronProject(project, "r")
       );

   if (!projectHandle)
      throw ReadPropertyException() << "Could not load project file '" << project << "'";

   // Load property
   const DataAccess::Interface::Property* property = projectHandle->findProperty (propertyName);
   if (!property) 
      throw ReadPropertyException() << "Unknown PropertyName value: " << propertyName;

   // Load snapshot
   if (snapshotTime < 0) 
      throw ReadPropertyException() << "Illegal snapshot time: " << snapshotTime;
   const DataAccess::Interface::Snapshot * snapshot = projectHandle->findSnapshot (snapshotTime);
   DataAccess::Mining::CauldronDomain domain ( projectHandle );
   domain.setSnapshot (snapshot);
   DataAccess::Mining::DomainPropertyCollection* domainProperties = projectHandle->getDomainPropertyCollection ();
   domainProperties->setSnapshot (snapshot);
	
   // Check whether the x and y are in the grid
   unsigned int a, b;
   const DataAccess::Interface::Grid * grid = projectHandle->getLowResolutionOutputGrid ();
   if (!grid->getGridPoint (x, y, a, b)) 
      throw ReadPropertyException() << "Illegal (XCoord, YCoord) pair: (" << x << ", " << y << ")";
       
   // Retrieve the results
   for (size_t i = 0; i < m_zs.size(); ++i)
   {
      DataAccess::Mining::ElementPosition element;
      if (!domain.findLocation (x, y, m_zs[i], element))
         throw ReadPropertyException() << "Illegal point coordinates: " << x << ", " << y << ", " << m_zs[i];

      m_results[Property_Iterator].push_back( domainProperties->getDomainProperty (property)->compute (element) );
   }
}



void Case::readProjectFile()
{

/*  for (int i = 0; i < 5; ++i )  // To be changed 
   {
     m_zs.push_back(double(i) * 100.0);
   }*/

   m_results.resize( m_variablesToDrill.size() );

/*  for (int i = 0; i < m_variablesToDrill.size(); ++i )
   {
     m_results[i].resize(m_zs.size());
   }
*/

   for(size_t i = 0; i < m_variablesToDrill.size(); ++i)
   {
      readOnePropertyProjectFile( m_variablesToDrill[i], i, m_ts, m_xs, m_ys );
   }
}



void Case::displayResults() const
{
   std::cout << "Output data are being saved..." << std::endl;;
   std::ofstream ofs( (m_outputDataFile + ".dat").c_str(), ios_base::out | ios_base::trunc );

   ofs << "Datamining from project " << m_workingProjectFile << " :"<< endl;
   for (size_t i = 0; i < m_results.size(); ++i)
   {
      ofs << m_variablesToDrill[i] << " ";
      for (size_t j = 0; j < m_results[i].size(); ++j)
      {
         ofs << m_results[i][j] << " ";
      }
      ofs  << endl;
   }
}



void Case::displayParameters() const
{
   std::cout << "Displaying parameters: " << std::endl;
   for (int i = 0; i < m_values.size(); ++i)
   {
      m_values[i]->print();
   }
}


